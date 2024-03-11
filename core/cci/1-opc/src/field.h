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

#ifndef FIELD_H_INCLUDED
#define FIELD_H_INCLUDED

/**
 * A field is one variable in a struct.
 *
 * Fields are stored in an intrusive singly-linked list and are owned by their
 * containing record.
 *
 * Fields are immutable.
 */
typedef void* field_t;

/**
 * Creates a new field.
 *
 * The field takes ownership of the given type.
 */
field_t* field_new(const char* name, type_t* type, size_t offset, field_t* next);

void field_delete(field_t* field);

const char* field_name(field_t* field);

type_t* field_type(field_t* field);

field_t* field_next(field_t* field);

/**
 * Returns the size (as in sizeof) of this field.
 */
size_t field_size(field_t* field);

/**
 * Returns the offset of this field from the start of its containing record.
 */
size_t field_offset(field_t* field);

/**
 * Returns the end offset of this field, i.e. its offset plus its size.
 */
size_t field_end(field_t* field);

#endif
