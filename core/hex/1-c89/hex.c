/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2025 Fraser Heavy Software
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

/*
 * This is the reference implementation of the Onramp hex tool. It is written
 * in ANSI C (i.e. C89) for maximum portability.
 *
 * It implements address assertions, checks for trailing backslashes, and
 * provides rich error messages with line numbers and expected addresses. It
 * passes all tests.
 *
 * This tool can also output debug info for hex files. The first word after an
 * address assertion is used as a symbol name in debug info.
 *
 * You should use this tool to develop and verify your hex files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

typedef int BOOL;
#define TRUE 1
#define FALSE 0

static void usage(const char* name) {
    printf("\nUsage: %s [-g] <input_file> -o <output_file>\n", name);
    exit(EXIT_FAILURE);
}

static uint8_t hex_char_to_value(char hex_char) {
    if (hex_char >= '0' && hex_char <= '9')
        return hex_char - '0';
    if (hex_char >= 'A' && hex_char <= 'F')
        return hex_char - 'A' + 10;
    if (hex_char >= 'a' && hex_char <= 'f')
        return hex_char - 'a' + 10;
    puts("ERROR: Invalid hex char.");
    exit(EXIT_FAILURE);
}

static void flush_output(FILE* file, const char* buffer, size_t count) {
    while (count > 0) {
        size_t step = fwrite(buffer, 1, count, file);
        if (step == 0) {
            puts("ERROR: Failed to write output file.");
            exit(EXIT_FAILURE);
        }
        buffer += step;
        count -= step;
    }
}

static BOOL debug_info = FALSE;
static FILE* debug_file = NULL;
static BOOL in_symbol = FALSE;
static char symbol_buffer[128];
static char* symbol_pos;
static long address = 0;
static long last_symbol_address = -1;
static long last_debug_line_address = 0;
static int line = 1;
static int last_debug_line = 1;

static void emit_debug_line(void) {
    if (!debug_info || last_debug_line == line) {
        return;
    }

    if (address != last_debug_line_address) {
        fprintf(debug_file, "%li\n", address - last_debug_line_address);
        last_debug_line_address = address;
    }

    if (line == last_debug_line + 1) {
        fprintf(debug_file, "#\n");
    } else {
        fprintf(debug_file, "#line %i\n", line);
    }
    last_debug_line = line;
}

static void end_symbol(void) {
    if (!in_symbol) {
        return;
    }
    in_symbol = FALSE;

    /* if symbol buffer is empty, or we're not debugging, nothing to do */
    if (symbol_pos == symbol_buffer || !debug_info) {
        return;
    }

    /* if the address hasn't changed since the last symbol, we issue a warning */
    if (address == last_symbol_address) {
        return;
    }

    /* output debug info */
    emit_debug_line();
    *symbol_pos = 0;
    fprintf(debug_file, "#symbol %s\n", symbol_buffer);
    last_symbol_address = address;
}

