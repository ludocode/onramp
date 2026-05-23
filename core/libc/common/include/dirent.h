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

#ifndef __ONRAMP_LIBC_DIRENT_H_INCLUDED
#define __ONRAMP_LIBC_DIRENT_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

#include <__onramp/__name_max.h>
#include <features.h>
#include <sys/types.h>

#define DT_UNKNOWN 0
#define DT_BLK 1
#define DT_CHR 2
#define DT_DIR 3
#define DT_FIFO 4
#define DT_LNK 5
#define DT_REG 6
#define DT_SOCK 7

// glibc macros for detecting contents of struct dirent
#define _DIRENT_HAVE_D_RECLEN
#define _DIRENT_HAVE_D_OFF
#define _DIRENT_HAVE_D_TYPE

struct __dir;
typedef struct __dir DIR;

/*
 * A directory entry.
 *
 * Arch man page: https://man.archlinux.org/man/core/man-pages/dirent.h.0p.en
 * FreeBSD man page: https://man.freebsd.org/cgi/man.cgi?query=dirent
 *
 * (We are avoiding implementing all the extensions for now; we're sticking
 * with POSIX until a program needs it.)
 */
struct dirent {
    ino_t d_ino;

    // linux and bsd extensions. see if we can avoid implementing these.
    #if 0
    off_t d_off;
    unsigned char d_type;
    reclen_t d_reclen;
    unsigned char d_namlen;
    #endif

    // d_name is supposed to have indeterminate size. Some programs may
    // incorrectly rely on it having a fixed size so we may need to change
    // this.
    // (Maybe we should define it this way only if _GNU_SOURCE/_DEFAULT_SOURCE
    // is defined or -fgnu-extensions is enabled.)
    // TODO for now just use the fixed size
    char d_name[_NAME_MAX + 1];
    //char d_name[];
};

DIR* opendir(const char* name);
int closedir(DIR* dir);
struct dirent* readdir(DIR* dir);
void seekdir(DIR* dir, long location);
long telldir(DIR* dir);
void rewinddir(DIR* dir);

DIR* fdopendir(int fd);
int dirfd(DIR* dir);

int scandir(const char* restrict dir,
            struct dirent*** restrict /*out*/ namelist,
            int (*filter)(const struct dirent *),
            int (*compare)(const struct dirent **, const struct dirent **));

#ifdef DISABLED
// gnu extension, not yet implemented
int scandirat(int fd, const char* restrict dir,
            struct dirent*** restrict /*out*/ namelist,
            int (*filter)(const struct dirent *),
            int (*compare)(const struct dirent **, const struct dirent **));
#endif

int alphasort(const struct dirent** left, const struct dirent** right);

#ifdef DISABLED
// gnu extension, not yet implemented
int versionsort(const struct dirent** left, const struct dirent** right);
#endif

//[[deprecated]] // TODO attribute
int readdir_r(DIR* restrict dir,
        struct dirent* restrict entry,
        struct dirent** restrict result);

#endif
