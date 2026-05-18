/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2026 Fraser Heavy Software
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

#include "parse.h"

#include "common.h"
#include "emit.h"
#include "label.h"
#include "symbol.h"

/** Starts a new file, either a real file or a file in a static archive. */
static void start_file(const char* new_filename) {
    ++file_index;
    set_current_filename(new_filename);
}

static void next_char(void) {
    //printf("last char %x\n",current_char);
    //printf("reading char...\n");

    current_char = fgetc(input_file);
    if (current_char == EOF) {
        if (!feof(input_file)) {
            fatal("Failed to read input file.");
        }
    }

    //printf("read char %x\n",current_char);
}

static void read_name(void) {
    // The first character can't be a digit
    if (!isalpha(current_char) && (current_char != '_') && (current_char != '$')) {
        //printf("%c\n", current_char);
        fatal("Expected label, symbol or directive name to start with a letter, underscore or dollar sign.");
    }

    buffer[0] = current_char;
    buffer_length = 1;
    next_char();

    // Subsequent chars can be digits
    while (isalnum(current_char) || (current_char == '_') || (current_char == '$')) {
        buffer[buffer_length++] = current_char;
        if (buffer_length == BUFFER_SIZE) {
            fatal("Name is too long.");
        }
        next_char();
    }

    buffer[buffer_length] = 0;
}

static void consume_horizontal_whitespace(void) {
    while (isspace(current_char) && !is_end_of_line(current_char)) {
        next_char();
    }
}

static bool try_parse_whitespace(void) {
    if (!isspace(current_char)) {
        return false;
    }

    bool was_carriage_return = (current_char == '\r');
    if (!line_manual && (was_carriage_return || (current_char == '\n'))) {
        if (!line_manual) {
            ++current_line;
            emit_source_location(NULL, current_line);
        }
    }
    next_char();

    // carriage return followed by line feed counts as a single line ending.
    if (was_carriage_return && (current_char == '\n')) {
        next_char();
    }

    return true;
}

static bool try_parse_comment(void) {
    if (current_char != ';') {
        return false;
    }

    // comment found. consume it
    while (1) {
        next_char();
        if (is_end_of_line(current_char)) {
            break;
        }
    }

    // note that we don't consume the carriage return or line feed. we let
    // try_parse_whitespace() do it so it handles line endings correctly.

    return true;
}

static bool try_parse_debug(void) {
    if (current_char != '#') {
        return false;
    }

    // debug line found. consume the '#'
    next_char();
    consume_horizontal_whitespace();

    // If it's a bare '#', it's a line increment. We only increment in manual
    // mode because we're not consuming the trailing '#'.
    if (is_end_of_line(current_char)) {
        if (line_manual) {
            ++current_line;
        }
        emit_increment_line(current_line - 1); // TODO get rid of argument
        return true;
    }

    // Read the directive type
    read_name();
    consume_horizontal_whitespace();

    // We currently only support #line.
    if (0 != strcmp(buffer, "line")) {
        fatal("Unrecognized debug directive");
    }

    // #line can be followed by the word "manual" to enable manual line increments
    if (isalpha(current_char)) {
        read_name();
        if (0 != strcmp(buffer, "manual")) {
            fatal("Unsupported command in #line directive.");
        }
        line_manual = true;
        consume_horizontal_whitespace();
        if (!is_end_of_line(current_char)) {
            fatal("Extra characters after `#line manual`.");
        }
        return true;
    }

    // Otherwise #line is followed by a line number
    current_line = 0;
    if (!isdigit(current_char)) {
        fatal("#line must be followed by a line number.");
    }
    do {
        int new_line = current_line * 10 + (current_char - '0');
        if (new_line <= current_line) {
            fatal("#line number is out of bounds.");
        }
        current_line = new_line;
        next_char();
    } while (isdigit(current_char));
    consume_horizontal_whitespace();

    // We reduce the given line number by 1 because we aren't going to consume
    // the line ending here.
    --current_line;

    // The filename is optional. If omitted, we emit without it
    if (is_end_of_line(current_char)) {
        emit_source_location(NULL, current_line + 1);
        return true;
    }

    // Otherwise we have to have a quoted filename
    if (current_char != '"') {
        fatal("Expected double-quote for optional filename in #line");
    }

    // Read the filename
    buffer_length = 0;
    for (;;) {
        next_char();
        if (current_char == '"') {
            break;
        }
        if (is_end_of_line(current_char)) {
            fatal("#line filename must begin and end with a double-quote");
        }
        buffer[buffer_length++] = current_char;
        if (buffer_length == BUFFER_SIZE) {
            fatal("Name is too long.");
        }
    }
    buffer[buffer_length] = 0;
    next_char();

    // Save the new filename
    set_current_filename(buffer);

    // The line must now end
    consume_horizontal_whitespace();
    if (!is_end_of_line(current_char)) {
        fatal("Unexpected trailing characters in #line directive");
    }

    // Garbage collected symbols are not emitted
    if (current_symbol && !current_symbol->is_used) {
        return true;
    }

    // Emit it
    emit_source_location(current_filename, current_line + 1);
    return true;
}

