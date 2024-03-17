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

#ifndef __ONRAMP_LIBC_STDIO_H_INCLUDED
#define __ONRAMP_LIBC_STDIO_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

#include <__onramp/__size_t.h>
#include <__onramp/__null.h>
#include <__onramp/__va_list.h>

struct __file;
typedef struct __file FILE;
typedef unsigned int fpos_t;

#define _IOFBF 1
#define _IOLBF 2
#define _IONBF 3

#define BUFSIZ 128
#define EOF (-1)
#define FOPEN_MAX 13 // TODO
#define FILENAME_MAX 256
#define L_tmpnam 256 // TODO
#define TMP_MAX 1000 // TODO

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 3

extern FILE* __stdin;
extern FILE* __stdout;
extern FILE* __stderr;
#define stdin (__stdin)
#define stdout (__stdout)
#define stderr (__stderr)

// standard C

int remove(const char* filename);
int rename(const char* old, const char* new);
FILE* tmpfile(void);
char* tmpnam(char* s);
int fclose(FILE* stream);
int fflush(FILE* stream);
FILE* fopen(const char* restrict filename, const char* restrict mode);
FILE* freopen(const char* restrict filename, const char* restrict mode, FILE* restrict stream);
int setvbuf(FILE* restrict stream, char* restrict buf, int mode, size_t size);

int fprintf(FILE* restrict stream, const char* restrict format, ...);
int fscanf(FILE* restrict stream, const char* restrict format, ...);
int printf(const char* restrict format, ...);
int scanf(const char* restrict format, ...);
int snprintf(char* restrict buffer, size_t n, const char* restrict format, ...);
int sprintf(char* restrict buffer, const char* restrict format, ...);
int sscanf(const char* restrict buffer, const char* restrict format, ...);
int vfprintf(FILE* restrict stream, const char* restrict format, va_list arg);
int vfscanf(FILE* restrict stream, const char* restrict format, va_list arg);
int vprintf(const char* restrict format, va_list arg);
int vscanf(const char* restrict format, va_list arg);
int vsnprintf(char* restrict buffer, size_t n, const char* restrict format, va_list arg);
int vsprintf(char* restrict buffer, const char* restrict format, va_list arg);
int vsscanf(const char* restrict buffer, const char* restrict format, va_list arg);

//[[deprecated]] char* gets(char* s);
int fgetc(FILE* stream);
char* fgets(char* restrict s, int n, FILE* restrict stream);
int fputc(int c, FILE* stream);
int fputs(const char* restrict s, FILE* restrict stream);
int getc(FILE* stream);
int getchar(void);
int putc(int c, FILE* stream);
int putchar(int c);
int puts(const char* s);
int ungetc(int c, FILE* stream);

size_t fread(void* restrict ptr, size_t size, size_t nmemb, FILE* restrict stream);
size_t fwrite(const void* restrict ptr, size_t size, size_t nmemb, FILE* restrict stream);
int fgetpos(FILE* restrict stream, fpos_t*  restrict pos);
int fseek(FILE* stream, long offset, int whence);
int fsetpos(FILE* stream, const fpos_t* pos);
long ftell(FILE* stream);
void rewind(FILE* stream);

void clearerr(FILE* stream);
int feof(FILE* stream);
int ferror(FILE* stream);
void perror(const char* s);

// gnu extensions
#ifdef _GNU_SOURCE
int asprintf(char** restrict out_string, const char* restrict format, ...);
int vasprintf(char** restrict out_string, const char* restrict format, va_list args);
#endif

#endif
