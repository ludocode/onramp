/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Fraser Heavy Software
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

#include <dirent.h>

#include "internal.h"

#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

struct __dir {
    int fd;
    struct dirent* dirent;
};

// TODO use offsetof(d_name)+_NAME_MAX+1 instead of sizeof so we can remove
// the buffer size of d_name
#define DIRENT_SIZE (sizeof(struct dirent))

DIR* opendir(const char* name) {
    int fd = open(name, O_RDONLY | O_DIRECTORY);
    if (fd < 0) {
        // open() set errno
        return NULL;
    }

    DIR* dir = fdopendir(fd);
    if (!dir) {
        errno = ENOMEM;
        return NULL;
    }

    dir->dirent = malloc(DIRENT_SIZE);
    if (dir->dirent == NULL) {
        free(dir);
        errno = ENOMEM;
        return NULL;
    }

    return dir;
}

int closedir(DIR* dir) {
    free(dir->dirent);
    return 0;
}

struct dirent* readdir(DIR* dir) {
    int ret = __dirent(dir->fd, dir->dirent->d_name);
    if (ret == 1) {
        return dir->dirent;
    }
    if (ret == 0) {
        // end of directory; must not change errno
        return NULL;
    }
    // __dirent() set errno
    return NULL;
}

int readdir_r(DIR* restrict dir,
        struct dirent* restrict entry,
        struct dirent** restrict result)
{
    // readdir_r() cannot change errno, but we need to set it to 0 to detect
    // end-of-directory from readdir(). We'll restore it afterwards.
    int old_errno = errno;
    errno = 0;

    // Calling readdir() defeats the purpose of making readdir_r() re-entrant.
    // We can fix this if we add preemptive multitasking later.
    struct dirent* dirent = readdir(dir);

    int new_errno = errno;
    errno = old_errno;

    if (dirent != NULL) {
        // Success
        memcpy(entry, dirent, DIRENT_SIZE);
        *result = entry;
        return 0;
    }

    // POSIX doesn't specify what we should do to result if we return non-zero
    // (error). We don't set it.

    switch (new_errno) {
        case 0:
            // End of directory; errno remains unchanged
            *result = NULL;
            return 0;

        case EOVERFLOW:
            return ENAMETOOLONG;
        default:
            break;
    }
    return new_errno;
}

#ifdef DISABLED
// TODO these are not implemented yet
void seekdir(DIR* dir, long location) {
}
long telldir(DIR* dir) {
}
void rewinddir(DIR* dir) {
}
#endif

DIR* fdopendir(int fd) {
    // TODO check if this is actually a directory

    DIR* dir = malloc(sizeof(DIR));
    if (dir == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    dir->dirent = malloc(offsetof(struct dirent, d_name) + _NAME_MAX + 1);
    if (dir->dirent == NULL) {
        errno = ENOMEM;
        free(dir);
        return NULL;
    }

    dir->fd = fd;
    return dir;
}

int dirfd(DIR* dir) {
    return dir->fd;
}

#ifdef DISABLED
int scandir(const char* restrict dir,
            struct dirent*** restrict /*out*/ namelist,
            int (*filter)(const struct dirent *),
            int (*compare)(const struct dirent **, const struct dirent **))
{
    fatal("scandir() not yet implemented");
}

int scandirat(int fd, const char* restrict dir,
            struct dirent*** restrict /*out*/ namelist,
            int (*filter)(const struct dirent *),
            int (*compare)(const struct dirent **, const struct dirent **))
{
    fatal("scandirat() not yet implemented");
}

int alphasort(const struct dirent** left, const struct dirent** right) {
    fatal("alphasort() not yet implemented");
}

int versionsort(const struct dirent** left, const struct dirent** right) {
    fatal("versionsort() not yet implemented");
}
#endif
