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

#define EOF (-1)

typedef void FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

FILE* fopen(const char* filename, const char* mode);
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* file);
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* file);
int fclose(FILE* file);
int feof(FILE* file);

long ftell(FILE* file);
int fseek(FILE* stream, long offset, int base);

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

int fputc(int c, FILE* file);
int fputs(const char* s, FILE* file);
int putchar(int c);
int puts(const char* s);

/* TODO these are probably not going to be defined by libc/0, only by libc/2 or
 * libc/3
typedef int fpos_t;
int fgetpos(FILE* file, fpos_t* pos);
int fsetpos(FILE* file, const fpos_t* pos);
*/

// TODO only defined by libc/1
int remove(const char* filename);

// posix
int fileno(FILE* file);
typedef int mode_t;
int fchmod(int fileno, int mode); // TODO this belongs in sys/stat.h

#endif
