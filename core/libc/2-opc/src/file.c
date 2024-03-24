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
#include "syscalls.h"

#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
            #include <__onramp/__pit.h>
            char* itoa_d(int value, char* buffer);

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
    // properties
    int fd;              // The underlying file descriptor, or -1 if none
    int flags;           // Flags of the underlying file descriptor
    char* path;          // The path to the file, or NULL if none
    bool wide;           // true if using wide character functions, false otherwise

    // state
    bool eof;
    bool error;

    // buffer
    int buffer_mode;     // buffering mode: _IOFBF (full), _IOLBF (line) or _IONBF (none)
    size_t buffer_size;  // size of buffer
    char* buffer;        // pointer to start of buffer
    size_t buffer_pos;   // current read/write position relative to start of buffer
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

// TODO for now buffering disabled
file->buffer_mode = _IONBF;
file->buffer_size = 0;

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

void __file_init(void) {
    stdin = fdopen(0, "r");
    stdout = fdopen(1, "w");
    stderr = fdopen(2, "w");

    // We can't tell whether the standard streams are attached to terminals so
    // we assume they are.
    setvbuf(stdin, NULL, _IOFBF, BUFSIZ);
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
    setvbuf(stderr, NULL, _IONBF, 0);
}

void __file_destroy(void) {
    while (all_files) {
        fclose(all_files);
    }
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

    if (file->buffer_read > 0) {
        // TODO for now we ignore flush calls on read. We need to seek
        // backwards on files to discard any buffered data, but we also need to
        // keep buffered data for streams otherwise we would lose data on
        // flush. We consider Onramp to have exclusive access to any open files
        // and any concurrent external changes are undefined behaviour so for
        // now we don't bother.
        return 0;
    }

    if (file->buffer_pos == 0) {
        // No data is buffered.
        return 0;
    }

    // We have a write buffer with data in it. Loop to write it out.
    bool error = false;
    char* start = file->buffer;
    char* end = file->buffer + file->buffer_pos;
    while (start != end) {
        ssize_t step = write(file->fd, start, end - start);
        if (step <= 0) {
            if (step < 0)
                error = true;
            break;
        }
        start += step;
    }

    if (start != end) {
        // We have data we couldn't flush. We want to preserve any data we
        // failed to write so we move it back to the start of the buffer. (This
        // might be a good reason to use a circular buffer instead. Currently
        // we don't bother.)
        memmove(file->buffer, start, start - end);

        // If error is true, we presume write() set errno for us.
        if (!error) {
            // It should not be possible for `write()` to return 0 when given
            // a non-zero number of bytes. It should have either written
            // successfully or returned -1. As a fallback we raise EIO.
            errno = EIO;
        }

        file->error = true;
        return EOF;
    }

    file->buffer_pos = 0;
    return 0;
}

