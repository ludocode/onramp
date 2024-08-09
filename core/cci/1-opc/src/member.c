/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Fraser Heavy Software
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

#include "member.h"

#include <stdlib.h>

#include "type.h"

/*
 * member_t looks like this:
 *
 * typedef struct {
 *     member_t* next;
 *     char* name;
 *     type_t* type;
 *     size_t offset;
 * } member_t;
 *
 */

#define MEMBER_OFFSET_NEXT 0
#define MEMBER_OFFSET_NAME 1
#define MEMBER_OFFSET_TYPE 2
#define MEMBER_OFFSET_OFFSET 3
#define MEMBER_MEMBER_COUNT 4

member_t* member_new(char* name, type_t* type, size_t offset, member_t* next) {
    member_t* member = malloc(sizeof(void*) * MEMBER_MEMBER_COUNT);
    if (!member) {
        fatal("Out of memory.");
    }
    *(char**)((void**)member + MEMBER_OFFSET_NAME) = name;
    *(type_t**)((void**)member + MEMBER_OFFSET_TYPE) = type;
    *(size_t*)((void**)member + MEMBER_OFFSET_OFFSET) = offset;
    *(member_t**)((void**)member + MEMBER_OFFSET_NEXT) = next;
    return member;
}

void member_delete(member_t* member) {
    free((char*)member_name(member));
    type_delete((type_t*)member_type(member));
    free(member);
}

member_t* member_next(const member_t* member) {
    return *(member_t**)((void**)member + MEMBER_OFFSET_NEXT);
}

const char* member_name(const member_t* member) {
    return *(char**)((void**)member + MEMBER_OFFSET_NAME);
}

type_t* member_type(const member_t* member) {
    return *(type_t**)((void**)member + MEMBER_OFFSET_TYPE);
}

size_t member_offset(const member_t* member) {
    return *(size_t*)((void**)member + MEMBER_OFFSET_OFFSET);
}

size_t member_size(const member_t* member) {
    return type_size(member_type(member));
}

size_t member_end(const member_t* member) {
    return member_offset(member) + member_size(member);
}
