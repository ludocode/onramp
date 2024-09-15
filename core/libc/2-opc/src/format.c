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

#include <stdio.h>

#include "internal.h"

#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//#ifdef __onramp_cci_opc__
#define NO_LONG_LONG // TODO temporarily using this until cci/2 supports long long
//#endif

typedef enum length_modifier_t {
    length_modifier_none = 0,
    length_modifier_h,
    length_modifier_l,
    length_modifier_hh,
    length_modifier_ll,
    length_modifier_L,
    length_modifier_j,
    length_modifier_z,
    length_modifier_t_,
} length_modifier_t;

typedef struct directive_t {
    length_modifier_t length_modifier;
    uint8_t conversion;

    // If argument_positions is true, these indicate the argument position.
    // Otherwise they indicate the literal value.
    int field_width;
    int precision;

    // The argument position of the conversion specifier if argument_positions
    // is true.
    int conversion_argument_position;

    // True if we have argument positions ('$')
    bool argument_positions : 1;

    // These flags are true if '*' was given, in which case the value should be
    // taken from an argument (either the next one, or a specific argument if
    // argument_positions is true.)
    bool field_width_as_argument : 1;
    bool precision_as_argument : 1;

    // Other flags
    bool alternate          : 1; // #
    bool zero_padded        : 1; // 0
    bool left_adjusted      : 1; // -
    bool plus_sign          : 1; // +
    bool blank_plus_sign    : 1; // space
    bool thousands_grouping : 1; // '  (Single UNIX)
    bool locale_digits      : 1; // I  (glibc)
} directive_t;

/**
 * Gets the char pointed to by pps, or a null byte if the end is reached.
 */
#ifndef __onramp_cpp_omc__
    #define DIRECTIVE_CHAR(s, end) (((s) == (end)) ? 0 : *(s))
#endif
#ifndef DIRECTIVE_CHAR
    static inline char DIRECTIVE_CHAR(const char* s, const char* end) {
        return (s == end) ? 0 : *s;
    }
#endif

// TODO can probably sweep out the "end" parameter in all this and use
// null-terminated format strings everywhere. Not really any point in keeping
// the non-null termination behaviour

static bool directive_try_parse_number(const char** s, const char* end, int* out_number) {
    if (*s == end)
        return false;
    if (**s < '1' || **s > '9')
        return false;

    int number = 0;
    while (*s != end && **s >= '0' && **s <= '9') {
        number *= 10; // TODO check for overflow
        number += (int)**s - '0'; // TODO check for overflow
        ++*s;
    }
    *out_number = number;
    return true;
}

static bool directive_parse_flags(directive_t* directive, const char** s, const char* end) {
    while (*s != end) {
        switch (**s) {
            case '#':
                if (directive->alternate) {
                    //__libc_error("Duplicate format flag: #");
                    return false;
                }
                directive->alternate = true;
                break;

            case '0':
                if (directive->zero_padded) {
                    //__libc_error("Duplicate format flag: 0");
                    return false;
                }
                directive->zero_padded = true;
                break;

            case '-':
                if (directive->left_adjusted) {
                    //__libc_error("Duplicate format flag: -");
                    return false;
                }
                directive->left_adjusted = true;
                break;

            case '+':
                if (directive->plus_sign) {
                    //__libc_error("Duplicate format flag: +");
                    return false;
                }
                directive->plus_sign = true;
                break;

            case ' ':
                if (directive->blank_plus_sign) {
                    //__libc_error("Duplicate format flag: (space)");
                    return false;
                }
                directive->blank_plus_sign = true;
                break;

            // Single UNIX extension
            case '\'':
                if (directive->thousands_grouping) {
                    //__libc_error("Duplicate format flag: \'");
                    return false;
                }
                directive->thousands_grouping = true;
                break;

            // GNU extension
            // (note: it's probably not safe to handle this because it might be
            // an old Windows length modifier for size_t.)
            case 'I':
                if (directive->locale_digits) {
                    //__libc_error("Duplicate format flag: I");
                    return false;
                }
                directive->locale_digits = true;
                break;

            default:
                // Even if no flags were found, it's not an error. Only duplicate flags are
                // errors.
                return true;
        }
        ++*s;
    }

    return true;
}

