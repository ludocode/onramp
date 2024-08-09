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

#ifndef MEMBER_H_INCLUDED
#define MEMBER_H_INCLUDED

#include "common.h"

/**
 * A member of a record (a struct or union.)
 *
 * Members are stored in an intrusive singly-linked list and are owned by their
 * containing record.
 *
 * Members are immutable.
 */

/**
 * Creates a new member.
 *
 * The member takes ownership of the given type and name.
 */
member_t* member_new(char* name, type_t* type, size_t offset, member_t* next);

void member_delete(member_t* member);

const char* member_name(const member_t* member);

type_t* member_type(const member_t* member);

member_t* member_next(const member_t* member);

/**
 * Returns the size (as in sizeof) of this member.
 */
size_t member_size(const member_t* member);

/**
 * Returns the offset of this member from the start of its containing record.
 */
size_t member_offset(const member_t* member);

/**
 * Returns the end offset of this member, i.e. its offset plus its size.
 */
size_t member_end(const member_t* member);

#endif
