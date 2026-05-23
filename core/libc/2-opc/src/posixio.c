/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2026 Fraser Heavy Software
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <__onramp/__pit.h>
#include <__onramp/__syscalls.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>

#include "internal.h"

/**
 * This implements low-level POSIX file I/O (e.g. open(), write(), etc.) The C
 * file API (e.g. fopen(), fwrite(), etc.) in libc/2 and libc/3 wraps this.
 */

// This is the program's terminal state. These are initialized in __io_init()
// because opC doesn't have global initializers. They are modified by the user
// program via fcntl() and tcsetattr(). If these don't match the VM's
// capabilities, we simulate the necessary behaviour.
static bool input_echo;
static bool input_block;
static bool input_canonical;

/*
 * A POSIX file description.
 *
 * TODO this should be reference counted, and multiple file descriptors can
 * reference the same file description. File descriptor flags (close-on-exec)
 * don't go here.
 */
typedef struct posixfile_t {
    int fd;           // The POSIX file descriptor (TODO this must be moved out)
    unsigned handle;  // The underlying Onramp file or directory handle
    unsigned flags;   // The flags with which the file was opened
    bool is_dir;      // true if this is a directory
    bool std_stream;  // true if this is a standard stream (which on v2 are not closed)
    char* path;       // The path to the file
    off_t position;   // Current position in the file
    char* dirent;             // 256-byte buffer, exists only if there is a cached dirent
    bool dirent_overflowed;   // true if there is a cached dirent that overflowed
    bool dirent_eof;          // true if the last dirent was end of file
} posixfile_t;

static posixfile_t* posixfiles[32];

#define POSIXFILES_CAPACITY (int)(sizeof(posixfiles)/sizeof(*posixfiles))

static posixfile_t* posixfile_new(int fd, int handle, int flags, const char* path) {
    posixfile_t* posixfile = calloc(1, sizeof(posixfile_t));
    if (posixfile == NULL) {
        return NULL;
    }

    posixfile->fd = fd;
    posixfile->handle = handle;
    posixfile->flags = flags;
    posixfile->path = strdup(path);
    if (posixfile->path == NULL) {
        free(posixfile);
        return NULL;
    }

    return posixfile;
}

static void posixfile_delete(posixfile_t* posixfile) {
    // Note we delete without closing. This is called by close().
    free(posixfile->dirent);
    free(posixfile->path);
    free(posixfile);
}

void __io_init(void) {

    // By default we match POSIX
    input_echo = true;
    input_block = true;
    input_canonical = true;

    // Create POSIX file descriptors
    posixfiles[0] = posixfile_new(0, __process_info_table[__ONRAMP_PIT_INPUT], O_RDONLY, "/dev/stdin");
    posixfiles[1] = posixfile_new(1, __process_info_table[__ONRAMP_PIT_OUTPUT], O_WRONLY, "/dev/stdout");
    posixfiles[2] = posixfile_new(2, __process_info_table[__ONRAMP_PIT_ERROR], O_WRONLY, "/dev/stderr");

    // Mark the standard streams so we don't close them
    posixfiles[0]->std_stream = true;
    posixfiles[1]->std_stream = true;
    posixfiles[2]->std_stream = true;
}

void __io_destroy(void) {
    for (int fd = 0; fd < POSIXFILES_CAPACITY; ++fd) {
        if (posixfiles[fd] != NULL) {
            // We're deleting all files because we eventually want to build a
            // leak checker into the libc. On a quick exit we only need to
            // close the file descriptors.
            posixfile_delete(posixfiles[fd]);
        }
    }
}

