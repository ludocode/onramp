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

        // TODO currently using this workaround since we don't have #undef in cpp/1
        // TODO add #undef in cpp/1 to simplify this
        #define __ONRAMP_STRING_IMPL
#include <string.h>
        int bcmp(const void* a, const void* b, size_t count);
        void* memcpy(void* dest, const void* src, size_t count);
        char* rindex(const char* s, int c);
        char* index(const char* s, int c);

#include "internal.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef __onramp_cpp_omc__
    #define MIN(a, b) (((a) < (b)) ? (a) : (b))
    #define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifdef __onramp_cpp_omc__
    static inline size_t MIN(size_t a, size_t b) {return (a < b) ? a : b;}
    static inline size_t MAX(size_t a, size_t b) {return (a > b) ? a : b;}
#endif

void bcopy(const void* src, void* dest, size_t count) {
    memmove(dest, src, count);
}

void bzero(void* p, size_t count) {
    memset(p, 0, count);
}

void* memccpy(void* restrict vdest, const void* restrict vsrc,
        int c, size_t count)
{
    const unsigned char* restrict src = (const unsigned char*)vsrc;
    unsigned char* dest = (unsigned char*)vdest;
    unsigned char* end = dest + count;

    /* The C23 standard specifies that we're searching for `(unsigned char)c`. */
    unsigned char uc = (unsigned char)c;

    while (dest != end) {
        /* The character is supposed to be copied even if it matches. */
        *dest = *src;

        /* Return a pointer to the next character after the match. */
        ++dest;
        if (*src == uc)
            return dest;
        ++src;
    }

    /* The character was not found. */
    return NULL;
}

void* memchr(const void* vdata, int c, size_t count) {
    const char* data = (const char*)vdata;
    size_t i;
    for (i = 0; i < count; ++i)
        if (data[i] == c)
            return (char*)(data + i);
    return NULL;
}

int memcmp(const void* p1, const void* p2, size_t count) {
    const unsigned char* c1 = (const unsigned char*)p1;
    const unsigned char* c2 = (const unsigned char*)p2;
    const unsigned char* end = c1 + count;
    while (c1 != end) {
        if (*c1 != *c2) {
            return (int)(*c1) - (int)(*c2);
        }
        ++c1;
        ++c2;
    }
    return 0;
}

int memlcmp(
        const void* v_a, size_t v_a_length,
        const void* v_b, size_t v_b_length)
{
    size_t v_length = MIN(v_a_length, v_b_length);
    int v_ret = memcmp(v_a, v_b, v_length);
    if (v_ret != 0)
        return v_ret;

    return (v_a_length < v_b_length) ? -1 :
            (v_a_length > v_b_length) ? 1 :
            0;
}

void* memmem(const void* vhaystack, size_t haystack_length,
    const void *vneedle, size_t needle_length)
{
    const uint8_t* haystack = (const uint8_t*)vhaystack;
    const uint8_t* needle = (const uint8_t*)vneedle;
    size_t i;

    /* Special cases */
    if (needle_length > haystack_length)
        return NULL;
    if (needle_length == 0)
        return (void*)vhaystack;
    if (needle_length == 1)
        return (void*)memchr(haystack, needle[0], haystack_length);

    /* This is a very simple Boyer-Moore-Horspool (BMH):
     *     https://en.wikipedia.org/wiki/Boyer%E2%80%93Moore%E2%80%93Horspool_algorithm */

    /*
     * BMH may skip up to needle_length beyond the end of the haystack. We
     * make sure it won't overflow. This has a limitation that you can't
     * just blindly pass SIZE_MAX as the haystack_length if you're sure
     * the needle will be found.
     *
     * TODO is this right? doesn't look like it skips that way, and even if
     * it did, it's fine if not used in a comparison... can maybe pull out
     * this assert completely
     */
    libc_assert(haystack_length + needle_length >= haystack_length);//, "haystack overflow");

    /* Compute skip table. We use a single byte for each skip entry so the
     * max skip is limited to 255 bytes. This uses relatively little stack
     * space. */
    {
        uint8_t max_skip = (uint8_t)MIN(255, needle_length);
        uint8_t skip[256];
        memset(skip, max_skip, sizeof(skip));
        for (i = 0; i < needle_length - 1; ++i)
            skip[needle[i]] = (uint8_t)MIN(255, needle_length - i - 1);

        /* Do the search */
        for (i = 0; i < haystack_length - needle_length + 1;) {
            if (bcmp(haystack + i, needle, needle_length) == 0)
                return (uint8_t*)(haystack + i);

            /* Jump ahead based on the skip value of the last character. */
            i += skip[haystack[i + needle_length - 1]];
        }
    }

    return NULL;
}