int fflush(FILE* file) {
    if (file) {
        return fflush_impl(file);
    }

    // fflush(NULL) means flush all output files.
    int ret = 0;
    for (file = all_files; file; file = file->next) {
        if (file->fd == -1) {
            // skip any file without a valid file descriptor
            continue;
        }
        if (file->buffer_read > 0) {
            // skip any file with buffered input data
            continue;
        }

        int error = fflush_impl(file);

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
// TODO for now buffering disabled
return 0;
    int ret = fflush(file);

    // A zero size buffer is equivalent to unbuffered.
    if (size == 0) {
        mode = _IONBF;
    }

    // Check whether we need to replace the buffer.
    // (If the flush failed, we should still discard the current buffer because
    // it might be a user buffer that will cease to exist.)
    if (ret != 0 || 
            mode != file->buffer_mode ||
            size != file->buffer_size ||
            buffer != file->buffer)
    {
        // Reset things in case flush failed.
        if (file->buffer_owned) {
            // TODO this doesn't make sense, there's no reason to discard an
            // internal buffer, we aren't even changing the size
            free(file->buffer);
        } else {
            file->buffer_size = BUFSIZ;
        }
        file->buffer = NULL;
        file->buffer_pos = 0;
    }

    if (ret != 0) {
        // If flush failed, we return failure here, so we don't actually use
        // the given buffer.
        return ret;
    }

    if (mode == _IONBF) {
        // Ignore the given buffer and size.
        file->buffer = NULL;
        file->buffer_size = 0;
    } else {
        file->buffer = buffer;
        file->buffer_size = size;
    }
    file->buffer_mode = mode;
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
    char x;
    if (1 == fread(&x, 1, 1, file)) {
        return x;
    }
    return EOF;
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
    return fgetc(file);
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

// Sets up a buffer.
static bool buffer_setup(FILE* file) {
    if (file->buffer_size < BUFSIZ) {
        file->buffer_size = BUFSIZ;
    }
    assert(file->buffer_size > 0);

    file->buffer = malloc(file->buffer_size);
    if (file->buffer == NULL) {
        file->error = true;
        return false;
    }

    return true;
}

size_t fread(void* restrict vout, size_t element_size, size_t element_count,
        FILE* restrict file)
{
    char* out = (char*)vout;

    if (element_size == 0 || element_count == 0)
        return 0;

    // The file must be open for reading.
    if (!(file->flags & (O_RDONLY | O_RDWR))) {
        return 0;
    }

    // If we have unbuffered writes we need to flush.
    if (file->buffer_pos > 0 && file->buffer_read == 0) {
        if (0 != fflush(file)) {
            return EOF;
        }
    }

    // Currently we ignore the buffering mode when reading. Onramp needs
    // exclusive access to open files; it's undefined behaviour for them to be
    // modified externally.
    if (file->buffer == NULL) {
        if (!buffer_setup(file)) {
            return EOF;
        }
    }

    // TODO we could make some attempt at reading in increments of the element
    // size. For now we don't bother.
    size_t requested = element_count;
    if (element_size > 1) {
        // TODO overflow check
        requested *= element_size;
    }

    // copy what's left in the buffer
    size_t copied = file->buffer_read - file->buffer_pos;
    if (requested < copied)
        copied = requested;
    memcpy(out, file->buffer + file->buffer_pos, copied);
    file->buffer_pos += copied;

    size_t left = requested - copied;
    size_t total = copied;
    bool error = false;

    if (left >= file->buffer_size) {
        // we still need at least as many bytes as fit into the buffer. read
        // directly to the output.
        ssize_t step = read(file->fd, out + copied, left);
        if (step > 0) {
            total += step;
        } else if (step < 0) {
            error = true;
        }
    } else if (left > 0) {
        // we need less bytes than fit into the buffer. let's try to fill the
        // buffer and copy from it again.
        ssize_t step = read(file->fd, file->buffer, file->buffer_size);
        if (step > 0) {
            if ((ssize_t)left > step)
                left = (size_t)step;
            memcpy(out + copied, file->buffer, left);
            file->buffer_read = step;
            file->buffer_pos = left;
        } else if (step < 0) {
            error = true;
        }
    }

    if (total == 0) {
        // We only set the error or eof flags if we read no bytes. If there
        // were bytes in the buffer we always return success even if the
        // attempt to read more failed.
        if (error) {
            file->error = true;
        } else {
            file->eof = true;
        }
    }

    if (element_size == 1)
        return total;
    return total / element_size;
}

size_t fwrite(const void* restrict vdata, size_t element_size, size_t element_count,
        FILE* restrict file)
{
    const char* restrict data = (const char*)vdata;

    if (element_size == 0 || element_count == 0)
        return 0;

    // The file must be open for writing.
    if (!(file->flags & (O_WRONLY | O_RDWR))) {
        return 0;
    }

    // If we have unbuffered reads, discard it. (We could also just error here
    // because it's unspecified behaviour if a read is followed by a write
    // without a seek.)
    if (file->buffer_read > 0) {
        file->buffer_pos = 0;
        file->buffer_read = 0;
    }

    // TODO try to handle partial element writes more intelligently. We could
    // for example only copy whole elements in the buffer. For now we don't
    // bother.
    size_t requested = element_count;
    if (element_size > 1) {
        // TODO overflow check
        requested *= element_size;
    }

    size_t written = 0;
    if (file->buffer_mode != _IONBF) {

        // Setup the buffer if needed
        if (file->buffer == NULL) {
            if (!buffer_setup(file)) {
                return EOF;
            }
        }

        // Write what we can to the buffer
        written = file->buffer_size - file->buffer_pos;
        if (written > requested)
            written = requested;
        memcpy(file->buffer + file->buffer_pos, data, written);

        // If we still have more, flush the buffer
        if (written < requested) {
            if (0 != fflush_impl(file)) {
                // On a failure to flush we return an error. We have no way to
                // know if any of our data made it out. (We could change this
                // to flush manually so we could tell but does it matter?)
                file->error = true;
                return 0;
            }
        }
    }

    while (written < requested) {

        // See if we can fit the rest in the buffer.
        size_t left = requested - written;
        if (left <= file->buffer_size) {
            memcpy(file->buffer, data + written, left);
            written = requested;
            file->buffer_pos = left;
            break;
        }

        // Write directly to the output.
        ssize_t step = write(file->fd, data + written, left);
        if (step <= 0) {
            break;
        }
        written += step;
    }

    // If we are line buffered and had a line feed anywhere in the input, we do
    // a final flush.
    if (file->buffer_mode == _IOLBF && memchr(data, '\n', requested)) {
        if (-1 == fflush(file)) {
            // As above, on a failure to flush we error.
            file->error = true;
            return 0;
        }
    }

    // We only set the error flag if we failed to write anything at all. If we
    // were able to write some, we return success for what we wrote even if a
    // subsequent write call failed.

    if (written == 0) {
        file->error = true;
        return 0;
    }

    if (element_size == 1)
        return written;
    return written / element_size;
}

int fgetpos(FILE* restrict file, fpos_t* restrict pos) {
    // TODO
    return -1;
}

int fseek(FILE* file, long offset, int whence) {
    if (0 != fflush_impl(file)) {
        // fllush() set errno and the error flag for us.
        return -1;
    }

    // The buffer should now be empty.
    assert(file->buffer_pos == file->buffer_read);

    // We don't call lseek() here because our off_t is 64 bits which is not
    // available in opC. Instead we do the syscall manually.
    int ret = __sys_fseek(file->fd, whence, offset, 0);
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
    // TODO
    return -1;
}

long ftell(FILE* file) {
    unsigned position[2];

    // We don't call lseek() here because our off_t is 64 bits which is not
    // available in opC. Instead we do the syscall manually.
    int ret = __sys_ftell(file->fd, position);
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