int open(const char* path, int flags, ...) {

    // Make sure exactly one of O_RDONLY, O_WRONLY and O_RDWR was given
    if (((int)!!(flags & O_RDONLY) + (int)!!(flags & O_WRONLY) +
            (int)!!(flags & O_RDWR)) != 1)
    {
        errno = EINVAL; // invalid flags
        return -1;
    }
    bool writeable = (flags & O_WRONLY) || (flags & O_RDWR);

    // Check for other flag incompatibilities
    if ((flags & O_EXCL) && !(flags & O_CREAT)) {
        errno = EINVAL; // invalid flags
        return -1;
    }
    if ((flags & O_TRUNC) && (flags & O_RDONLY)) {
        errno = EINVAL; // invalid flags
        return -1;
    }

    // Find a free file descriptor
    size_t fd;
    for (fd = 0; fd < POSIXFILES_CAPACITY; ++fd) {
        if (posixfiles[fd] == 0) {
            break;
        }
    }
    if (fd == POSIXFILES_CAPACITY) {
        errno = ENFILE; // too many open files
        return -1;
    }

    // TODO normalize the path. should append it onto the working directory

    // If the file is being opened for creation only, we have to make sure the
    // file doesn't exist first. We do that by opening for reading.
    if ((flags & O_CREAT) && (flags & O_EXCL)) {
        int handle = __sys_open(path, false);
        if (handle >= 0) {
            // The file exists and O_EXCL was specified.
            if (0 != __sys_close(handle)) {
                // TODO we've leaked a file descriptor, this should panic.
                __fatal("Failed to close a file!");
            }
            errno = EEXIST;
            return -1;
        }

        // It is possible to specify (O_CREAT | O_EXCL | O_RDONLY). In this
        // case we have to open the file for writing to create it, but we'll
        // still store O_RDONLY in the file description flags so we won't allow
        // write() on it.
        writeable = true;
    }

    // Open it.
    int handle = __sys_open(path, !!writeable);
    if (handle < 0) {
        // TODO parse out the different error types.
        errno = EACCES;
        return -1;
    }

    // Create the wrapper
    posixfile_t* posixfile = posixfile_new(0, handle, flags, path);
    posixfiles[fd] = posixfile;
    posixfile->handle = handle;

    // TODO use stat to check the type if implemented. the stat syscall isn't specified yet; see #39.

    // TODO move directory stuff to a function in libc/3 dirent.c

    // Check if this could be a directory.
    bool maybe_directory = true;
    if ((flags & O_CREAT) && (flags & O_EXCL)) {
        maybe_directory = false;
    } else if (__process_info_table[__ONRAMP_PIT_VERSION] < 4) {
        maybe_directory = false;
    } else if (!__syscall_is_supported(__SYS_DIRENT)) {
        maybe_directory = false;
    }

    // Make a dirent call to check if it's a directory.
    if (maybe_directory) {
        char dirent_buffer[256];
        int ret = __sys_dirent(handle, dirent_buffer);
        if (ret == 0 || ret == __ERROR_END_OF_FILE || ret == __ERROR_OVERFLOW) {

            // It's a directory!
            posixfile->is_dir = true;

            // If this was opened for writing, the VM should have refused to
            // open it, but in case it didn't we fail here.
            if (writeable) {
                close(fd);
                errno = EISDIR;
                return -1;
            }

            // Cache the result so we can return it from the next __dirent()
            if (ret == __ERROR_OVERFLOW) {
                posixfile->dirent_overflowed = true;
            } else if (ret == __ERROR_END_OF_FILE) {
                posixfile->dirent_eof = true;
            } else {
                // We need to store the dirent to be returned on the next
                // __dirent().
                posixfile->dirent = malloc(_NAME_MAX + 1);
                if (!posixfile->dirent) {
                    close(fd);
                    errno = ENOMEM;
                    return -1;
                }

                // We're trusting that the VM has properly null-terminated the
                // buffer.
                strcpy(posixfile->dirent, dirent_buffer);
            }
            return fd;
        }
    }

    // It's not a directory. Make sure O_DIRECTORY was not specified.
    if (flags & O_DIRECTORY) {
        close(fd);
        errno = EISDIR;
        return -1;
    }

    // Change the mode (if the file was created)
    if (0/*TODO !stat_ok*/) {
        va_list args;
        va_start(args, flags);
        mode_t mode = va_arg(args, mode_t);
        va_end(args);
        if (0 != chmod(path, mode)) {
            (void)__sys_close(handle);
            errno = EACCES; // failed to change file mode
            return -1;
        }
    }

    // Truncate
    if (flags & O_TRUNC) {
        // TODO handle trunc not supported, it's supposed to be optional. We
        // should try to delete the file first.
        if (__syscall_is_supported(__SYS_TRUNC)) {
            if (0 != __sys_trunc(handle, 0, 0)) {
                (void)__sys_close(handle);
                errno = EIO; // failed to truncate
                return -1;
            }
        }
    }

    return fd;
}

int creat(const char* path, mode_t mode) {
    return open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
}