static void directive_parse_length_modifier(directive_t* directive, const char** s, const char* end) {
    switch (DIRECTIVE_CHAR(*s, end)) {

        // We handle 'h' and 'l' specially since they may be provided twice

        case 'h':
            ++*s;
            if (DIRECTIVE_CHAR(*s, end) == 'h') {
                ++*s;
                directive->length_modifier = length_modifier_hh;
            } else {
                directive->length_modifier = length_modifier_h;
            }
            break;

        case 'l':
            ++*s;
            if (DIRECTIVE_CHAR(*s, end) == 'l') {
                ++*s;
                directive->length_modifier = length_modifier_ll;
            } else {
                directive->length_modifier = length_modifier_l;
            }
            break;

        // BSD extension, deprecated
        case 'q':
            ++*s;
            directive->length_modifier = length_modifier_ll;
            break;

        case 'L':
            ++*s;
            directive->length_modifier = length_modifier_L;
            break;

        case 'j':
            ++*s;
            directive->length_modifier = length_modifier_j;
            break;

        // non-standard, deprecated
        case 'Z': // fallthrough
        // standard
        case 'z':
            ++*s;
            directive->length_modifier = length_modifier_z;
            break;

        case 't':
            ++*s;
            directive->length_modifier = length_modifier_t_;
            break;

        // Any other character is not length modifier.
        default:
            // Return to avoid consuming the character
            return;
    }
}

static bool directive_parse_conversion_specifier(directive_t* directive, const char** s, const char* end) {
    if (*s == end) {
        //__libc_error("Format directive truncated");
        return false;
    }

    switch (**s) {
        // integer in decimal, octal, or hex notations
        case 'i': // fallthrough
        case 'd': // fallthrough
        case 'o': // fallthrough
        case 'u': // fallthrough
        case 'x': // fallthrough
        case 'X': // fallthrough

        // float in simple, exponent, mixed, or hex notations
        case 'e': // fallthrough
        case 'E': // fallthrough
        case 'f': // fallthrough
        case 'F': // fallthrough
        case 'g': // fallthrough
        case 'G': // fallthrough
        case 'a': // fallthrough
        case 'A': // fallthrough

        // char and string
        case 'c': // fallthrough
        case 's': // fallthrough

        // SUS only, "lc", TODO
        case 'C': // fallthrough
        case 'S': // fallthrough

        // pointer
        case 'p': // fallthrough

        // number of characters written so far
        case 'n': // fallthrough

        // output of strerror(errno)
        case 'm':
            break;

        case 0:
            //__libc_error("Format directive truncated");
            return false;

        default:
            //__libc_error("Format directive invalid");
            return false;
    }

    directive->conversion = (uint8_t)**s;
    ++*s;
    return true;
}

/**
 * Directive parser.
 */