int main(int argc, const char** argv) {
    int i;
    const char* input_filename = NULL;
    const char* output_filename = NULL;
    FILE* input_file;
    FILE* output_file;

    /* parse command-line options */
    for (i = 1; i < argc; ++i) {
        /* output */
        if (0 == strcmp("-o", argv[i])) {
            if (output_filename != NULL) {
                puts("ERROR: -o cannot be specified twice.");
                usage(argv[0]);
            }
            if (++i == argc) {
                puts("ERROR: -o must be followed by a filename.");
                usage(argv[0]);
            }
            output_filename = argv[i];

        /* debug info */
        } else if (0 == strcmp("-g", argv[i])) {
            debug_info = TRUE;

        /* unrecognized argument */
        } else if (argv[i][0] == '-') {
            printf("ERROR: Unsupported option: %s\n", argv[i]);
            usage(argv[0]);

        /* input */
        } else {
            if (input_filename != NULL) {
                puts("ERROR: only one input filename can be provided.");
                usage(argv[0]);
            }
            input_filename = argv[i];
        }
    }

    if (input_filename == NULL) {
        fputs("ERROR: Input filename not specified.\n", stderr);
        usage(argv[0]);
    }
    if (output_filename == NULL) {
        fputs("ERROR: Output filename not specified.\n", stderr);
        usage(argv[0]);
    }

    input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        fputs("ERROR: Failed to open input file.\n", stderr);
        exit(EXIT_FAILURE);
    }
    output_file = fopen(output_filename, "wb");
    if (input_file == NULL) {
        fputs("ERROR: Failed to open output file.\n", stderr);
        exit(EXIT_FAILURE);
    }

    if (debug_info) {
        size_t output_filename_len = strlen(output_filename);
        char* debug_filename = malloc(output_filename_len + 4);
        memcpy(debug_filename, output_filename, output_filename_len);
        strcpy(debug_filename + output_filename_len, ".od");

        debug_file = fopen(debug_filename, "wb");
        if (debug_file == NULL) {
            fputs("ERROR: Failed to open debug file.\n", stderr);
            exit(EXIT_FAILURE);
        }

        fprintf(debug_file, "; Onramp debug info for hex file\n");
        fprintf(debug_file, "#line 0 \"%s\"\n", input_filename);
    }

    /* do hex conversion */
    {
        char input_buffer[128];
        char output_buffer[128];
        char address_assertion[11];
        size_t output_pos = 0;
        size_t input_pos = 0;
        size_t input_count = 0;
        int address_assertion_length = 0;
        char first_hex_char = 0;
        BOOL in_comment = FALSE;
        BOOL in_address_assertion = FALSE;
        BOOL was_carriage_return = FALSE;
        BOOL was_backslash = FALSE;
        BOOL end_of_file = FALSE;

        /* TODO this loop is a bit messy. I probably should have put the read
         * character stuff in a separate function so that each parse element
         * can pull as many characters as it wants instead of having to set
         * state and loop back. I'll fix it eventually. */

        while (TRUE) {
            int b = 0;

            /* get next char (or -1 at end of file) */
            if (input_pos == input_count) {
                input_pos = 0;
                input_count = fread(input_buffer, 1, sizeof(input_buffer), input_file);
                if (input_count == 0) {
                    if (feof(input_file)) {
                        end_of_file = TRUE;
                    } else {
                        printf("ERROR: Failed to read from file around line %i\n", line);
                        exit(EXIT_FAILURE);
                    }
                }
            }
            b = end_of_file ? -1 : input_buffer[input_pos++];

            /* handle address assertions */
            if (in_address_assertion) {

                /* assemble the assertion */
                if (b == 'x' || b == 'X' || isxdigit(b)) {
                    if (address_assertion_length == 10) {
                        printf("ERROR: Address assertion is too long on line %i\n", line);
                        exit(EXIT_FAILURE);
                    }
                    if (address_assertion_length != 1 && (b == 'x' || b == 'X')) {
                        printf("ERROR: Invalid character in address assertion on line %i\n", line);
                        exit(EXIT_FAILURE);
                    }
                    address_assertion[address_assertion_length++] = b;
                    continue;
                }

                /* make sure it is followed by whitespace or end-of-file */
                if (b != -1 && !isspace(b)) {
                    printf("ERROR: Invalid character in address assertion on line %i\n", line);
                    exit(EXIT_FAILURE);
                }

                /* make the assertion */
                address_assertion[address_assertion_length] = 0;
                if (address_assertion_length < 2 ||
                        address_assertion[0] != '0' ||
                        (address_assertion[1] != 'x' && address_assertion[1] != 'X'))
                {
                    printf("ERROR: Address assertion must start with 0x on line %i\n", line);
                    exit(EXIT_FAILURE);
                }
                if (address_assertion_length == 2) {
                    printf("ERROR: Address assertion truncated on line %i\n", line);
                    exit(EXIT_FAILURE);
                }
                {
                    long value = strtol(address_assertion + 2, NULL, 16);
                    if (value != address) {
                        printf("ERROR: Address assertion failed on line %i: declared %s which is %li from actual address 0x%lX\n",
                                line, address_assertion, value - address, address);
                        exit(EXIT_FAILURE);
                    }
                }
                in_address_assertion = FALSE;

                /* The rest of the line is a comment, which we will use as a
                 * symbol name if debug output is on. We haven't handled the
                 * character yet so we keep going. */
                in_comment = TRUE;
                if (debug_info) {
                    in_symbol = TRUE;
                    symbol_pos = symbol_buffer;
                }
            }

            /* a backslash can only appear in a comment and not at the end of a */
            /* line. */
            if (b == '\\') {
                if (!in_comment) {
                    printf("ERROR: Unrecognized character at line %i\n", line);
                    exit(EXIT_FAILURE);
                }
                was_backslash = TRUE;
                continue;
            }
            if (was_backslash && (b == '\r' || b == '\n' || b == -1)) {
                printf("ERROR: Backslash found at end of line %i\n", line);
                exit(EXIT_FAILURE);
            }
            was_backslash = FALSE;

            /* if we've reached the end of the file, we're done */
            if (b == -1) {
                end_symbol();
                break;
            }

            /* check for a hex char */
            if (!in_comment && (
                        (b >= '0' && b <= '9') ||
                        (b >= 'A' && b <= 'F') ||
                        (b >= 'a' && b <= 'f')))
            {
                if (first_hex_char == 0) {
                    first_hex_char = b;
                    continue;
                }

                /* we have two consecutive hex chars */
                emit_debug_line();
                {
                    uint8_t value = (hex_char_to_value(first_hex_char) << 4) | hex_char_to_value(b);
                    output_buffer[output_pos++] = value;
                }
                if (output_pos == sizeof(output_buffer)) {
                    flush_output(output_file, output_buffer, output_pos);
                    output_pos = 0;
                }

                ++address;
                first_hex_char = 0;
                continue;
            }

            /* if we have a first hex char, it is an error to not follow with */
            /* another hex char. */
            if (first_hex_char != 0) {
                printf("ERROR: Incomplete hex byte after '%c' on line %i\n", first_hex_char, line);
                exit(EXIT_FAILURE);
            }

            /* check for a newline. we treat both carriage return and line feed */
            /* as newlines, except when they are together, in which case we */
            /* increment the line only once. */
            if (b == '\r') {
                end_symbol();
                in_comment = FALSE;
                ++line;
                was_carriage_return = TRUE;
                continue;
            }
            if (b == '\n') {
                end_symbol();
                in_comment = FALSE;
                if (!was_carriage_return)
                    ++line;
                was_carriage_return = FALSE;
                continue;
            }
            was_carriage_return = FALSE;

            /* if we're in a symbol, append the character. */
            if (in_symbol) {
                if (isspace(b)) {
                    if (symbol_pos != symbol_buffer) {
                        end_symbol();
                    }
                } else if (b != '=') { /* skip leading '=' if any */
                    *symbol_pos++ = b;
                    if (symbol_pos == symbol_buffer + sizeof(symbol_buffer) - 1) {
                        end_symbol();
                    }
                }
            }

            /* if we're in a comment, discard until newline */
            if (in_comment)
                continue;

            /* detect comment */
            if (b == ';' || b == '#') {
                in_comment = TRUE;
                continue;
            }

            /* start address assertions */
            if (b == '@') {
                address_assertion_length = 0;
                in_address_assertion = TRUE;
                continue;
            }

            /* skip whitespace */
            if (isspace(b))
                continue;

            /* anything else is an error */
            printf("ERROR: Unrecognized character at line %i\n", line);
            exit(EXIT_FAILURE);
        }

        /* check for truncation */
        if (first_hex_char != 0) {
            printf("ERROR: Truncated hex byte at end of file\n");
            exit(EXIT_FAILURE);
        }

        /* flush the rest of the buffer */
        flush_output(output_file, output_buffer, output_pos);
    }

    /* done */
    fclose(input_file);
    fclose(output_file);
    return EXIT_SUCCESS;
}