static bool try_parse_hex(void) {
    if (!isxdigit(current_char)) {
        return false;
    }

    int value;
    value = (hex_to_int(current_char) << 4);
    next_char();
    value = (value | hex_to_int(current_char));
    next_char();

    if (current_symbol == NULL) {
        fatal("Bytes cannot appear before the first symbol.");
    }

    current_address = (current_address + 1);

    // Garbage collected symbols are not emitted
    if (!current_symbol->is_used) {
        return true;
    }

    emit_byte(value);
    return true;
}

static bool try_parse_invocation(void) {
    char type;
    type = current_char;
    if ((type != '&') && (type != '^') && (type != '<') && (type != '>')) {
        return false;
    }

    if (current_symbol == NULL) {
        fatal("An invocation cannot appear before the first symbol.");
    }

    // increment current address. if ^ it's 4, otherwise it's 2.
    if (type == '^') {
        current_address = (current_address + 4);
    }
    if (type != '^') {
        current_address = (current_address + 2);
    }

    // read the label name into the buffer
    next_char();
    read_name();

    // we only check for validity and emit the address in the final pass
    if (!output_pass) {
        if (optimize && type != '&') {
            symbol_add_use(current_symbol, string_intern_bytes(buffer, buffer_length));
        }
        return true;
    }

    // find the label or symbol address
    string_t* name = string_intern_bytes(buffer, buffer_length);
    int address;
    if (type == '&') {
        label_t* label = symbol_find_label(current_symbol, name);
        if (!label) {
            fatal("Label not found: %s", buffer);
        }
        address = current_symbol->address + label->offset;
        //printf("invocation %c is label: sym %i + label %i == %i\n", type, (int)current_symbol->address, (int)label->offset, address);
    } else {
        symbol_t* symbol = symbols_find(name, file_index);
        if (!symbol) {
            fatal("Symbol not found: %s", buffer);
        }
        address = symbol->address;
        //printf("invocation %c is symbol %s address %i\n", type, symbol->name->bytes, address);
    }
    string_deref(name);

    // Garbage collected symbols are not emitted
    if (!current_symbol->is_used) {
        return true;
    }

    // emit the address
    if (type == '^') {
        emit_int(address);
    } else if (type == '<') {
        emit_short(address >> 16);
    } else if (type == '>') {
        emit_short(address);
    } else if (type == '&') {
        int offset = address - (current_address + current_symbol->address);
        if (offset < -0x8000 || offset > 0xFFFF) {
            fatal("Relative invocation out of bounds.");
        }
        if (offset & 0x3) {
            fatal("Relative invocation is misaligned.");
        }
        emit_short((unsigned)offset >> 2);
    }

    return true;
}

static void assign_current_symbol_size(void) {
    if (current_symbol != NULL) {
        //printf("Assigning size %i to symbol %s\n", current_address, current_symbol->name->bytes);
        current_symbol->size = current_address;
        current_symbol = NULL;
    }
}

static int parse_priority(void) {
    next_char();
    if (!isdigit(current_char))
        return -1;
    int priority = 0;
    do {
        priority *= 10;
        priority += current_char - '0';
        if (priority > 65535) {
            fatal("The maximum constructor/destructor priority is 65535.");
        }
        next_char();
    } while (isdigit(current_char));
    return priority;
}