int close(int fd) {
    if (fd < 0 || fd >= POSIXFILES_CAPACITY || posixfiles[fd] == NULL) {
        errno = EBADF; // no such file descriptor
        return -1;
    }
    posixfile_t* posixfile = posixfiles[fd];

    // We don't close the standard streams.
    // TODO this should only be done on v2 VMs, but before we fix it we need to
    // proxy child syscalls. In libc/2 we need to reference count file
    // descriptors; in other parts of the bootstrap we need to proxy close and
    // ignore it on the standard streams.
    if (!posixfile->std_stream) {
        // v2/v3 had a dclose syscall for directories. In v4 it's just close.
        if (posixfile->is_dir &&
                __process_info_table[__ONRAMP_PIT_VERSION] < 4 &&
                __syscall_is_supported(__SYS_DCLOSE))
        {
            if (0 != __sys_dclose(posixfile->handle)) {
                // TODO we've leaked a file descriptor, this should panic.
                __fatal("Failed to dclose a directory!");
            }
        } else {
            if (0 != __sys_close(posixfile->handle)) {
                // TODO we've leaked a file descriptor, this should panic.
                __fatal("Failed to close a file!");
            }
        }
    }

    posixfile_delete(posixfile);
    posixfiles[fd] = NULL;
    return 0;
}

static off_t __fd_size_v3(int fd) {
    posixfile_t* posixfile = posixfiles[fd];

    if (posixfile->is_dir) {
        errno = EISDIR;
        return -1;
    }

    // seek to the end
    int ret = __sys_fseek(posixfile->handle, SEEK_END, 0, 0);
    if (ret != 0) {
        // TODO for now we assume stream isn't seekable
        errno = ESPIPE;
        return -1;
    }

    // get the size
    #ifdef __onramp_abi_bootstrap__
    unsigned result[2]; // TODO will have to malloc this to build with cci/0
    #endif
    #ifndef __onramp_abi_bootstrap__
    off_t result;
    #endif
    off_t size = 0;

    ret = __sys_size(posixfile->handle, (void*)&result);
    switch (ret) {
        case 0:
            break;
        default:
            // TODO other errors. assume it's a pipe
            errno = ESPIPE;
            size = -1;
    }

    #ifdef __onramp_abi_bootstrap__
    if (size == 0 && (result[1] != 0 || (int)result[0] < 0)) {
        errno = EOVERFLOW;
        size = -1;
    }
    if (size == 0) {
        size = result[0];
    }
    #endif

    #ifndef __onramp_abi_bootstrap__
    if (result < 0) {
        errno = EOVERFLOW;
        size = -1;
    }
    if (size == 0) {
        size = result;
    }
    #endif

    // seek back
    ret = __sys_fseek(posixfile->handle, SEEK_SET,
            (uint32_t)posixfile->position,
            (uint32_t)((posixfile->position >> 16) >> 16)); // shift twice for when off_t is 32 bits
    if (ret != 0) {
        // we're in trouble, we successfully seeked but we can't seek back!
        // TODO we should probably set a flag to put this fd in an error state
        // and prevent any further I/O on it
        errno = ESPIPE;
        return -1;
    }

    return size;
}

static off_t __fd_size_v4(int fd) {
    posixfile_t* posixfile = posixfiles[fd];

    if (posixfile->is_dir) {
        errno = EISDIR;
        return -1;
    }

    #ifdef __onramp_abi_bootstrap__
    unsigned result[2]; // TODO will have to malloc this to build with cci/0
    #endif
    #ifndef __onramp_abi_bootstrap__
    off_t result;
    #endif

    int ret = __sys_size(posixfile->handle, (void*)&result);
    switch (ret) {
        case 0:
            break;
        default:
            // TODO other errors. assume it's a pipe
            errno = ESPIPE;
            return -1;
    }

    #ifdef __onramp_abi_bootstrap__
    if (result[1] != 0 || (int)result[0] < 0) {
        errno = EOVERFLOW;
        return -1;
    }
    return result[0];
    #endif

    #ifndef __onramp_abi_bootstrap__
    if (result < 0) {
        errno = EOVERFLOW;
        return -1;
    }
    return result;
    #endif
}

