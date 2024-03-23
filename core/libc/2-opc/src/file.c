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

/*
 * This file contains the low-level FILE* implementation, along with various
 * other filesystem functions.
 */

        // TODO add #undef in cpp/1 to simplify this
        #define __ONRAMP_LIBC_2_FILE_IMPL
#include <stdio.h>

#include "internal.h"

#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

FILE* stdin;
FILE* stdout;
FILE* stderr;

static FILE* all_files;

/*
typedef enum {
    file_mode_read_only,
    file_mode_write_only,
    file_mode_read_write,
} file_mode_t;
*/

/**
 * Our FILE structure.
 *
 * FILEs are lazily buffered. We store the requested buffer configuration and a
 * buffer is allocated as needed on a read/write call.
 *
 * All FILEs are stored in a doubly-linked list. This is used to implement
 * fflush(NULL) which flushes all files.
 *
 * A FILE's file descriptor can be -1, meaning it is not backed by a file
 * descriptor. This can happen for example when freopen() fails.
 *
 * A FILE's path can also be NULL, for example if it was opened with fdopen().
 */
struct __file {
    int fd;              // The underlying file descriptor, or -1 if none
    int flags;           // Flags of the underlying file descriptor
    char* path;          // The path, or NULL if none
    bool wide;           // true if using wide character functions, false otherwise

    // buffer
    int buffer_mode;     // buffering mode: _IOFBF (full), _IOLBF (line) or _IONBF (none)
    size_t buffer_size;  // size of buffer
    char* buffer;        // pointer to start of buffer
    size_t buffer_pos;   // current read/write position in buffer
    size_t buffer_read;  // data available for reading from buffer, or 0 if writing
    bool buffer_owned;   // whether we need to free the buffer on fclose()/setvbuf()

    // linked list
    FILE* previous;
    FILE* next;
};

static FILE* file_new(void) {
    FILE* file = calloc(1, sizeof(FILE));
    file->fd = -1;

    // files start out fully buffered by default.
    file->buffer_mode = _IOFBF;
    file->buffer_size = BUFSIZ;

    // insert into linked list
    if (all_files) {
        all_files->previous = file;
    }
    file->next = all_files;
    all_files = file;

    return file;
}

static void file_delete(FILE* file) {

    // remove file from linked list
    if (file->next) {
        file->next->previous = file->previous;
    }
    if (file->previous) {
        file->previous->next = file->next;
    } else {
        all_files = file->next;
    }

    if (file->buffer_owned) {
        free(file->buffer);
    }
    free(file->path);
    free(file);
}

void __stdio_init(void) {
    stdin = fdopen(0, "r");
    stdout = fdopen(1, "w");
    stderr = fdopen(2, "w");

    // We can't tell whether the standard streams are attached to terminals so
    // we assume they are.
    setvbuf(stdin, NULL, _IOFBF, BUFSIZ);
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
    setvbuf(stderr, NULL, _IONBF, 0);
}

void __stdio_destroy(void) {
}

int fclose_impl(FILE* file) {
    if (file->fd != -1) {
        return close(file->fd);
    }
    return 0;
}

int fclose(FILE* file) {
    int ret = fclose_impl(file);
    file_delete(file);
    return ret;
}

static int fflush_impl(FILE* file) {
    // TODO
    return 0;
}

int fflush(FILE* file) {
    if (file) {
        return fflush_impl(file);
    }

    // fflush(NULL) means flush all files.
    int ret = 0;
    for (file = all_files; file; file = file->next) {
        int error = fflush_impl(file);

        // Return the first error code we come across.
        if (ret == 0 && error != 0) {
            ret = error;
        }
    }
    return ret;
}

static int parse_mode(const char* mode) {
    bool r = false;
    bool w = false;
    bool a = false;
    bool p = false;
    bool b = false;
    bool x = false;

    for (; *mode; ++mode) {
        switch (*mode) {
            case 'r':
                if (r) return 0;
                r = true;
                break;
            case 'w':
                if (w) return 0;
                w = true;
                break;
            case 'a':
                if (a) return 0;
                a = true;
                break;
            case '+':
                if (p) return 0;
                p = true;
                break;
            case 'b':
                if (b) return 0;
                b = true;
                break;
            case 'x':
                if (x) return 0;
                x = true;
                break;
            default:
                return 0;
        }
    }

    // Exactly one of 'r', 'w' and 'a' must be provided
    if ((int)r + (int)w + (int)a != 1) {
        return 0;
    }

    // 'x' requires 'w'
    if (x && !w) {
        return 0;
    }

    // 'b' is ignored.

    if (r && p) {
        return O_RDWR;
    }
    if (w && p) {
        return O_RDWR | O_CREAT | O_TRUNC;
    }
    if (a && p) {
        return O_RDWR | O_CREAT | O_APPEND;
    }
    if (r) {
        return O_RDONLY;
    }
    if (w) {
        return O_WRONLY | O_CREAT | O_TRUNC;
    }
    if (a) {
        return O_WRONLY | O_CREAT | O_APPEND;
    }

    // This should be unreachable.
    __fatal("Internal error: fopen() mode parsing failed.\n");
}