void* memmove(void* vdest, const void* vsrc, size_t count) {
    void* start = vdest;
    const unsigned char* src = (const unsigned char*)vsrc;
    unsigned char* dest = (unsigned char*)vdest;
    if (dest < src) {
        // TODO vectorize
        unsigned char* end = dest + count;
        while (dest != end)
            *dest++ = *src++;
    } else if (dest > src) {
        // TODO vectorize
        unsigned char* end = dest;
        dest += count;
        src += count;
        while (dest != end)
            *--dest = *--src;
    }
    return start;
}

void* mempcpy(void* restrict dest, const void* restrict src, size_t n) {
    memmove(dest, src, n);
    return (char*)dest + n;
}

void* memrchr(const void* vdata, int c, size_t count) {
    const char* data = (const char*)vdata;
    size_t i;
    void* ret = NULL;
    for (i = 0; i < count; ++i)
        if (data[i] == c)
            ret = (char*)(data + i);
    return ret;
}

void* memset(void* vdest, int c, size_t count) {
    /* TODO test this */
    // TODO vectorize
    unsigned char* restrict dest = (unsigned char*)vdest;
    unsigned char* start = dest;
    unsigned char* end = dest + count;
    unsigned char uc = (unsigned char)c;
    while (dest != end)
        *dest++ = uc;
    return start;
}

char* stpcpy(char* restrict dest, const char* restrict src) {
    for (;;) {
        *dest = *src;
        if (*src == 0)
            break;
        ++dest;
        ++src;
    }
    return dest;
}

char* stpncpy(char* restrict dest, const char* restrict src, size_t n) {
    char* end = dest + n;
    while (dest != end) {
        *dest = *src;
        if (*src == 0)
            break;
        ++dest;
        ++src;
    }
    return dest;
}

char* strcat(char* restrict dest, const char* restrict src) {
    char* original_dest = dest;
    while (*dest != 0)
        ++dest;
    for (;;) {
        *dest = *src;
        if (*src == 0)
            break;
        ++src;
        ++dest;
    }
    return original_dest;
}

char* strchr(const char* s, int c) {
    for (;;) {
        if (*s == c)
            return (char*)s;
        if (*s == 0)
            break;
        ++s;
    }
    return NULL;
}

char* strchrnul(const char* s, int c) {
    while (*s != 0) {
        if (*s == c)
            break;
        ++s;
    }
    return (char*)s;
}

int strcmp(const char* a, const char* b) {
    while (*a == *b) {
        if (*a == 0)
            return 0;
        ++a;
        ++b;
    }
    return (int)(unsigned char)(*a) - (int)(unsigned char)(*b);
}

char* strcpy(char* restrict dest, const char* restrict src) {
    char* original_dest = dest;
    for (;;) {
        *dest = *src;
        if (*src == 0)
            break;
        ++dest;
        ++src;
    }
    return original_dest;
}

size_t strcspn(const char* s, const char* reject) {
    const char* start = s;
    while (*s) {
        const char* k;
        for (k = reject; *k != 0; ++k)
            if (*s == *k)
                goto done;
        ++s;
    }
done:
    return (size_t)(s - start);
}

size_t strlcat(char* restrict to,
        const char* restrict from, size_t count)
{
    const char* from_start = from;

    /* Find the end of the destination. */
    while (*to != 0 && count != 0) {
        ++to;
        --count;
    }

    /* Write bytes (if there's any space left) */
    if (count > 0) {

        /* copy up to count-1 bytes */
        char* to_end = to + count - 1;
        while (to != to_end && *from) {
            *to++ = *from++;
        }

        /* write the null-terminator */
        *to = 0;
    }

    /* return the total length of from */
    while (*from)
        ++from;
    return (size_t)(from - from_start);
}

size_t strlcpy(char* restrict to,
        const char* restrict from, size_t count)
{
    const char* from_start = from;

    /* TODO test this */

    /* If count is 0, we can't even write a null-terminator. I can't
     * imagine how calling this with a size of 0 could ever be reasonable
     * in correct code but the man page implies that it's not undefined
     * behaviour so we have to handle it "safely". */
    if (count > 0) {

        /* copy up to count-1 bytes */
        char* to_end = to + count - 1;
        while (to != to_end && *from != 0) {
            *to = *from;
            ++to;
            ++from;
        }

        /* write the null-terminator */
        *to = 0;
    }

    /* return the total length of from */
    while (*from)
        ++from;
    return (size_t)(from - from_start);
}