off_t __fd_size(int fd) {
    // TODO this is copy-pasted a bunch of places, make it a function, also
    // because we need to allow callers to set arbitrary ids with dup2 so we
    // will need a more sophisticated id table
    if (fd < 0 || fd >= POSIXFILES_CAPACITY || posixfiles[fd] == NULL) {
        errno = EBADF; // no such file descriptor
        return -1;
    }

    if (__process_info_table[__ONRAMP_PIT_VERSION] < 4) {
        return __fd_size_v3(fd);
    }
    return __fd_size_v4(fd);
}

static off_t lseek_v3(posixfile_t* posixfile, off_t offset, int whence) {

    // perform the seek
    int ret = __sys_fseek(posixfile->handle,
            whence == SEEK_SET ? 0 : whence == SEEK_CUR ? 1 : 2,
            (unsigned)offset,
            #ifdef __onramp_abi_bootstrap__
            0
            #endif
            #ifndef __onramp_abi_bootstrap__
            (unsigned)(offset >> 32)
            #endif
            );
    if (ret != 0) {
        // TODO for now we assume stream isn't seekable
        errno = ESPIPE;
        return -1;
    }

    // for SEEK_SET the offset is the new position
    if (whence == SEEK_SET) {
        posixfile->position = offset;
        return offset;
    }

    // otherwise we have to query the position
    #ifdef __onramp_abi_bootstrap__
    unsigned result[2]; // TODO will have to malloc this to build with cci/0
    #endif
    #ifndef __onramp_abi_bootstrap__
    off_t result;
    #endif

    ret = __sys_ftell(posixfile->handle, (void*)&result);
    if (ret != 0) {
        // TODO for now we assume stream isn't seekable
        errno = ESPIPE;
        return -1;
    }

    #ifdef __onramp_abi_bootstrap__
    if (result[1] != 0 || (int)result[0] < 0) {
        errno = EOVERFLOW;
        return -1;
    }
    posixfile->position = result[0];
    return result[0];
    #endif
    #ifndef __onramp_abi_bootstrap__
    posixfile->position = result;
    return result;
    #endif
}

static off_t lseek_v4(posixfile_t* posixfile, off_t offset, int whence) {
    switch (whence) {
        case SEEK_SET:
            break;
        case SEEK_CUR:
            offset += posixfile->position;
            break;
        case SEEK_END: {
            off_t size = __fd_size(posixfile->fd);
            if (size < 0) {
                // __fd_size() set errno
                return -1;
            }
            offset += size;
            break;
        }
        default:
            // TODO unreachable
    }

    if (offset == posixfile->position) {
        return offset;
    }

    int ret = __sys_seek(posixfile->handle,
            (unsigned)offset,
            (unsigned)((offset >> 16) >> 16)); // shift twice for when off_t is 64 bits

    // TODO POSIX requires support for setting an out-of-bounds file position;
    // a subsequent write causes the space to be zero-filled. Our VM spec
    // allows this behaviour for syscall seek but does not require it. We need
    // to catch cases where the seek fails, store the position and set an
    // out-of-bounds flag. On a read, if the flag is set, EOF. On a write, if
    // the flag is set, we seek to the end, zero-fill the padding, then proceed
    // with the write.

    // TODO we need a function to convert a syscall error to errno
    switch (ret) {
        case 0:
            break;
        default:
            // TODO other errors. assume it's a pipe.
            errno = ESPIPE;
            return -1;
    }

    posixfile->position = offset;
    return offset;
}

off_t lseek(int fd, off_t offset, int whence) {
    if (whence < 0 || whence > 2) {
        errno = EINVAL;
        return -1;
    }

    if (fd < 0 || fd >= POSIXFILES_CAPACITY || posixfiles[fd] == NULL) {
        errno = EBADF; // no such file descriptor
        return -1;
    }
    posixfile_t* posixfile = posixfiles[fd];

    if (whence == SEEK_CUR && offset == 0) {
        // TODO not if file is not seekable
        return posixfile->position;
    }

    if (__process_info_table[__ONRAMP_PIT_VERSION] < 4) {
        return lseek_v3(posixfile, offset, whence);
    }
    return lseek_v4(posixfile, offset, whence);
}