static bool try_parse_symbol(void) {
    int type = current_char;
    if (type != '=' && type != '@') {
        return false;
    }
    next_char();

    bool weak = false;
    bool constructor = false;
    bool destructor = false;
    int constructor_priority = -1;
    int destructor_priority = -1;

    // parse flags
    for (;;) {
        switch (current_char) {
            case '?':
                if (weak)
                    fatal("Duplicate `?` flag on symbol definition.");
                weak = true;
                next_char();
                break;
            case '{':
                if (constructor)
                    fatal("Duplicate `{` flag on symbol definition.");
                constructor = true;
                constructor_priority = parse_priority();
                break;
            case '}':
                if (destructor)
                    fatal("Duplicate `}` flag on symbol definition.");
                destructor = true;
                destructor_priority = parse_priority();
                break;
            default:
                goto done_flags;
        }
    }
done_flags:

    // read the symbol name into the buffer
    read_name();
    string_t* name = string_intern_bytes(buffer, buffer_length);
    //printf("define symbol %c%s file %i\n", type, name->bytes, file_index);

    // symbols are defined only on the collection pass. on the output pass we
    // restore the current symbol and pad to a word boundary.
    if (output_pass) {

        // pad to a word boundary
        if (current_symbol && current_symbol->is_used) {
            for (int i = current_address; i & 3; ++i) {
                //printf("    emitting padding byte\n");
                emit_byte(0);
            }
        }

        // setup the new symbol
        current_address = 0;
        current_symbol = symbols_find(name, file_index);
        //printf("    symbol address %zu\n", current_symbol->address);
        assert(current_symbol != NULL);

        // emit debug info for it
        if (current_symbol->is_used) {
            emit_symbol_debug(name);
        }

        string_deref(name);
        return true;
    }

    // assign the size of the previous symbol
    assign_current_symbol_size();

    // define the new symbol
    //printf("symbol %s type %c index %i\n", buffer, type, file_index);
    symbol_t* symbol = symbols_define(name, file_index, type == '@');

    symbol->weak = weak;
    symbol->constructor = constructor;
    symbol->destructor = destructor;
    symbol->constructor_priority = constructor_priority;
    symbol->destructor_priority = destructor_priority;

    symbols_insert(symbol);

    current_symbol = symbol;
    current_address = 0;
    return true;
}

static bool try_parse_label(void) {
    char type = current_char;
    if (type != ':') {
        return false;
    }
    //printf("define label %c addr %i\n",type,current_address);

    if (current_symbol == NULL) {
        fatal("A label cannot appear outside of a symbol.");
    }

    // read the label name into the buffer
    next_char();
    read_name();

    // labels are defined in the collection pass and ignored during output.
    if (output_pass) {
        return true;
    }

    // check that this label isn't already defined
    string_t* name = string_intern_bytes(buffer, buffer_length);
    if (symbol_find_label(current_symbol, name) != 0) {
        fatal("Duplicate label definition");
    }
    if (symbols_find(name, file_index) != 0) {
        fatal("Label is already defined as a symbol");
    }

    // define the label
    label_t* label = symbol_define_label(current_symbol, name);
    label->offset = current_address;
    return true;
}

/** Parses a metadata line (file delimiter) in a static archive. */
static bool try_parse_archive(void) {
    if (current_char != '%') {
        return false;
    }
    //printf("found archive line, output_pass %i\n", output_pass);

    // Read the filename
    next_char();
    // TODO use arrays
    buffer_length = 0;
    while (!is_end_of_line(current_char)) {
        *(buffer + buffer_length) = current_char;
        buffer_length = (buffer_length + 1);
        if (buffer_length == BUFFER_SIZE) {
            fatal("Filename of file in archive is too long.");
        }
        next_char();
    }
    *(buffer + buffer_length) = 0;
    //printf("archive line file: %s\n",buffer);

    next_char(); // consume line ending
    current_line = 1;
    emit_source_location(buffer, current_line);
    start_file(buffer);
    return true;
}

/** Performs one step of parsing. */
static void parse(void) {
    //printf("char %c %x\n",current_char,current_char);
    if (try_parse_whitespace()) {
        return;
    }
    if (try_parse_hex()) {
        return;
    }
    if (try_parse_comment()) {
        return;
    }
    if (try_parse_debug()) {
        return;
    }
    if (try_parse_invocation()) {
        return;
    }
    if (try_parse_symbol()) {
        return;
    }
    if (try_parse_label()) {
        return;
    }
    if (try_parse_archive()) {
        return;
    }
    fatal("Invalid character.");
}

/** Performs a full pass of parsing on the given input file. */
static void perform_pass_input(const char* input_filename) {
    start_file(input_filename);
    current_line = 0;

    input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        fatal("Failed to open input file.");
    }

    current_char = 0;
    next_char();
    current_line = 1;
    emit_source_location(input_filename, current_line);

    while (current_char != EOF) {
        parse();
    }

    // assign the size of the last symbol in the file
    if (!output_pass) {
        assign_current_symbol_size();
    }

    fclose(input_file);
}

void perform_pass(const char** input_filenames, size_t input_filenames_count) {
    current_address = 0;
    file_index = -1;

    for (size_t i = 0; i < input_filenames_count; ++i) {
        perform_pass_input(input_filenames[i]);
    }
}
