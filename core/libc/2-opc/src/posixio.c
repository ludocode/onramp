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
 * A POSIX file descriptor.
 */
typedef struct posixfile_t {
    int fd;           // The POSIX file descriptor
    unsigned handle;  // The underlying Onramp file or directory handle
    unsigned flags;   // The flags with which the file was opened
    bool is_dir;      // true if this is a directory
    bool std_stream;  // true if this is a standard stream (which must not be closed)
    char* path;       // The path to the file
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
    // Note we delete without closing. We don't close the standard
    // input/output/error streams; other files must be closed manually.
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

int __fd_handle(int fd) {
    if (fd < 0 || fd >= POSIXFILES_CAPACITY || posixfiles[fd] == NULL) {
        errno = EBADF; // no such file descriptor
        return -1;
    }
    return posixfiles[fd]->handle;
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

    // Collect information about the existing path now. We'll need it for
    // various checks.
// TODO stat is not implemented yet. Most of these checks are disabled for now.
/*
    struct stat buffer;
    bool stat_ok = 0 == stat(path, buffer);

    // If the user didn't specify O_CREAT, the path must exist.
    if (!(flags & O_CREAT)) {
        if (!stat_ok) {
            errno = ENOENT; // no such file
            return -1;
        }
    }

    // If the user requested exclusive create, writing is required, and the
    // path must not exist.
    if ((flags & O_CREAT) && (flags & O_EXCL)) {
        if (!writeable) {
            errno = EINVAL; // invalid flags
            return -1;
        }
        if (stat_ok) {
            errno = EEXIST; // file cannot exist
            return -1;
        }
    }
*/
    bool is_dir = 0/*stat_ok && stat.st_mode & S_IFDIR*/;
    if (is_dir && writeable) {
        errno = EISDIR; // cannot open a directory writeable
        return -1;
    }
    if ((flags & O_DIRECTORY) && !is_dir) {
        errno = ENOTDIR; // not a directory
        return -1;
    }

    // The behaviour of O_TRUNC with O_RDONLY or with a directory is
    // unspecified. We consider it an error.
    if ((flags & O_TRUNC) && ((flags & O_RDONLY) || is_dir)) {
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

    // Open it. If the file exists and is a directory, we need to use dopen
    // instead of fopen.
    int handle;
    if (is_dir) {
        handle = __sys_dopen(path);
    } else {
        handle = __sys_fopen(path, !!writeable);
    }
    if (handle < 0) {
        // TODO parse out the different error types.
        errno = EACCES;
        return -1;
    }

    // Change the mode (if the file was created)
    if (0/*TODO !stat_ok*/) {
        va_list args;
        va_start(args, flags);
        mode_t mode = va_arg(args, mode_t);
        va_end(args);
        if (0 != chmod(path, mode)) {
            (void)__sys_fclose(handle);
            errno = EACCES; // failed to change file mode
            return -1;
        }
    }

    // Truncate
    if (flags & O_TRUNC) {
        // TODO handle ftrunc not supported, it's supposed to be optional. We
        // should try to delete the file first.
        if (__syscall_is_supported(__SYS_FTRUNC)) {
            if (0 != __sys_ftrunc(handle, 0, 0)) {
                (void)__sys_fclose(handle);
                errno = EIO; // failed to truncate
                return -1;
            }
        }
    }

    // Create the wrapper
    posixfile_t* posixfile = posixfile_new(0, handle, flags, path);
    posixfile->is_dir = is_dir;
    posixfiles[fd] = posixfile;
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
        if (posixfile->is_dir) {
            __sys_dclose(posixfile->handle);
        } else {
            __sys_fclose(posixfile->handle);
        }
    }

    posixfile_delete(posixfile);
    posixfiles[fd] = NULL;
    return 0;
}

// lseek() requires long long for off_t. We don't use it during bootstrapping
// so we only need this when the final toolchain is rebuilt. We don't bother to
// put it in libc/3.
#ifndef __onramp_cci_opc__
off_t lseek(int fd, off_t offset, int whence) {
    if (fd < 0 || fd >= POSIXFILES_CAPACITY || posixfiles[fd] == NULL) {
        errno = EBADF; // no such file descriptor
        return -1;
    }
    posixfile_t* posixfile = posixfiles[fd];

    // perform the seek (if necessary)
    if (whence != SEEK_CUR || offset != 0) {
        int ret = __sys_fseek(posixfile->handle,
                whence == SEEK_SET ? 0 : whence == SEEK_CUR ? 1 : 2,
                (unsigned)offset, (unsigned)(offset >> 32));
        if (ret != 0) {
            // TODO for now we assume stream isn't seekable
            errno = ESPIPE;
            return -1;
        }
    }

    // return the current position
    off_t result;
    int ret = __sys_ftell(posixfile->handle, (void*)&result);
    if (ret != 0) {
        // TODO for now we assume stream isn't seekable
        errno = ESPIPE;
        return -1;
    }
    return result;
}
#endif

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
        result = __sys_fread(posixfile->handle, buffer, count);
        if (result < 0) {
            // TODO parse out the result code
            errno = EIO; // io error
            return -1;
        }

        // If the VM input is non-blocking but the program wants blocking and
        // we received no data, we block internally until we get data.
        // TODO check if program has called fcntl(O_NONBLOCK)
        if (result == 0 && posixfile->std_stream && input_block &&
                !(__process_info_table[__ONRAMP_PIT_CAPABILITIES] & __ONRAMP_CAPABILITIES_INPUT_BLOCKING))
        {
            #ifndef __onramp_libc_opc__
            usleep(10000);
            #endif
            continue;
        }

        break;
    }

    // If the VM input doesn't echo but the program wants echo, we echo
    // ourselves.
    // TODO handle escape sequences; backspace, arrow keys in canonical mode
    if (result > 0 && posixfile->std_stream && input_echo &&
                !(__process_info_table[__ONRAMP_PIT_CAPABILITIES] & __ONRAMP_CAPABILITIES_INPUT_ECHO))
    {
        __sys_fwrite(__process_info_table[__ONRAMP_PIT_OUTPUT], buffer, result);
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
        errno = EINVAL; // this is not writeable
        return -1;
    }

    int result = __sys_fwrite(posixfile->handle, buffer, count);
    if (result < 0) {
        // TODO parse out the result code
        errno = EIO; // io error
        return -1;
    }

    return result;
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
    if (!__syscall_is_supported(__SYS_UNLINK)) {
        errno = EIO;
        return -1;
    }

    int ret = __sys_unlink(path);
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

    int ret = __sys_rmdir(path);
    if (ret == 0) {
        return 0;
    }

    // TODO more precise error codes
    errno = EIO;
    return -1;
}
