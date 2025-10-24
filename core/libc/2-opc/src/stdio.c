/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2025 Fraser Heavy Software
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
#include "syscalls.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

            #include <__onramp/__pit.h>
            char* itoa_d(int value, char* buffer);

FILE* stdin;
FILE* stdout;
FILE* stderr;

static FILE* all_files;

// TODO implement this somewhere
static inline void sched_yield(void) {}

static int fflush_write(FILE* file);

// helper for print debugging
//#define W(x) write(1, x, strlen(x))

/*
typedef enum {
    file_mode_read_only,
    file_mode_write_only,
    file_mode_read_write,
} file_mode_t;
*/

typedef enum buffer_use_t {
    buffer_use_none,    // buffer is not in use
    buffer_use_reading, // buffer is currently configured for reading
    buffer_use_writing, // buffer is currently configured for writing
} buffer_use_t;

/**
 * Our FILE structure.
 *
 * FILEs are lazily buffered. We store the requested buffer configuration and a
 * buffer is allocated as needed on a read/write call.
 *
 * The buffer can only be used for reading or writing, not both simultaneously.
 * If a file is opened for both, the buffer is flushed when switching between
 * reading and writing. This keeps the implementation simple.
 *
 * ungetc() is implemented by pushing data into the front of the buffer (which
 * may require flushing to make space.) Even if the buffering mode is _IONBF
 * (non-buffered), a buffer will still need to be allocated if ungetc() is
 * called.
 *
 * All FILEs are stored in a doubly-linked list. This is used to implement
 * fflush(NULL) which flushes all files.
 *
 * A FILE's file descriptor can be -1, meaning it is not backed by a file
 * descriptor. This can happen for example when freopen() fails.
 *
 * A FILE's path can also be NULL, for example if it was opened with fdopen()
 * or pipe().
 */
struct __file {
    // properties
    int fd;              // The underlying file descriptor, or -1 if none
    int flags;           // Flags of the underlying file descriptor
    char* path;          // The path to the file, or NULL if none
    bool wide;           // true if using wide character functions, false otherwise
    bool readable;       // true if the file was opened for reading
    bool writable;       // true if the file was opened for writing

    // state
    bool eof;
    bool error;

