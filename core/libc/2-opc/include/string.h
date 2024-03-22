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

#ifndef __ONRAMP_LIBC_STRING_H_INCLUDED
#define __ONRAMP_LIBC_STRING_H_INCLUDED

#ifndef __onramp_libc__
    #error "__onramp/__predef.h must be force-included by the preprocessor before any libc headers."
#endif

#include <__onramp/__size_t.h>
#include <__onramp/__null.h>

/* TODO organize these like the standard */

#ifndef __ONRAMP_STRING_IMPL
    #ifndef __onramp_cci_full__
        #define bcmp memcmp
        #define memcpy memmove
        #define index strchr
        #define rindex strrchr
    #endif
    #ifdef __onramp_cci_full__
        int bcmp(const void* a, const void* b, size_t count) __asm__("memcmp");
        void* memcpy(void* dest, const void* src, size_t count) __asm__("memmove");
        char* rindex(const char* s, int c) __asm__("strrchr");
        char* index(const char* s, int c) __asm__("strchr");
    #endif
#endif

void bcopy(const void* src, void* dest, size_t count);
void bzero(void* p, size_t count);

void* memccpy(void* restrict dest, const void* restrict src, int c, size_t n);
void* memchr(const void* s, int c, size_t n);
int memcmp(const void* a, const void* b, size_t count);
void* memmem(const void* haystack, size_t haystack_length,
        const void *needle, size_t needle_length);
void* memmove(void* dest, const void* src, size_t count);
void* mempcpy(void* restrict dest, const void* restrict src, size_t n);
void* memrchr(const void* s, int c, size_t n);
void* memset(void* dest, int c, size_t count);

char* stpcpy(char* restrict dest, const char* restrict src);
char* stpncpy(char* restrict dest, const char* restrict src, size_t n);
char* strcat(char* restrict dest, const char* restrict src);
char* strchr(const char* s, int c);
char* strchrnul(const char* s, int c);
int strcmp(const char* left, const char* right);
char* strcpy(char* restrict to, const char* restrict from);
size_t strcspn(const char* s, const char* reject);
size_t strlcat(char* restrict to, const char* restrict from, size_t count);
size_t strlcpy(char* restrict dest, const char* restrict src, size_t n);
size_t strlen(const char* s);
char* strncat(char* restrict dest, const char* restrict src, size_t n);
int strncmp(const char* left, const char* right, size_t n);
char* strncpy(char* restrict dest, const char* restrict src, size_t n);
size_t strnlen(const char* s, size_t maxlen);
size_t strnlen_s(const char* s, size_t maxlen);
char* strnstr(const char* haystack, const char* needle, size_t n);
char* strpbrk(const char* s, const char* accept);
char* strrchr(const char* s, int c);
char* strsep(char** restrict string, const char* restrict delimiters);
char* strset(char* str, int c);
size_t strspn(const char* s, const char* accept);
char* strstr(const char* haystack, const char* needle);
char* strtok(char* restrict string, const char* restrict delimiters);
char* strtok_r(char* restrict string, const char* restrict delimiters,
        char** restrict save_state);

char* strdup(const char* str);
char* strndup(const char* str, size_t max_length);

void* __memdup(const void* src, size_t size);

#endif
