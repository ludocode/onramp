#include <stdio.h>

#include "internal.h"

#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

// FILE
struct __file {
    int fd;
    char buffer[128];
    //...
};

FILE __stdin = {.fd = 0};
FILE __stdout = {.fd = 1};
FILE __stderr = {.fd = 2};

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

int fclose(FILE* stream) {
    // TODO
    return -1;
}

int fflush(FILE* stream) {
    // TODO
    return -1;
}

FILE* fopen(const char* restrict filename, const char* restrict mode) {
    // TODO
    return NULL;
}

FILE* freopen(const char* restrict filename, const char* restrict mode, FILE* restrict stream) {
    // TODO
    return NULL;
}

int setvbuf(FILE* restrict stream, char* restrict buf, int mode, size_t size) {
    // TODO
    return -1;
}

char* gets(char* s) {
    // TODO maybe we should not implement this function at all? We certainly
    // won't use it. Do we care about being able to build software that uses
    // it?
    return fgets(s, INT_MAX, stdin);
}

int fgetc(FILE* stream) {
    // TODO
    return -1;
}

char* fgets(char* restrict s, int n, FILE* restrict stream) {
    // TODO
    return s;
}

int fputc(int ic, FILE* stream) {
    unsigned char c = (unsigned char)ic;
    if (1 != fwrite(&c, 1, 1, stream))
        return EOF;
    return (int)c;
}

int fputs(const char* restrict s, FILE* restrict stream) {
    size_t len = strlen(s);
    if (len != fwrite(s, 1, len, stream))
        return EOF;
    return 1;
}

int getc(FILE* stream) {
    // TODO
    return -1;
}

int getchar(void) {
    // TODO
    return -1;
}

int putc(int c, FILE* stream) {
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

int ungetc(int c, FILE* stream) {
    // TODO
    return -1;
}

size_t fread(void* restrict ptr, size_t size, size_t nmemb, FILE* restrict stream) {
    // TODO
    return -1;
}

size_t fwrite(const void* restrict ptr, size_t size, size_t nmemb, FILE* restrict stream) {
    // TODO check for overflow
    size_t total = size * nmemb;
    size_t remaining = total;

    while (remaining > 0) {
        int ret = write(stream->fd, ptr, remaining);
        if (ret == -1) {
            if (errno == EINTR)
                continue;
            break;
        }
        remaining -= (size_t)ret;
    }

    return (total - remaining) / size;
}

int fgetpos(FILE* restrict stream, fpos_t* restrict pos) {
    // TODO
    return -1;
}

int fseek(FILE* stream, long offset, int whence) {
    // TODO
    return -1;
}

int fsetpos(FILE* stream, const fpos_t* pos) {
    // TODO
    return -1;
}

long ftell(FILE* stream) {
    // TODO
    return -1;
}

void rewind(FILE* stream) {
    // TODO
}

void clearerr(FILE* stream) {
    // TODO
}

int feof(FILE* stream) {
    // TODO
    return -1;
}

int ferror(FILE* stream) {
    // TODO
    return -1;
}

void perror(const char* s) {
    // TODO
}