FILE* fopen(const char* restrict filename, const char* restrict mode) {
    int flags = parse_mode(mode);
    if (flags == 0)
        return NULL;

    int fd = open(filename, flags, 0644);
    if (fd == -1) {
        // errno is set by open()
        return NULL;
    }

    FILE* file = file_new();
    if (file == NULL) {
        close(fd);
        errno = ENOMEM;
        return NULL;
    }

    file->path = strdup(filename);
    if (file->path == NULL) {
        file_delete(file);
        close(fd);
        errno = ENOMEM;
        return NULL;
    }

    file->fd = fd;
    file->flags = flags;
    return file;
}

FILE* freopen(const char* restrict filename, const char* restrict mode, FILE* restrict file) {
    // TODO
    return NULL;
}

FILE* fdopen(int fd, const char* mode) {
    FILE* file = file_new();
    if (file == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    file->fd = fd;
    file->flags = parse_mode(mode);
    return file;
}

/*
 * According to the specs, the buffering mode can only be changed before the
 * first read or write call. This means the buffer has to be allocated lazily.
 *
 * As an extension we support changing the buffering mode even after the first
 * read/write. The only real difference in implementation is that setvbuf() has
 * to flush first. This flush can fail so it's not completely trivial but it's
 * not a big deal.
 *
 * On a call to setvbuf() (or any of its wrappers), we flush and free the
 * current buffer (if necessary), and then we store the requested buffer mode,
 * size and external buffer if given. On a read/write, if we're buffered and
 * the buffer is null, we allocate it.
 */
int setvbuf(FILE* restrict file, char* restrict buffer, int mode, size_t size) {
    int ret = fflush(file);

    // Check whether we need to replace the buffer.
    // (If the flush failed, we should discard the current buffer either way
    // because it might be a user buffer that will cease to exist.)
    if (ret != 0 || 
            mode != file->buffer_mode ||
            size != file->buffer_size ||
            buffer != file->buffer)
    {
        // Reset things in case flush failed.
        if (file->buffer_owned) {
            free(file->buffer);
        } else {
            file->buffer_size = BUFSIZ;
        }
        file->buffer = NULL;
        file->buffer_pos = 0;
    }

    if (ret != 0) {
        return ret;
    }

    file->buffer = buffer;
    file->buffer_mode = mode;
    file->buffer_size = size;
    return 0;
}



/*
 * Wrappers
 */

void setbuf(FILE* restrict file, char* restrict buffer) {
    setvbuf(file, buffer, buffer ? _IOFBF : _IONBF, BUFSIZ);
}

void setbuffer(FILE* restrict file, char* buffer, size_t size) {
    setvbuf(file, buffer, buffer ? _IOFBF : _IONBF, size);
}

void setlinebuf(FILE *file) {
    setvbuf(file, NULL, _IOLBF, 0);
}

char* gets(char* s) {
    return fgets(s, INT_MAX, stdin);
}

int fgetc(FILE* file) {
    // TODO
    return -1;
}

char* fgets(char* restrict s, int n, FILE* restrict file) {
    // TODO
    return s;
}

int fputc(int ic, FILE* file) {
    unsigned char c = (unsigned char)ic;
    if (1 != fwrite(&c, 1, 1, file))
        return EOF;
    return (int)c;
}

int fputs(const char* restrict s, FILE* restrict file) {
    size_t len = strlen(s);
    if (len != fwrite(s, 1, len, file))
        return EOF;
    return 1;
}

int getc(FILE* file) {
    // TODO
    return -1;
}

int getchar(void) {
    // TODO
    return -1;
}

int putc(int c, FILE* file) {
    return fputc(c, stdout);
}

int putchar(int c) {
    return putc(c, stdout);
}

int puts(const char* s) {
    if (fputs(s, stdout) == EOF)
        return EOF;
    if (putchar('\n') == EOF)
        return EOF;
    return 1;
}

int ungetc(int c, FILE* file) {
    // TODO
    return -1;
}

size_t fread(void* restrict ptr, size_t size, size_t nmemb, FILE* restrict file) {
    // TODO
    return -1;
}

                        #include <__onramp/__pit.h>
                        #include "syscalls.h"
size_t fwrite(const void* restrict ptr, size_t size, size_t nmemb, FILE* restrict file) {
    // TODO check for overflow
    size_t total = size * nmemb;
    size_t remaining = total;

__sys_fwrite(__process_info_table[__ONRAMP_PIT_OUTPUT], ptr, total);
return total;

    while (remaining > 0) {

        int ret = write(file->fd, ptr, remaining);
        if (ret == -1) {
            if (errno == EINTR)
                continue;
            break;
        }
        remaining -= (size_t)ret;
    }

    return (total - remaining) / size;
}

int fgetpos(FILE* restrict file, fpos_t* restrict pos) {
    // TODO
    return -1;
}

int fseek(FILE* file, long offset, int whence) {
    // TODO
    return -1;
}

int fsetpos(FILE* file, const fpos_t* pos) {
    // TODO
    return -1;
}

long ftell(FILE* file) {
    // TODO
    return -1;
}

void rewind(FILE* file) {
    // TODO
}

void clearerr(FILE* file) {
    // TODO
}

int feof(FILE* file) {
    // TODO
    return -1;
}

int ferror(FILE* file) {
    // TODO
    return -1;
}

void perror(const char* s) {
    // TODO
}

int remove(const char* filename) {
    // TODO
    return -1;
}

int rename(const char* old, const char* new) {
    // TODO
    return -1;
}

FILE* tmpfile(void) {
    // TODO
    return NULL;
}

char* tmpnam(char* s) {
    // TODO
    return NULL;
}