static bool directive_parse(directive_t* directive, const char** s, const char* end) {
    memset(directive, 0, sizeof(*directive));

    // The directive must start with '%'
    if (DIRECTIVE_CHAR(*s, end) != '%') {
        //__libc_error("Format directive must start with '%'");
        return false;
    }
    ++*s;

    // %% is a literal %; we call it a % directive
    if (DIRECTIVE_CHAR(*s, end) == '%') {
        directive->conversion = '%';
        ++*s;
        return true;
    }

    // Argument position ('$') comes before flags so we have to check for that
    // first.
    int number;
    bool have_number = directive_try_parse_number(s, end, &number);
    if (have_number) {
        if (DIRECTIVE_CHAR(*s, end) == '$') {
            // A number followed by '$' is an argument position
            ++*s;
            directive->conversion_argument_position = number;
            directive->argument_positions = true;
        } else {
            // Otherwise the number is a field width.
            directive->field_width = number;
        }
    }

    // If we don't have a field width yet, we can parse flags and field width.
    if (directive->field_width == 0) {

        // Flags
        if (!directive_parse_flags(directive, s, end))
            return false;

        // Field width by argument
        if (DIRECTIVE_CHAR(*s, end) == '*') {
            ++*s;
            directive->field_width_as_argument = true;

            if (directive->argument_positions) {
                have_number = directive_try_parse_number(s, end, &number);
                if (!have_number || DIRECTIVE_CHAR(*s, end) != '$') {
                    //__libc_error("Format directive field width '*' requires argument position");
                    return false;
                }
                ++*s;
                directive->field_width = number;
            }

        // Literal Field Width
        } else {
            have_number = directive_try_parse_number(s, end, &number);
            if (have_number) {
                directive->field_width = number;
            }
        }
    }

    // After field width we have optional '.' and precision
    if (DIRECTIVE_CHAR(*s, end) == '.') {
        ++*s;

        // Precision by argument
        if (DIRECTIVE_CHAR(*s, end) == '*') {
            ++*s;
            directive->precision_as_argument = true;

            if (directive->argument_positions) {
                have_number = directive_try_parse_number(s, end, &number);
                if (!have_number || DIRECTIVE_CHAR(*s, end) != '$') {
                    //__libc_error("Format directive field width '*' requires argument position");
                    return true;
                }
                ++*s;
                directive->precision = number;
            }

        // Literal Precision
        } else {
            have_number = directive_try_parse_number(s, end, &number);
            if (have_number) {
                directive->precision = number;
            }
        }
    }

    // Length modifier
    directive_parse_length_modifier(directive, s, end);

    // Conversion specifier
    return directive_parse_conversion_specifier(directive, s, end);
}

/**
 * Converts an unsigned integer to a decimal string.
 */
static size_t utod(uintmax_t value, char* output) {
    if (value == 0) {
        *output = '0';
        return 1;
    }

    char reverse[64];
    char* p = reverse;
    while (value > 0) {
        *p++ = '0' + (value % 10);
        value /= 10;
    }

    size_t length = p - reverse;
    for (size_t i = 0; i < length; ++i) {
        output[i] = reverse[length - i - 1];
    }
    return length;
}


/**
 * Converts an unsigned integer to a hexdecimal string.
 */
static size_t utoh(uintmax_t value, char* output, bool uppercase) {
    if (value == 0) {
        *output = '0';
        return 1;
    }

    char reverse[64];
    char* p = reverse;
    while (value > 0) {
        int digit = value & 0xF;
        if (digit >= 10) {
            *p = (uppercase ? 'A' : 'a') + digit - 10;
        } else {
            *p = '0' + digit;
        }
        ++p;
        value >>= 4;
    }

    size_t length = p - reverse;
    for (size_t i = 0; i < length; ++i) {
        output[i] = reverse[length - i - 1];
    }
    return length;
}

/**
 * Output state for the print function.
 *
 * Output can be directed to a FILE or to a buffer. This also tracks the total
 * number of bytes output and whether any errors occurred.
 */
typedef struct {
    char* buffer;
    char* buffer_end;
    FILE* file;
    size_t total;
    bool error;
} output_t;

static void output_init_file(output_t* output, FILE* file) {
    memset(output, 0, sizeof(*output));
    output->file = file;
}

static void output_init_buffer(output_t* output, char* buffer, size_t buffer_size) {
    memset(output, 0, sizeof(*output));
    output->buffer = buffer;
    output->buffer_end = buffer + buffer_size;
}

static void print_output(output_t* output, const char* bytes, size_t count) {
    output->total += count;

    if (output->file) {
        if (count != fwrite(bytes, 1, count, output->file)) {
            output->error = true;
        }
    }

    if (output->buffer) {
        char* buffer = output->buffer;
        size_t free_space = output->buffer_end - output->buffer;
        if (free_space < count) {
            count = free_space;
            output->buffer = NULL;
        } else {
            output->buffer += count;
        }
        memcpy(buffer, bytes, count);
    }
}

