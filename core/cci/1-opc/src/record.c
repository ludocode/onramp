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

#include "record.h"

#include <stdlib.h>
#include <string.h>

#include "member.h"
#include "type.h"

/*
 * record_t looks like this:
 *
 * typedef struct {
 *     char* name;
 *     member_t* members;
 *     size_t size;
 *     bool is_struct;
 * } record_t;
 *
 * A record is created when it is first declared. Structs and unions can be
 * forward-declared; the members are added and the size is computed later when
 * it is defined.
 */

#define RECORD_OFFSET_NAME 0
#define RECORD_OFFSET_MEMBERS 1
#define RECORD_OFFSET_SIZE 2
#define RECORD_OFFSET_IS_STRUCT 3
#define RECORD_MEMBER_COUNT 4

record_t* record_new(char* name, bool is_struct) {
    record_t* record = malloc(sizeof(void*) * RECORD_MEMBER_COUNT);
    if (!record) {
        fatal("Out of memory.");
    }
    *(char**)((void**)record + RECORD_OFFSET_NAME) = name;
    *(member_t**)((void**)record + RECORD_OFFSET_MEMBERS) = NULL;
    *(size_t*)((void**)record + RECORD_OFFSET_SIZE) = 0;
    *(bool*)((void**)record + RECORD_OFFSET_IS_STRUCT) = is_struct;
    return record;
}

static void record_destroy_members(record_t* record) {
    member_t* member = record_members(record);
    while (member) {
        member_t* next = member_next(member);
        member_delete(member);
        member = next;
    }
}

void record_delete(record_t* record) {
    record_destroy_members(record);
    free((char*)record_name(record));
    free(record);
}

const char* record_name(const record_t* record) {
    return *(char**)((void**)record + RECORD_OFFSET_NAME);
}

member_t* record_members(const record_t* record) {
    return *(member_t**)((void**)record + RECORD_OFFSET_MEMBERS);
}

void record_set_members(record_t* record, member_t* members, bool is_struct) {
    if (record_members(record) != NULL) {
        fatal_2("Internal error: cannot change members of record ", record_name(record));
    }
    *(member_t**)((void**)record + RECORD_OFFSET_MEMBERS) = members;

    bool last = true;
    size_t size = 4;
    while (members) {

        // Check for arrays. Only the last member in a struct is allowed to be
        // an array of size zero/indeterminate (but note that our members are in
        // reverse order.) As an extension we allow any member of a union to be
        // flexible.
        type_t* type = member_type(members);
        if (type_is_array(type)) {
            int length = type_array_length(type);
            bool allow_zero = (last | !is_struct);
            if (!allow_zero) {
                if ((length == 0) | (length == TYPE_ARRAY_INDETERMINATE)) {
                    fatal("Only the last member in a struct is allowed to be an array of zero/indeterminate length.");
                }
            }
            if (allow_zero) {
                if (length == TYPE_ARRAY_INDETERMINATE) {
                    // Change indeterminate to zero so that sizeof() works.
                    type_set_array_length(type, 0);
                }
            }
        }

        // Find the largest end offset; that's the (unpadded) size of our
        // struct. (It's not necessarily the last member because this could be a
        // union.)
        size_t end = member_end(members);
        if (end > size) {
            size = end;
        }

        last = false;
        members = member_next(members);
    }

    // round up to multiple of word size (4)
    // Note: We should actually round to a multiple of the largest required
    // alignment of our members. For example a struct with three shorts could be
    // 6 bytes, not 8. We are allowed to have extra padding though so this
    // isn't a violation of the spec, it's just a waste of space. This won't
    // matter for bootstrapping.
    // TODO would be easy to fix though, just need to track largest element
    // size in the above loop, just don't want to risk breaking anything right
    // now. do it later. need to fix type_alignment() at the same time.
    size = ((size + 3) & (~3));

    // cache it
    *(size_t*)((void**)record + RECORD_OFFSET_SIZE) = size;
}

size_t record_size(const record_t* record) {
    size_t size = *(size_t*)((void**)record + RECORD_OFFSET_SIZE);
    if (size == 0) {
        fatal("Cannot `sizeof` an incomplete struct or union.");
    }
    return size;
}

bool record_is_struct(const record_t* record) {
    return *(bool*)((void**)record + RECORD_OFFSET_IS_STRUCT);
}

const member_t* record_find_member(const record_t* record, const char* name,
        size_t* out_offset)
{
    if (record_members(record) == NULL) {
        fatal("Cannot access members of an incomplete struct or union.");
    }
    member_t* member = record_members(record);
    while (member) {
        const char* other = member_name(member);
        if (0 == *other) {
            // Anonymous members must be structs or unions.
            size_t offset;
            const member_t* nested = record_find_member(type_record(member_type(member)), name, &offset);
            if (nested) {
                *out_offset = (offset + member_offset(member));
                return nested;
            }
        }
        if (0 == strcmp(name, other)) {
            *out_offset = member_offset(member);
            return member;
        }
        member = member_next(member);
    }
    return NULL;
}
