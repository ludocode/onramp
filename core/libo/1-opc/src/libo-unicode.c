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

#include "libo-unicode.h"

#include "libo-error.h"

// TODO these two encode/decode functions will move to the libc at some point
// to become the mbs/wcs conversion functions. For now we keep them in libo.

char32_t utf8_decode(char8_t** position, char8_t* end) {
    char8_t* start = *position;
    size_t count = end - start;
    if (count == 0) {
        fatal("Internal error: attempting to decode UTF-8 from an empty buffer.");
    }

    char8_t lead = start[0];

    // ASCII
    if (lead <= 0x7Fu) {
        ++*position;
        return lead;
    }

    // 2-byte sequence
    if ((lead & 0xE0u) == 0xC0u) {
        if (count < 2) // truncated sequence
            goto encoding_error;

        char8_t cont = start[1];
        if ((cont & 0xC0u) != 0x80u) // not a continuation byte
            goto encoding_error;

        *position += 2;
        return ((char32_t)(lead & ~0xE0u) << 6) |
                (char32_t)(cont & ~0xC0u);
    }

    // 3-byte sequence
    if ((lead & 0xF0u) == 0xE0u) {
        if (count < 3) // truncated sequence
            goto encoding_error;

        char8_t cont1 = start[1];
        if ((cont1 & 0xC0u) != 0x80u) // not a continuation byte
            goto encoding_error;
        char8_t cont2 = start[2];
        if ((cont2 & 0xC0u) != 0x80u) // not a continuation byte
            goto encoding_error;

        char32_t value = ((char32_t)(lead  & ~0xF0u) << 12) |
                         ((char32_t)(cont1 & ~0xC0u) <<  6) |
                          (char32_t)(cont2 & ~0xC0u);

        if (value < 0x800u) // overlong sequence
            goto encoding_error;
        if (value >= 0xD800u && value <= 0xDFFFu) // surrogate
            goto encoding_error;

        *position += 3;
        return value;
    }

    // 4-byte sequence
    if ((lead & 0xF8u) == 0xF0u) {
        if (count < 4) // truncated sequence
            goto encoding_error;

        char8_t cont1 = start[1];
        if ((cont1 & 0xC0u) != 0x80u) // not a continuation byte
            goto encoding_error;
        char8_t cont2 = start[2];
        if ((cont2 & 0xC0u) != 0x80u) // not a continuation byte
            goto encoding_error;
        char8_t cont3 = start[3];
        if ((cont3 & 0xC0u) != 0x80u) // not a continuation byte
            goto encoding_error;

        char32_t value = ((char32_t)(lead  & ~0xF8u) << 18) |
                         ((char32_t)(cont1 & ~0xC0u) << 12) |
                         ((char32_t)(cont2 & ~0xC0u) <<  6) |
                          (char32_t)(cont3 & ~0xC0u);

        if (value < 0x10000u) // overlong sequence
            goto encoding_error;
        if (value > 0x10FFFFu) // codepoint limit
            goto encoding_error;

        *position += 4;
        return value;
    }

    // other errors include a continuation byte without a lead, or lead for a
    // 5-byte sequence or longer.

encoding_error:
    ++*position;
    return lead + 0x110000u;
}

size_t utf8_encode(char8_t* out, char32_t codepoint) {

    // ASCII
    if (codepoint <= 0x7Fu) {
        out[0] = (char8_t)codepoint;
        return 1;
    }

    // 2-byte sequence
    if (codepoint <= 0x7FFu) {
        out[0] = (char8_t)((codepoint >> 6) | 0xC0u);
        out[1] = (char8_t)((codepoint & 0x3Fu) | 0x80u);
        return 2;
    }

    // 3-byte sequence
    if (codepoint <= 0xFFFFu) {
        out[0] = (char8_t)((codepoint >> 12) | 0xE0u);
        out[1] = (char8_t)(((codepoint >> 6) & 0x3Fu) | 0x80u);
        out[2] = (char8_t)((codepoint & 0x3Fu) | 0x80u);
        return 3;
    }

    // 4-byte sequence
    if (codepoint <= 0x10FFFFu) {
        out[0] = (char8_t)((codepoint >> 18) | 0xF0u);
        out[1] = (char8_t)(((codepoint >> 12) & 0x3Fu) | 0x80u);
        out[2] = (char8_t)(((codepoint >> 6) & 0x3Fu) | 0x80u);
        out[3] = (char8_t)((codepoint & 0x3Fu) | 0x80u);
        return 4;
    }

    // Raw byte due to decoding error
    if (codepoint <= 0x1100FFu) {
        out[0] = (char8_t)(codepoint - 0x110000u);
        return 1;
    }

    // Invalid codepoint
    fatal("Internal error: attempting to encode invalid codepoint to UTF-8.");

}

bool uchar_is_end_of_line(char32_t c) {
    // These are the recommendations in UAX-31.
    switch (c) {
        case '\n': // line feed
        case '\v': // vertical tab
        case '\f': // form feed
        case '\r': // carriage return
        case 0x0085u: // next line
        case 0x2028u: // line separator
        case 0x2029u: // paragraph separator
            return true;
        default:
            break;
    }
    return false;
}

bool uchar_is_ignorable(char32_t c) {

    // Most characters will be ASCII so we check this first.
    if (c <= 0x7Fu)
        return false;

    switch (c) {

        // Directional markers are considered ignorable format controls. See
        // UAX-31 for an example of an expression where this matters.
        //     https://www.unicode.org/reports/tr31/
        case 0x200Eu: // left-to-right mark
        case 0x200Fu: // right-to-left mark

        // Other default-ignorable characters
        case 0x200Du: // zero-width joiner
        case 0xFE0Fu: // variation selector-16 (a.k.a. emoji presentation selector)

            return true;
    }

    // Emoji TAG characters are default-ignorable
    if (c >= 0xE0020u && c <= 0xE007Fu)
        return true;

    return false;
}

bool uchar_is_whitespace(char32_t c) {

    // End-of-line characters are whitespace
    if (uchar_is_end_of_line(c))
        return true;

    switch (c) {
        case ' ':  // space
        case '\t': // horizontal tab
            return true;
        default:
            break;
    }

    return false;

}

bool uchar_is_identifier(char32_t c, bool first_char) {

    // ASCII
    if (c <= 0x7Fu) {
        if (!first_char && c >= '0' && c <= '9') return true;
        if (c >= 'A' && c <= 'Z') return true;
        if (c >= 'a' && c <= 'z') return true;
        if (c == '_') return true;
        return false;
    }

    // Unicode or raw bytes. We can't realistically support UAX-31 so we just
    // return true if this isn't whitespace. Note that this also includes
    // invalid characters so if the source is e.g. Windows-1252 it should still
    // mostly parse correctly.
    if (c <= 0x1100FFu) {
        return !uchar_is_whitespace(c);
    }

    fatal("Internal error: Invalid code point passed to uchar_is_identifier().");
}

