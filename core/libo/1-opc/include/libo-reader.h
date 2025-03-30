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

#ifndef ONRAMP_LIBO_READER_H_INCLUDED
#define ONRAMP_LIBO_READER_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __onramp__
    #include <uchar.h>
#endif
#ifndef __onramp__
    // TODO external compilers don't properly support char8_t yet
    // (and macOS doesn't support <uchar.h> at all)
    typedef uint8_t char8_t;
    typedef uint16_t char16_t;
    typedef uint32_t char32_t;
#endif

struct string_t;

#define READER_EOF 0xFFFFFFFF
#define READER_BYTES_CAPACITY 32
#define READER_CHARS_CAPACITY 4

/**
 * A UTF-8 file reader.
 *
 * Unicode characters are decoded and returned one at a time as char32_t.
 *
 * Invalid bytes are added to 0x110000 and returned. This is outside the range
 * of Unicode so they cannot be mistaken for Unicode code points, and the
 * original bytes can be re-created on a conversion back to the source encoding.
 *
 * The reader supports unloading the file. The file's position will be saved
 * and it will be re-opened automatically when needed. This can be used to
 * reduce the total number of open files.
 */
typedef struct reader_t {

    // The current file state.
    FILE* file;
    bool end_of_file;

    // Data needed to reload the file after it is unloaded.
    struct string_t* filename;
    fpos_t file_offset;

    // The byte buffer. UTF-8 code points are decoded from this buffer into
    // chars[].
    // (When decoding an external memory buffer, bytes_start and bytes_end
    // point into the external buffer and bytes is unused.)
    char8_t bytes[READER_BYTES_CAPACITY];
    char8_t* bytes_start;
    char8_t* bytes_end;

    // The character buffer.
    char32_t chars[READER_CHARS_CAPACITY];
    char32_t* chars_start;
    char32_t* chars_end;

} reader_t;

/**
 * Initializes a reader from a file.
 *
 * If the file has already been opened, it can be passed in. Otherwise the file
 * will be opened.
 */
void reader_init_file(reader_t* reader, struct string_t* filename,
        FILE* /*nullable*/ file);

/**
 * Initializes a reader that reads a buffer in memory.
 */
void reader_init_bytes(reader_t* reader, char8_t* bytes, size_t count);

void reader_destroy(reader_t* reader);

/**
 * Returns the character at the given index in the buffer or READER_EOF on
 * end-of-file.
 */
char32_t reader_peek(reader_t* reader, size_t index);

/**
 * Consumes the given number of characters.
 *
 * The characters must exist in the buffer. If the characters are READER_EOF, a
 * fatal error is raised.
 *
 * If the current character is READER_EOF, a fatal error is raised.
 */
void reader_consume(reader_t* reader, size_t index);

/**
 * Consumes the current character and returns the next one or READER_EOF on
 * end-of-file.
 *
 * This is equivalent to calling reader_consume(1) followed by reader_peek(0).
 *
 * If the current character is READER_EOF, a fatal error is raised.
 */
char32_t reader_next(reader_t* reader);

/**
 * Unloads the file opened by this reader.
 *
 * This is used to limit the total number of open files.
 *
 * The reader will store its position in the file. It will automatically
 * re-open and seek the file to resume parsing when needed.
 */
void reader_unload(reader_t* reader);

#endif
