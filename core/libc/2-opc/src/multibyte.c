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

/**
 * @file multibyte.c
 *
 * This implements the mbs/wcs conversion functions.
 *
 * We currently only support Unicode in UTF-8 multibyte strings and UTF-32 wide
 * character strings.
 */

#include <stdlib.h>
#include <uchar.h>

// UTF-8 to UTF-32
int mbtowc(wchar_t* /*nullable*/ restrict out_wchar, const char* /*nullable*/ restrict bytes, size_t count) {
    if (bytes == NULL)
        return 0;
    if (count == 0)
        return -1;

    const char8_t* ubytes = (const char8_t*)bytes;
    char8_t lead = ubytes[0];

    // ASCII
    if (lead <= 0x7Fu) {
        if (out_wchar) {
            *out_wchar = lead;
        }
        return 1;
    }

    // 2-byte sequence
    if ((lead & 0xE0u) == 0xC0u) {
        if (count < 2) // truncated sequence
            return -1;

        char8_t cont = ubytes[1];
        if ((cont & 0xC0u) != 0x80u) // not a continuation byte
            return -1;

        wchar_t wchar = ((wchar_t)(lead & ~0xE0u) << 6) |
                         (wchar_t)(cont & ~0xC0u);

        if (wchar < 0x80u) // overlong sequence
            return -1;

        if (out_wchar) {
            *out_wchar = wchar;
        }
        return 2;
    }

    // 3-byte sequence
    if ((lead & 0xF0u) == 0xE0u) {
        if (count < 3) // truncated sequence
            return -1;

        char8_t cont1 = ubytes[1];
        if ((cont1 & 0xC0u) != 0x80u) // not a continuation byte
            return -1;
        char8_t cont2 = ubytes[2];
        if ((cont2 & 0xC0u) != 0x80u) // not a continuation byte
            return -1;

        wchar_t wchar = ((wchar_t)(lead  & ~0xF0u) << 12) |
                        ((wchar_t)(cont1 & ~0xC0u) <<  6) |
                         (wchar_t)(cont2 & ~0xC0u);

        if (wchar < 0x800u) // overlong sequence
            return -1;
        if (wchar >= 0xD800u && wchar <= 0xDFFFu) // surrogate
            return -1;

        if (out_wchar) {
            *out_wchar = wchar;
        }
        return 3;
    }

    // 4-byte sequence
    if ((lead & 0xF8u) == 0xF0u) {
        if (count < 4) // truncated sequence
            return -1;

        char8_t cont1 = ubytes[1];
        if ((cont1 & 0xC0u) != 0x80u) // not a continuation byte
            return -1;
        char8_t cont2 = ubytes[2];
        if ((cont2 & 0xC0u) != 0x80u) // not a continuation byte
            return -1;
        char8_t cont3 = ubytes[3];
        if ((cont3 & 0xC0u) != 0x80u) // not a continuation byte
            return -1;

        wchar_t wchar = ((wchar_t)(lead  & ~0xF8u) << 18) |
                        ((wchar_t)(cont1 & ~0xC0u) << 12) |
                        ((wchar_t)(cont2 & ~0xC0u) <<  6) |
                         (wchar_t)(cont3 & ~0xC0u);

        if (wchar < 0x10000u) // overlong sequence
            return -1;
        if (wchar > 0x10FFFFu) // codepoint limit
            return -1;

        if (out_wchar) {
            *out_wchar = wchar;
        }
        return 4;
    }

    // other errors include a continuation byte without a lead, or lead for a
    // 5-byte sequence or longer.
    return -1;
}

// UTF-32 to UTF-8
int wctomb(char* /*nullable*/ out_bytes, wchar_t wchar) {

    // ASCII
    if (wchar <= 0x7Fu) {
        if (out_bytes) {
            out_bytes[0] = (char)wchar;
        }
        return 1;
    }

    // 2-byte sequence
    if (wchar <= 0x7FFu) {
        if (out_bytes) {
            out_bytes[0] = (char)((wchar >> 6) | 0xC0u);
            out_bytes[1] = (char)((wchar & 0x3Fu) | 0x80u);
        }
        return 2;
    }

    // 3-byte sequence
    if (wchar <= 0xFFFFu) {
        if (out_bytes) {
            out_bytes[0] = (char)((wchar >> 12) | 0xE0u);
            out_bytes[1] = (char)(((wchar >> 6) & 0x3Fu) | 0x80u);
            out_bytes[2] = (char)((wchar & 0x3Fu) | 0x80u);
        }
        return 3;
    }

    // 4-byte sequence
    if (wchar <= 0x10FFFFu) {
        if (out_bytes) {
            out_bytes[0] = (char)((wchar >> 18) | 0xF0u);
            out_bytes[1] = (char)(((wchar >> 12) & 0x3Fu) | 0x80u);
            out_bytes[2] = (char)(((wchar >> 6) & 0x3Fu) | 0x80u);
            out_bytes[3] = (char)((wchar & 0x3Fu) | 0x80u);
        }
        return 4;
    }

    // Invalid wchar
    return -1;
}

int mblen(const char* /*nullable*/ bytes, size_t count) {
    return mbtowc(NULL, bytes, count);
}