ssize_t read(int fd, void* buffer, size_t count) {
    if (fd < 0 || fd >= POSIXFILES_CAPACITY || posixfiles[fd] == NULL) {
        errno = EBADF; // no such file descriptor
        return -1;
    }
    posixfile_t* posixfile = posixfiles[fd];

    if (posixfile->is_dir) {
        errno = EISDIR; // this is a directory
        return -1;
    }

    if (posixfile->flags & O_WRONLY) {
        errno = EINVAL; // this is not readable
        return -1;
    }

    int result;
    for (;;) {
        result = __sys_read(posixfile->handle, buffer, count);

        // If the VM input is non-blocking but the program wants blocking and
        // we received ERROR_TRY_LATER (or 0 for backwards compatibility), we
        // block internally until we get data.
        // TODO check if program has called fcntl(O_NONBLOCK)
        if (((result == 0 && posixfile->std_stream && input_block) || (result == __ERROR_TRY_LATER)) &&
                !(__process_info_table[__ONRAMP_PIT_CAPABILITIES] & __ONRAMP_CAPABILITIES_INPUT_BLOCKING))
        {
            continue;
        }

        // If we received 0 on a blocking stream, we treat it as EOF for
        // backwards compatibility. POSIX read() returns 0 on EOF.
        if (result == __ERROR_END_OF_FILE) {
            result = 0;
            break;
        }
        if (result >= 0) {
            break;
        }

        switch (result) {
            case __ERROR_END_OF_FILE:
                result = 0; // in POSIX zero indicates end-of-file
                break;
            case __ERROR_TRY_LATER:
                errno = EWOULDBLOCK; // non-blocking, read would block
                return -1;
            case __ERROR_UNSUPPORTED:
                errno = EINVAL; // device does not support reading
                return -1;
            case __ERROR_GENERIC:
            case __ERROR_IO:
            default:
                errno = EIO; // io error
                return -1;
        }

        break;
    }
    posixfile->position += (unsigned)result;

    // If the VM input doesn't echo but the program wants echo, we echo
    // ourselves.
    // TODO handle escape sequences; backspace, arrow keys in canonical mode
    // TODO this should be moved into a separate function, and we need to check the interactive bit in v4
    if (input_echo && posixfile->std_stream &&
                !(__process_info_table[__ONRAMP_PIT_CAPABILITIES] & __ONRAMP_CAPABILITIES_INPUT_ECHO))
    {
        // Loop until all the data is written.
        size_t remaining = result;
        while (remaining > 0) {
            int step = __sys_write(__process_info_table[__ONRAMP_PIT_OUTPUT], buffer, remaining);
            if (step == __ERROR_TRY_LATER) {
                continue;
            }
            if (step <= 0) {
                break;
            }
            remaining -= step;
            buffer = (char*)buffer + step;
            continue;
        }
    }

    return result;
}

ssize_t write(int fd, const void* buffer, size_t count) {
    if (fd < 0 || fd >= POSIXFILES_CAPACITY || posixfiles[fd] == NULL) {
        errno = EBADF; // no such file descriptor
        return -1;
    }
    posixfile_t* posixfile = posixfiles[fd];

    if (posixfile->is_dir || (posixfile->flags & O_RDONLY)) {
        errno = EBADF; // this is not writeable
        return -1;
    }

    int result;
    for (;;) {
        result = __sys_write(posixfile->handle, buffer, count);

        if (result > 0) {
            break;
        }

        if (result == __ERROR_TRY_LATER) {
            // TODO: don't loop if fcntl(O_NONBLOCK) was called
            continue;
        }

        if (result == 0) {
            // TODO: v4 VMs don't allow a zero return value. For backwards
            // compatibility with v2/v3 we interpret it as an I/O error.
            errno = EIO; // io error
            return -1;
        }

        switch (result) {
            case __ERROR_END_OF_FILE:
                errno = EPIPE; // broken pipe
                return -1;
            case __ERROR_GENERIC:
            case __ERROR_IO:
            default:
                errno = EIO; // io error
                return -1;
        }
    }

    posixfile->position += (unsigned)result;
    return result;
}