size_t strlen(const char* s) {
    const char* end = s;
    while (*end != 0)
        ++end;
    return (size_t)(end - s);
}

char* strncat_impl(char* restrict dest, const char* restrict src, size_t n) {
    char* original_dest = dest;
    while (*dest != 0)
        ++dest;
    const char* end = src + n;
    while (*src != 0 && src != end) {
        *dest = *src;
        ++dest;
        ++src;
    }
    *dest = 0;
    return original_dest;
}

int strncmp(const char* a, const char* b, size_t n) {
    const char* end = a + n;
    while (*a == *b && a != end) {
        if (*a == 0)
            return 0;
        ++a;
        ++b;
    }
    return (int)(unsigned char)(*a) - (int)(unsigned char)(*b);
}

char* strncpy_impl(char* restrict to, const char* restrict from, size_t count) {
    char* ret = to;
    char* end = to + count;
    while (to != end && *from != 0) {
        *to = *from;
        ++to;
        ++from;
    }
    /* strncpy() pads with zeroes. */
    while (to != end)
        *to++ = 0;
    return ret;
}

size_t strnlen(const char* s, size_t maxlen) {
    const char* maxend = s + maxlen;
    const char* end = s;
    while (end != maxend && *end != 0)
        ++end;
    return (size_t)(end - s);
}

size_t strnlen_s(const char* s, size_t maxlen) {
    if (!s)
        return 0;
    return strnlen(s, maxlen);
}

char* strnstr(const char* haystack, const char* needle, size_t n) {
    return (char*)memmem(haystack, MAX(n, strlen(haystack)), needle, strlen(needle));
}

char* strpbrk(const char* s, const char* accept) {
    for (; *s != 0; ++s) {
        const char* k;
        for (k = accept; *k != 0; ++k)
            if (*s == *k)
                return (char*)s;
    }
    return NULL;
}

char* strrchr(const char* s, int c) {
    char* ret = NULL;
    for (;;) {
        if (*s == c)
            ret = (char*)s;
        if (*s == 0)
            break;
        ++s;
    }
    return ret;
}

char* strsep(char** sp, const char* delimiters) {
    char* token;
    char* s = *sp;
    if (s == NULL) {
        /* no tokens left. */
        return NULL;
    }

    token = s;
    while (*s != 0) {
        const char* k;
        for (k = delimiters; *k != 0; ++k) {
            if (*s == *k) {
                /* found a delimiter! */
                *s = 0;
                *sp = s + 1;
                return token;
            }
        }
        ++s;
    }

    /* no delimiter, this is the last token. */
    *sp = NULL;
    return token;
}

char* strset(char* str, int ic) {
    char* original_str = str;
    char c = (char)ic;
    for (; *str != 0; ++str)
        *str = c;
    return original_str;
}

size_t strspn(const char* s, const char* accept) {
    const char* start = s;
    while (*s) {
        const char* k;
        for (k = accept; *k != 0; ++k) {
            if (*s == *k)
                goto cont;
        }
        goto done;
    cont:
        ++s;
    }
done:
    return (size_t)(s - start);
}

char* strstr(const char* haystack, const char* needle) {
    return (char*)memmem(haystack, strlen(haystack), needle, strlen(needle));
}

static char** strtok_state;

char* strtok(char* restrict string, const char* restrict delimiters) {
    return strtok_r(string, delimiters, strtok_state);
}

char* strtok_r(char* restrict v_str,
        const char* restrict v_delims,
        char** restrict v_state)
{
    char* ret;
    if (v_str != NULL)
        *v_state = v_str;
    /* strsep() supports empty tokens; strtok_r() does not. loop to skip them. */
    do {
        ret = strsep(v_state, v_delims);
    } while (ret != NULL && *ret == 0);
    return ret;
}

/* TODO currently included in libc/0 malloc_util
void* __memdup(const void* src, size_t size) {
    if (src == NULL)
        return NULL;
    void* dest = malloc(size);
    if (dest == NULL)
        return NULL;
    memmove(dest, src, size);
    return dest;
}
*/



/*
 * These functions are aliased in the headers but we still need to provide
 * definitions in case assembly code references them. They'll be
 * garbage-collected by ld/2 if unused.
 */

int bcmp(const void* a, const void* b, size_t count) {
    return memcmp(a, b, count);
}

void* memcpy(void* dest, const void* src, size_t count) {
    return memmove(dest, src, count);
}

char* rindex(const char* s, int c) {
    return strrchr(s, c);
}

char* index(const char* s, int c) {
    return strchr(s, c);
}