    // buffer
    int buffer_mode;          // mode: _IOFBF (full), _IOLBF (line) or _IONBF (none)
    size_t buffer_size;       // size of buffer in bytes
    char* buffer;             // start of buffer, or NULL if no buffer
    char* buffer_pos;         // current read/write position in buffer, or NULL if not in use
    char* buffer_end;         // end of buffered data (for reading) or empty space (for writing) or NULL
    char* buffer_ungetc;      // end of ungetc() characters in buffer, or NULL if ungetc() was not called
    buffer_use_t buffer_use;  // whether the buffer is in use for reading, writing or neither
    bool buffer_owned;        // whether we need to free the buffer on fclose()/setvbuf()

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

void __stdio_setup(void) {
    stdin = fdopen(0, "r");
    stdout = fdopen(1, "w");
    stderr = fdopen(2, "w");

    // We can't tell whether the standard streams are attached to terminals so
    // we assume they are.
    // TODO capabilities bits should tell us this.
    setvbuf(stdin, NULL, _IOFBF, BUFSIZ);
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
    setvbuf(stderr, NULL, _IONBF, 0);
}

void __stdio_teardown(void) {
    while (all_files) {
        fclose(all_files);
    }
}

int fclose_impl(FILE* file) {
    int ret = fflush_write(file);

    if (file->fd != -1) {
        int close_err = close(file->fd);
        if (close_err != 0 && ret == 0) {
            ret = close_err;
        }
    }

    return ret;
}

int fclose(FILE* file) {
    int ret = fclose_impl(file);
    file_delete(file);

    // The user might have closed a standard stream. If they did, null it out
    // so that other internal functions don't try to access them.
    if (file == stdin)  stdin = NULL;
    if (file == stdout) stdout = NULL;
    if (file == stderr) stderr = NULL;

    return ret;
}

/**
 * Writes all of the given data to the given file descriptor.
 *
 * Returns true if successful, false on failure. In case of failure, the error
 * from write() will be in errno.
 */
static bool write_all(int fd, const char* data, size_t count) {
    while (count > 0) {
        ssize_t step = write(fd, data, count);
        if (step <= 0) {
            if (step < 0) {
                return false;
            }
            // TODO currently we assume a return value of 0 is an I/O error.
            // This is incomplete; we should be able to handle non-blocking
            // output streams and EOF on a closed output stream.
            errno = EIO;
            return false;
        }
        data += step;
        count -= step;
    }
    return true;
}

static int fflush_impl(FILE* file) {
    if (file->fd == -1) {
        // It's possible for a FILE to have no underlying file descriptor, for
        // example if freopen() failed. In this case we're supposed to raise
        // EBADF.
        errno = EBADF;
        return -1;
        // (It's also possible to use fdopen() with a bogus file descriptor, or
        // to close() a file descriptor while a FILE is still using it. In
        // these cases the seek() or write() calls below will raise EBADF.)
    }

    if (file->buffer_use != buffer_use_writing) {
        // We don't flush a read buffer. (If we have input from stdin that
        // hasn't been consumed yet we need to keep it.)
        // TODO Linux discards the contents of seekable files. This might
        // matter for example if the file was edited externally and we want to
        // flush to refresh. We should try to seek back (if we're sure it's a
        // file) and if it succeeds discard.
        // TODO we need to discard ungetc() chars. We should take into account
        // buffer_ungetc when calculating the position to seek.
        return 0;
    }

    if (file->buffer_pos == file->buffer) {
        // No data is buffered.
        return 0;
    }

    // We have a write buffer with data in it. Write it out.
    if (!write_all(file->fd, file->buffer, file->buffer_pos - file->buffer)) {
        file->error = true;
        return EOF;
    }

    file->buffer_pos = file->buffer;
    return 0;
}

static int fflush_write(FILE* file) {
    if (file->buffer_use == buffer_use_writing) {
        return fflush_impl(file);
    }
    return 0;
}

int fflush(FILE* file) {
    if (file) {
        return fflush_impl(file);
    }

    // fflush(NULL) means flush all output files.
    int ret = 0;
    for (file = all_files; file; file = file->next) {
        int error = fflush_write(file);

        // Return the first error code we come across.
        if (ret == 0 && error != 0) {
            ret = error;
        }
    }
    return ret;
}

/*
 * Parses the given mode string, returning flags for the underlying POSIX
 * open() call.
 */
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
    // TODO we could consider having a capabilities flag for whether we should
    // translate CRLF to LF, that way we could set it on Windows. Probably
    // useless though.

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
    file->readable = (flags & O_RDONLY) || (flags & O_RDWR);
    file->writable = (flags & O_WRONLY) || (flags & O_RDWR);
    return file;
}

FILE* freopen(const char* restrict filename, const char* restrict mode, FILE* restrict file) {
    // TODO
    __fatal("freopen() not implemented yet");
    return NULL;
}