// TODO move to libc/3, need to move posixfile_t to header
int __dirent(int fd, char name[256]) {
    if (fd < 0 || fd >= POSIXFILES_CAPACITY || posixfiles[fd] == NULL) {
        errno = EBADF; // no such file descriptor
        return -1;
    }
    posixfile_t* posixfile = posixfiles[fd];

    if (!posixfile->is_dir) {
        errno = EBADF;
        return -1;
    }

    // If we have a cached dirent, return it.

    if (posixfile->dirent_overflowed) {
        posixfile->dirent_overflowed = false;
        errno = EOVERFLOW;
        return -1;
    }

    if (posixfile->dirent_eof) {
        return 0;
    }

    if (posixfile->dirent) {
        strcpy(name, posixfile->dirent);
        free(posixfile->dirent);
        posixfile->dirent = 0;
        return 1;
    }

    // Otherwise we need to make a syscall.
    int ret = __sys_dirent(posixfile->handle, name);
    if (ret == 0) {
        // For backwards compatibility we treat a blank name as end of
        // directory.
        if (*name == 0) {
            return 0;
        }

        // TODO parse out ".", ".."

        return 1;
    }

    if (ret > 0) {
        // TODO panic, nonsense return value
        __fatal("syscall dirent returning positive value!");
    }

    switch (ret) {
        case __ERROR_END_OF_FILE:
            // must not set errno (see readdir())
            return 0;

        case __ERROR_OVERFLOW:
            errno = EOVERFLOW;
            return -1;

        case __ERROR_UNSUPPORTED:
            // Something went wrong; we should have recognized that this is not
            // a directory when we opened it.
            errno = ENOTSUP;
            return -1;

        case __ERROR_IO:
        case __ERROR_GENERIC: // assume it's an I/O error
        default:
            errno = EIO;
            return -1;
    }

    // unreachable
}

int chmod(const char* path, mode_t mode) {

    // We only support the user executable bit.
    if (mode & S_IXUSR) {
        mode = 0755;
    } else {
        mode = 0644;
    }

    if (__sys_chmod(path, mode) < 0) {
        // TODO parse out error codes
        errno = EACCES;
    }
    return 0;
}

int fchmod(int fd, mode_t mode) {
    if (fd < 0 || fd >= POSIXFILES_CAPACITY || posixfiles[fd] == NULL) {
        errno = EBADF; // no such file descriptor
        return -1;
    }
    posixfile_t* posixfile = posixfiles[fd];
    return chmod(posixfile->path, mode);
}

int fcntl(int fd, int command, ...) {
    if (fd != STDIN_FILENO) {
        errno = EBADF; // can only modify terminal state of stdin
        return -1;
    }

    if (command == F_GETFL) {
        // TODO we need to also return the O_ACCMODE flags. for now we don't.
        // only non-blocking is implemented so far.
        return (input_block ? 0 : O_NONBLOCK);
    }

    if (command == F_SETFL) {
        va_list args;
        va_start(args, command);
        int flags = va_arg(args, mode_t);
        va_end(args);
        input_block = !(flags & O_NONBLOCK);
        return 0;
    }

    // unrecognized command
    errno = EINVAL;
    return -1;
}

int tcgetattr(int fd, struct termios* termios) {
    if (fd != STDIN_FILENO) {
        errno = EBADF; // can only modify terminal state of stdin
        return -1;
    }
    termios->c_lflag =
        (input_echo ? ECHO : 0) |
        (input_canonical ? ICANON : 0);
    return 0;
}

int tcsetattr(int fd, int actions, const struct termios* termios) {
    if (actions != TCSANOW) {
        errno = EINVAL; // TODO no other actions are supported
        return -1;
    }
    if (fd != STDIN_FILENO) {
        errno = EBADF; // can only modify terminal state of stdin
        return -1;
    }
    input_echo = termios->c_lflag & ECHO;
    input_canonical = termios->c_lflag & ICANON;
    return 0;
}

int unlink(const char* path) {
    if (!__syscall_is_supported(__SYS_DELETE)) {
        errno = EIO;
        return -1;
    }

    int ret = __sys_delete(path);
    if (ret == 0) {
        return 0;
    }

    // TODO more precise error codes
    errno = EIO;
    return -1;
}

int rmdir(const char* path) {
    if (!__syscall_is_supported(__SYS_RMDIR)) {
        errno = EIO;
        return -1;
    }

    // TODO we need to check that this is actually a directory

    int ret;
    if (__process_info_table[__ONRAMP_PIT_VERSION] < 4 &&
            __syscall_is_supported(__SYS_RMDIR))
    {
        ret = __sys_rmdir(path);
    } else {
        ret = __sys_delete(path);
    }
    if (ret == 0) {
        return 0;
    }

    // TODO more precise error codes
    errno = EIO;
    return -1;
}
