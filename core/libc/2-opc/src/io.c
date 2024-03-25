/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 Fraser Heavy Software
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

#include <__onramp/__pit.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "internal.h"
#include "syscalls.h"

/**
 * This implements low-level POSIX file I/O (e.g. open(), write(), etc.) The C
 * file API (e.g. fopen(), fwrite(), etc.) in libc/2 and libc/3 wraps this.
 */

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
    posixfiles[0] = posixfile_new(0, __process_info_table[__ONRAMP_PIT_INPUT], O_RDONLY, "/dev/stdin");
    posixfiles[1] = posixfile_new(1, __process_info_table[__ONRAMP_PIT_OUTPUT], O_WRONLY, "/dev/stdout");
    posixfiles[2] = posixfile_new(2, __process_info_table[__ONRAMP_PIT_ERROR], O_WRONLY, "/dev/stderr");

    // mark the standard streams so we don't close them
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
        if (0 != __sys_ftrunc(handle, 0, 0)) {
            (void)__sys_fclose(handle);
            errno = EIO; // failed to truncate
            return -1;
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

    int result = __sys_fread(posixfile->handle, buffer, count);
    if (result < 0) {
        // TODO parse out the result code
        errno = EIO; // io error
        return -1;
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
// TODO syscall fwrite doesn't set r0 yet, need to fix some code, see c-debugger
result = count;
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
