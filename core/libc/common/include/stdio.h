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



// FILE is an incomplete struct so that it's not convertible to anything else.
// (omC doesn't have structs so it's just void when bootstrapping cci/1.)

#ifdef __onramp_cci_omc__
typedef void FILE;
#endif

#ifndef __onramp_cci_omc__
struct __file;
typedef struct __file FILE;
#endif

// These are supposed to be macros. They also need to be real variables to be
// accessible from assembly and backwards-compatible with previous stages, but
// we can't define recursive macros with the omC preprocessor. We therefore
// use this workaround to define them.
extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;
#ifndef __ONRAMP_LIBC_2_FILE_IMPL
    #ifndef __onramp_cpp_omc__
        #define stdin ((FILE*)stdin)
        #define stdout ((FILE*)stdout)
        #define stderr ((FILE*)stderr)
    #endif
#endif



typedef long fpos_t;

#define _IOFBF 1
#define _IOLBF 2
#define _IONBF 3

#define BUFSIZ 512
#define EOF (-1)
#define FOPEN_MAX 13 // TODO
#define FILENAME_MAX 256
#define L_tmpnam 256 // TODO
#define TMP_MAX 1000 // TODO

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2



// omC

FILE* fopen(const char* restrict filename, const char* restrict mode);
size_t fread(void* restrict ptr, size_t element_size, size_t element_count, FILE* restrict file);
size_t fwrite(const void* restrict ptr, size_t element_size, size_t element_count, FILE* restrict file);
int fclose(FILE* file);
int feof(FILE* file);

int fgetc(FILE* file);
int fputc(int c, FILE* file);
int fputs(const char* restrict s, FILE* restrict file);
int putchar(int c);
int putc(int c, FILE* file);
int puts(const char* s);

int fseek(FILE* file, long offset, int whence);
long ftell(FILE* file);

int remove(const char* filename);

int chmod(const char* filename, int mode); // TODO this belongs in sys/stat.h



// opC

#ifndef __onramp_cci_omc__

int rename(const char* old, const char* new);
FILE* tmpfile(void);
char* tmpnam(char* s);
int fflush(FILE* file);
FILE* freopen(const char* restrict filename, const char* restrict mode, FILE* restrict file);
int setvbuf(FILE* restrict file, char* restrict buf, int mode, size_t size);

int fprintf(FILE* restrict file, const char* restrict format, ...);
int fscanf(FILE* restrict file, const char* restrict format, ...);
int printf(const char* restrict format, ...);
int scanf(const char* restrict format, ...);
int snprintf(char* restrict buffer, size_t n, const char* restrict format, ...);
int sprintf(char* restrict buffer, const char* restrict format, ...);
int sscanf(const char* restrict buffer, const char* restrict format, ...);
int vfprintf(FILE* restrict file, const char* restrict format, va_list arg);
int vfscanf(FILE* restrict file, const char* restrict format, va_list arg);
int vprintf(const char* restrict format, va_list arg);
int vscanf(const char* restrict format, va_list arg);
int vsnprintf(char* restrict buffer, size_t n, const char* restrict format, va_list arg);
int vsprintf(char* restrict buffer, const char* restrict format, va_list arg);
int vsscanf(const char* restrict buffer, const char* restrict format, va_list arg);

//[[deprecated]] char* gets(char* s);
char* fgets(char* restrict s, int n, FILE* restrict file);
int getc(FILE* file);
int getchar(void);
int ungetc(int c, FILE* file);

int fgetpos(FILE* restrict file, fpos_t* restrict pos);
int fsetpos(FILE* file, const fpos_t* pos);
void rewind(FILE* file);

void clearerr(FILE* file);
int ferror(FILE* file);
void perror(const char* s);

// posix
FILE* fdopen(int fd, const char* mode);

// gnu extensions
#ifdef _GNU_SOURCE
int asprintf(char** restrict out_string, const char* restrict format, ...);
int vasprintf(char** restrict out_string, const char* restrict format, va_list args);
#endif

// posix
int fileno(FILE* file);
typedef int mode_t;

#endif



#endif