static void print_d(output_t* output, directive_t* directive, va_list* args, char* number_buffer) {

    // get argument
    intmax_t value;
    if (directive->length_modifier == length_modifier_ll) {
        #ifndef NO_LONG_LONG
            value = va_arg(*args, long long);
        #endif
        #ifdef NO_LONG_LONG
            output->error = true;
            return;
        #endif
    } else {
        value = va_arg(*args, int);
    }

    // convert negative to positive
    uintmax_t uvalue;
    if (value >= 0)
        uvalue = (uintmax_t)value;
    else if (value == INTMAX_MIN)
        uvalue = (uintmax_t)INTMAX_MAX + 1u;
    else
        uvalue = -(uintmax_t)value;

    // format it
    size_t length = utod(uvalue, number_buffer);
    if (value < 0)
        print_output(output, "-", 1);

    // TODO the rest of the modifiers. For now we only handle precision.
    assert(!directive->argument_positions); // TODO not yet implemented
    if (length < (size_t)directive->precision) {
        for (size_t i = (size_t)directive->precision - length; i-- > 0;) {
            print_output(output, "0", 1);
        }
    }

    print_output(output, number_buffer, length);
}

/**
 * Main print function.
 */
static void print(const char* format, output_t* output, va_list args) {
    const char* p = format;

    // buffer large enough to format any number, floating point or otherwise
    char number_buffer[128];

    while (*p != 0) {

        // Scan forward until we find a format directive and output the content.
        if (*p != '%') {
            const char* start = p;
            while (*p != '%' && *p != 0)
                ++p;
            print_output(output, start, p - start);
            continue;
        }

        // We've found a directive. Parse it.
        directive_t directive;
        if (!directive_parse(&directive, &p, NULL)) {
            libc_assert(false);
            output->error = true;
            return;
        }

        // Although we parse argument positions, we don't actually support them
        // (yet).
        if (directive.argument_positions) {
            libc_assert(false);
            output->error = true;
            return;
        }

        switch (directive.conversion) {
            case '%':
                print_output(output, "%", 1);
                break;

            case 'd':
            case 'i':
                print_d(output, &directive, (va_list*)&args, number_buffer);
                break;

            case 's': {
                // TODO length modifier
                if (directive.length_modifier != length_modifier_none) {
                    //libc_assert(false);
                    output->error = true;
                    return;
                }
                const char* str = va_arg(args, const char*);
                size_t length = strlen(str);
                print_output(output, str, length);
                break;
            }

            case 'c': {
                // TODO length modifier
                if (directive.length_modifier != length_modifier_none) {
                    //libc_assert(false);
                    output->error = true;
                    return;
                }
                char c = (char)va_arg(args, int);
                print_output(output, &c, 1);
                break;
            }

            case 'u': {
                uintmax_t value;
                if (directive.length_modifier == length_modifier_ll) {
                    #ifndef NO_LONG_LONG
                        value = va_arg(args, unsigned long long);
                    #endif
                    #ifdef NO_LONG_LONG
                        output->error = true;
                        return;
                    #endif
                } else {
                    value = va_arg(args, unsigned);
                }
                size_t length = utod(value, number_buffer);
                print_output(output, number_buffer, length);
                break;
            }

            case 'p': // fallthrough
            case 'x': // fallthrough
            case 'X': {
                uintmax_t value;
                if (directive.length_modifier == length_modifier_ll) {
                    #ifndef NO_LONG_LONG
                        value = va_arg(args, unsigned long long);
                    #endif
                    #ifdef NO_LONG_LONG
                        output->error = true;
                        return;
                    #endif
                } else {
                    value = va_arg(args, unsigned);
                }
                size_t length = utoh(value, number_buffer, directive.conversion == 'X');
                print_output(output, number_buffer, length);
                break;
            }

            default:
                libc_assert(false);
                break;
        }
    }
}