FILE* fdopen(int fd, const char* mode) {
    FILE* file = file_new();
    if (file == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    int flags = parse_mode(mode);

    file->fd = fd;
    file->flags = flags;
    file->readable = (flags & O_RDONLY) || (flags & O_RDWR);
    file->writable = (flags & O_WRONLY) || (flags & O_RDWR);
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

    // A zero size buffer is equivalent to unbuffered.
    if (size == 0) {
        mode = _IONBF;
    }

    // If unbuffered, ignore the given buffer.
    if (mode == _IONBF) {
        buffer = NULL;
    }

    // Flush so we can release the existing buffer.
    int ret = fflush_impl(file);
    if (ret != 0) {
        // If flush failed, we need to return failure, so we don't use the
        // given buffer. If we already have an unowned buffer, we stop using it
        // just in case the caller ignores the return value of this; we
        // otherwise don't change the buffering mode.
        if (file->buffer && !file->buffer_owned) {
            // leave the buffer mode as-is; just stop using the buffer
            file->buffer_size = BUFSIZ;
            file->buffer = NULL;
            file->buffer_pos = NULL;
            file->buffer_end = NULL;
            file->buffer_ungetc = NULL;
            file->buffer_use = buffer_use_none;
            file->buffer_owned = false;
        }
        return ret;
    }

    // Free the current buffer, if any
    // TODO could optimize to keep the existing buffer if it matches that
    // requested. For now we don't bother.
    if (file->buffer_owned) {
        free(file->buffer);
    }

    if (mode == _IONBF) {
        file->buffer_size = 0;
        file->buffer = NULL;
    } else if (buffer) {
        file->buffer_size = size;
        file->buffer = buffer;
    }

    file->buffer_mode = mode;
    file->buffer_pos = NULL;
    file->buffer_end = NULL;
    file->buffer_ungetc = NULL;
    file->buffer_use = buffer_use_none;
    file->buffer_owned = false;

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

    // Shortcut for buffering
    if (file->buffer_use == buffer_use_reading && file->buffer_pos != file->buffer_end) {
        return (unsigned char)*file->buffer_pos++;
    }

    // Fallback to fread()
    unsigned char x;
    if (1 == fread(&x, 1, 1, file)) {
        return x;
    }

    return EOF;
}

char* fgets(char* restrict s, int n, FILE* restrict file) {
    if (n <= 0) {
        // In the C23 spec 7.23.7.2, passing a negative or zero n to fgets() is
        // undefined behaviour. This wording is not in previous specs but we
        // assume it is true there as well.
        __fatal("A negative or zero value was passed as buffer size to fgets().");
    }

    char* p = s;
    char* end = p + n - 1;
    while (p != end) {
        int c = fgetc(file);
        if (c == EOF) {
            if (p == s) {
                // No characters have been read.
                return NULL;
            }
            break;
        }
        *p++ = c;
        if (c == '\n') {
            break;
        }
    }

    *p = 0;
    return s;
}

int fputc(int ic, FILE* file) {
    unsigned char c = (unsigned char)ic;

    // Shortcut for buffering
    if (file->buffer_use == buffer_use_writing && file->buffer_pos != file->buffer_end) {
        *file->buffer_pos++ = c;

        // If we wrote a line ending in line buffered mode we have to flush.
        if (c == '\n' && file->buffer_mode == _IOLBF) {
            if (0 != fflush_impl(file)) {
                return EOF;
            }
        }

        return c;
    }

    // Fallback to fwrite()
    if (1 != fwrite(&c, 1, 1, file)) {
        return EOF;
    }
    return (int)c;
}

int fputs(const char* restrict s, FILE* restrict file) {
    size_t len = strlen(s);
    if (len != fwrite(s, 1, len, file))
        return EOF;
    return 1;
}

int getc(FILE* file) {
    return fgetc(file);
}

int getchar(void) {
    return getc(stdin);
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

    // The characters pushed by ungetc are tracked by file->buffer_ungetc. If
    // file->buffer_ungetc is NULL or is at most file->buffer_pos, there are no
    // unread ungetc() characters. If file->buffer_ungetc is greater than
    // file->buffer_pos, the characters in between are those pushed by ungetc()
    // that have not yet been read.
    //
    // We keep track of them for two reasons:
    // - If we run out of space for ungetc(), we can move the previous ungetc()
    //   characters to the end of the buffer so we can push more;
    // - If the user calls fseek(), we should discard the ungetc() characters
    //   and not include them in the calculation of the previous stream
    //   position.

    // TODO:
    // - need a buffer in read mode. flush or allocate as needed. allocate BUFSIZ even if _IONBF
    // - if buffer_pos is null or start of buffer:
    //    - if buffer_ungetc is end of buffer, fail, we can't fit any more
    //    - if buffer_ungetc is not null, preserve it: memmove [buffer_pos,buffer_ungetc) to end of buffer
    //    - set buffer_ungetc to end of buffer
    //    - set buffer_pos to end of buffer minus previous ungetc data
    // - if buffer_ungetc is null, set it to buffer_pos so we can remember where ungetc ends
    // - push the character before buffer_pos and we're done
    return -1;
}

// Sets up a buffer.
static bool buffer_setup(FILE* file) {
    assert(file->buffer == NULL);

    if (file->buffer_size < BUFSIZ) {
        file->buffer_size = BUFSIZ;
    }

    file->buffer = (char*)malloc(file->buffer_size);
    if (file->buffer == NULL) {
        file->error = true;
        return false;
    }
    file->buffer_end = file->buffer + file->buffer_size;
    file->buffer_pos = file->buffer;
    file->buffer_owned = true;
    file->buffer_ungetc = NULL;
    file->buffer_use = buffer_use_none;

    return true;
}

size_t fread(void* restrict vout, size_t element_size, size_t element_count,
        FILE* restrict file)
{
    char* restrict out = (char*)vout;
    size_t total = element_size*element_count; // TODO check for overflow
    size_t remaining = total;

    // Make sure the file is readable
    if (!file->readable) {
        file->error = true;
        errno = EBADF;
        return 0;
    }

    // If we have any existing data in the buffer, take it.
    if (file->buffer_use == buffer_use_reading) {
        size_t step = file->buffer_end - file->buffer_pos;
        if (step > remaining) {
            step = remaining;
        }
        memcpy(out, file->buffer_pos, step);
        file->buffer_pos += step;
        out += step;
        remaining -= step;
    }
    if (remaining == 0) {
        return element_count;
    }

    // We'll need to read(). Flush standard output and error streams before
    // reading from standard input.
    if (file == stdin) {
        // The user might have closed these.
        if (stdout) {
            fflush_impl(stdout);
        }
        if (stderr) {
            fflush_impl(stderr);
        }
    }

    // Configure buffering
    if (file->buffer_mode != _IONBF) {

        // Allocate the buffer if it isn't already
        if (file->buffer == NULL) {
            if (!buffer_setup(file)) {
                // file->error is set
                return 0;
            }
        }

        // Switch the buffer to read mode
        if (file->buffer_use != buffer_use_reading) {
            if (fflush_impl(file) != 0) {
                return 0;
            }
            file->buffer_use = buffer_use_reading;
            file->buffer_pos = file->buffer + file->buffer_size;
            file->buffer_end = file->buffer_pos;
            file->buffer_ungetc = NULL;
        }
    }

    // If the amount requested is still larger than the buffer size, read
    // chunks directly into the output.
    while (remaining > file->buffer_size) {
        ssize_t step = read(file->fd, out, remaining);
        if (step <= 0) {
            if (step < 0) {
                if (errno == EWOULDBLOCK) {
                    sched_yield();
                    continue;
                }
                file->error = true;
            } else {
                file->eof = true;
            }
            return (total - remaining) / element_size;
        }
        out += step;
        remaining -= (size_t)step;
    }

    // The remaining amount is less than the buffer size. We try to fill the
    // buffer and pull data from it until we're done. This needs to be a loop
    // because the read call could return much less than the capacity of the
    // buffer, even less than we requested.
    while (remaining != 0) {

        // Fill the buffer as much as we can.
        ssize_t result = read(file->fd, file->buffer, file->buffer_size);
        if (result <= 0) {
            if (result < 0) {
                if (errno == EWOULDBLOCK) {
                    sched_yield();
                    continue;
                }
                file->error = true;
            } else {
                file->eof = true;
            }
            return (total - remaining) / element_size;
        }
        file->buffer_pos = file->buffer;
        file->buffer_end = file->buffer + result;

        // Take data from the buffer.
        size_t step = (size_t)result;
        if (step > remaining) {
            step = remaining;
        }
        memcpy(out, file->buffer, step);
        file->buffer_pos += step;
        out += step;
        remaining -= step;
    }

    return element_count;
}

size_t fwrite(const void* restrict vdata, size_t element_size, size_t element_count,
        FILE* restrict file)
{
    const char* restrict data = (const char*)vdata;
    size_t total = element_size*element_count; // TODO check for overflow
    size_t remaining = total;

    // Make sure the file is writable
    if (!file->writable) {
        file->error = true;
        errno = EBADF;
        return 0;
    }

    // Handle buffering
    if (file->buffer_mode != _IONBF) {

        // Allocate the buffer if it isn't already
        if (file->buffer == NULL) {
            if (!buffer_setup(file)) {
                // file->error is set
                return 0;
            }
        }

        // Switch the buffer to write mode
        if (file->buffer_use != buffer_use_writing) {
            if (fflush_impl(file) != 0) {
                return 0;
            }
            file->buffer_use = buffer_use_writing;
            file->buffer_pos = file->buffer;
            file->buffer_end = file->buffer + file->buffer_size;
            file->buffer_ungetc = NULL;
        }

        // Copy as much data as we can to the buffer (as long as it doesn't
        // contain a newline in line-buffered mode)
        size_t step = file->buffer_end - file->buffer_pos;
        if (step > remaining) {
            step = remaining;
        }
        if (file->buffer_mode != _IOLBF || !memchr(data, '\n', step)) {
            memcpy(file->buffer_pos, data, step);
            file->buffer_pos += step;
            data += step;
            remaining -= step;
        }
        if (remaining == 0) {
            return element_count;
        }

        // We still have data left. Flush to empty the buffer.
        if (0 != fflush_impl(file)) {
            // We don't know how much was written and how much was lost. The
            // file is in an error state.
            return 0;
        }

        // If the remaining data now fits in the buffer (and it doesn't contain
        // a line ending in line-buffered mode) copy it in.
        if (remaining <= file->buffer_size &&
                (file->buffer_mode != _IOLBF || !memchr(data, '\n', remaining)))
        {
            memcpy(file->buffer_pos, data, remaining);
            file->buffer_pos += remaining;
            return element_count;
        }
    }

    // If we have any data left, it's because either we're unbuffered or the
    // data didn't fit in the buffer. Write it out directly.
    // TODO we should probably do this like fread(), loop until the data fits
    // in the buffer
    if (remaining > 0) {
        if (!write_all(file->fd, data, remaining)) {
            // TODO need to differentiate between IO error (e.g. out of space)
            // and a closed output stream. For now we assume writing should
            // never fail.
            file->error = true;
            return (total - remaining) / element_size;
        }
    }
    return element_count;
}

int fgetpos(FILE* restrict file, fpos_t* restrict pos) {
    long x = ftell(file);
    if (x < 0)
        return -1;
    *pos = x;
    return 0;
}

int fseek(FILE* file, long offset, int whence) {

    // We only flush if we have a write buffer. If we're reading, flush() would
    // cause a seek, but we're about to seek anyway.
    if (0 != fflush_write(file)) {
        // fllush() set errno and the error flag for us.
        return -1;
    }

    if (file->buffer_use == buffer_use_reading) {

        // We have to offset SEEK_CUR by the amount of data remaining in the buffer.
        if (whence == SEEK_CUR && file->buffer_use == buffer_use_reading) {
            long new_offset = offset - (file->buffer_end - file->buffer_pos);
            if (new_offset > offset) {
                errno = EOVERFLOW;
                return -1;
            }
            offset = new_offset;
        }

        // Dump the remaining buffer contents.
        file->buffer_use = buffer_use_none;
        file->buffer_pos = NULL;
        file->buffer_end = NULL;
        file->buffer_ungetc = NULL;
    }

    // We don't call lseek() here because our off_t is 64 bits which is not
    // available in opC. Instead we do the syscall manually.
    int ret = __sys_fseek(__fd_handle(file->fd),
            whence == SEEK_SET ? 0 : whence == SEEK_CUR ? 1 : 2,
            offset,
            (offset < 0) ? UINT32_MAX : 0);
    if (ret < 0) {
        // TODO convert Onramp error codes. For now we assume the stream isn't
        // seekable.
        // TODO the VM is allowed to forbid seeking beyond the end of the file,
        // in which case (if the file is writeable) we need to extend it
        // manually, and maybe lazily.
        errno = ESPIPE;
        return -1;
    }

    return 0;
}

int fsetpos(FILE* file, const fpos_t* pos) {
    return fseek(file, *pos, SEEK_SET);
}

long ftell(FILE* file) {
    unsigned position[2];

    // We don't call lseek() here because our off_t is 64 bits which is not
    // available in opC. Instead we do the syscall manually.
    int ret = __sys_ftell(__fd_handle(file->fd), position);
    if (ret < 0) {
        // TODO convert Onramp error codes. For now we assume the stream isn't
        // seekable.
        errno = ESPIPE;
        return -1;
    }

    if (position[1] != 0 || position[0] > (unsigned)INT_MAX) {
        errno = EOVERFLOW;
        return -1;
    }

    // For a write buffer, add the pending buffer contents. For a read buffer,
    // subtract the remaining buffer contents.
    if (file->buffer_use == buffer_use_writing) {
        unsigned new_pos = position[0] + (file->buffer_pos - file->buffer);
        if (new_pos < position[0]) {
            errno = EOVERFLOW;
            return -1;
        }
        position[0] = new_pos;
    } else if (file->buffer_use == buffer_use_reading) {
        unsigned new_pos = position[0] - (file->buffer_end - file->buffer_pos);
        if (new_pos > position[0]) {
            errno = EOVERFLOW;
            return -1;
        }
        position[0] = new_pos;
    }

    return position[0];
}

void rewind(FILE* file) {
    // TODO
}

void clearerr(FILE* file) {
    // TODO
}

int feof(FILE* file) {
    return file->eof;
}

int ferror(FILE* file) {
    return file->error;
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

int fileno(FILE* file) {
    if (file->fd < 0) {
        errno = EBADF;
        return -1;
    }
    return file->fd;
}
