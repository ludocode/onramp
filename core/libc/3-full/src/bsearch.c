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

#include <stdlib.h>

void* bsearch(const void* key, const void* vbase,
                size_t element_count, size_t element_size,
                int (*compare)(const void* key, const void* element))
{
    const char* base = (const char*)vbase;
    size_t start = 0;
    size_t end = element_count;

    while (start != end) {
        size_t i = (start + end) >> 1;
        const void* p = base + i * element_size;
        int c = compare(key, p); // key must be the first argument, see C17 7.22.5$2
//printf("-- %i,%i i %i v %i c %i\n", start, end, i, *(int*)p, c);
        if (c == 0)
            return (void*)p;
        if (c < 0)
            end = i;
        else
            start = i + 1;
    }

    return 0;
}
