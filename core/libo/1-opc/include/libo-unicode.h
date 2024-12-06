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

#ifndef ONRAMP_LIBO_UNICODE_H_INCLUDED
#define ONRAMP_LIBO_UNICODE_H_INCLUDED

/**
 * @file libo-unicode.h
 *
 * This is Onramp's internal implementation of Unicode.
 *
 * We only implement a very small subset of Unicode here. It is enough to parse
 * most UTF-8 source files with Unicode identifiers.
 *
 * This implementation is self-contained and entirely hand-written. We don't
 * use anything machine-generated and we don't implement anything that requires
 * copyrighted Unicode data tables.
 */

#include <uchar.h>
#include <stdbool.h>

// TODO external compilers don't properly support char8_t yet
typedef unsigned char char8_t;

/**
 * The offset added to invalid bytes in a Unicode stream.
 */
#define UTF8_ERROR_OFFSET 0x110000u

/**
 * Decodes one Unicode code point from the given UTF-8 buffer.
 *
 * The code point is returned and `position` is incremented past it.
 *
 * If the bytes do not form a valid UTF-8 character, one byte is consumed,
 * added to UTF8_ERROR_OFFSET and returned. This is outside the range of valid
 * Unicode characters so it can be used to re-create the original byte stream.
 */
char32_t utf8_decode(char8_t** position, char8_t* end);

/**
 * Encodes one Unicode code point into the given UTF-8 buffer.
 *
 * The buffer must have sufficient space for four bytes.
 *
 * If the code point is UTF8_ERROR_OFFSET or higher, the offset is subtracted
 * and the byte is output as-is.
 */
size_t utf8_encode(char8_t* out, char32_t codepoint);

/**
 * Returns true if this an end-of-line character according to UAX-31.
 *
 * See: https://www.unicode.org/reports/tr31/
 *
 * Note that this does not collapse a carriage return followed by a line feed
 * into a single newline. This must be done separately.
 */
bool uchar_is_end_of_line(char32_t c);

/**
 * Returns true if this is an ignorable character according to UAX-31.
 */
bool uchar_is_ignorable(char32_t c);

/**
 * Returns true if this is a pattern white space character according to UAX-31.
 */
bool uchar_is_whitespace(char32_t c);

/**
 * Returns true if this character is valid in an identifier.
 *
 * If `first_char` is true, this returns true if the character is valid as the
 * first character of an identifier (i.e. numbers are excluded.) Otherwise it
 * retunrns true if the character is valid as the continuation of an
 * identifier.
 *
 * This does not follow the recommendations of UAX-31 as it requires large,
 * copyrighted, machine-generated data tables. Instead we allow nearly all
 * non-ascii non-whitespace unicode characters in identifiers.
 *
 * Note that this does not return true for `$`. This can be checked separately
 * if GNU extensions are enabled.
 */
bool uchar_is_identifier(char32_t c, bool first_char);

#endif