int vsnprintf(char* restrict buffer, size_t buffer_size, const char* restrict format, va_list args) {
    output_t output;
    output_init_buffer(&output, buffer, buffer_size);
    print(format, &output, args);
    if (output.total < buffer_size) {
        buffer[output.total] = 0; // null-terminate
    }
    if (output.error) {
        return -1;
    }
    return output.total;
}

int vfprintf(FILE* restrict file, const char* restrict format, va_list args) {
    output_t output;
    output_init_file(&output, file);
    print(format, &output, args);
    if (output.error) {
        return -1;
    }
    return output.total;
}

int vasprintf(char** restrict out_string, const char* restrict format, va_list args) {

    /* Try to format to a stack-allocated buffer */
    char buffer[128];
    va_list temp_args;
    int ret;
    va_copy(temp_args, args);
    ret = vsnprintf(buffer, sizeof(buffer), format, temp_args);
    va_end(temp_args);
    if (ret < 0) {
        *out_string = NULL;
        return ret;
    }

    /* If it fit, allocate a copy. We use memdup() rather than strdup() because
     * we already know the size. */
    size_t size = 1 + (size_t)ret;
    if (size <= sizeof(buffer)) {
        libc_assert(buffer[ret] == 0);
        *out_string = (char*)__memdup(buffer, size);
        if (*out_string == NULL)
            return -1;
        return ret;
    }

    /* Otherwise allocate the correct size and re-format. */
    *out_string = (char*)malloc(size);
    if (*out_string == NULL)
        return -1;
    ret = vsnprintf(*out_string, size, format, args);
    if (ret == -1) {
        /* This shouldn't be possible; it worked the first time. We
         * check anyway to be sure. */
        __fatal("Internal error: second format of vasprintf() failed.");
    }
    libc_assert((*out_string)[size - 1] == 0);
    return ret;
}

#ifdef DISABLED
/**
 * Main scan function.
 *
 * TODO. Not clear we can implement this shared unless we also have a way to
 * peek data from a FILE or otherwise put it back in the buffer when we read
 * too much.
 */
static int scan(
        const char* format,
        char* buffer,
        size_t buffer_size,
        int (*fill)(char** buffer, size_t* buffer_size), // TODO should probably be consume()
        va_list args)
{
    // TODO
    return -1;
}

int vfscanf(FILE* restrict stream, const char* restrict format, va_list args) {
    // TODO
    (void)scan;
    return -1;
}

int vsscanf(const char* restrict s, const char* restrict format, va_list args) {
    // TODO
    return -1;
}
#endif

/*
 * The rest of these functions just wrap the implementations above.
 */

int fprintf(FILE* restrict stream, const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(stream, format, args);
    va_end(args);
    return ret;
}

#ifdef DISABLED
int fscanf(FILE* restrict stream, const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfscanf(stream, format, args);
    va_end(args);
    return ret;
}
#endif

int printf(const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(stdout, format, args);
    va_end(args);
    return ret;
}

#ifdef DISABLED
int scanf(const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfscanf(stdin, format, args);
    va_end(args);
    return ret;
}
#endif

int snprintf(char* restrict buffer, size_t buffer_size, const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsnprintf(buffer, buffer_size, format, args);
    va_end(args);
    return ret;
}

int sprintf(char* restrict buffer, const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsnprintf(buffer, INT_MAX, format, args);
    va_end(args);
    return ret;
}

#ifdef DISABLED
int sscanf(const char* restrict s, const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsscanf(s, format, args);
    va_end(args);
    return ret;
}
#endif

int vprintf(const char* restrict format, va_list args) {
    return vfprintf(stdout, format, args);
}

#ifdef DISABLED
int vscanf(const char* restrict format, va_list args) {
    return vfscanf(stdin, format, args);
}
#endif

int vsprintf(char* restrict s, const char* restrict format, va_list args) {
    return vsnprintf(s, INT_MAX, format, args);
}

int asprintf(char** restrict out_string, const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vasprintf(out_string, format, args);
    va_end(args);
    return ret;
}
