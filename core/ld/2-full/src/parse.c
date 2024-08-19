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

#include "parse.h"

#include "common.h"
#include "emit.h"
#include "label.h"
#include "symbol.h"

/** Starts a new file, either a real file or a file in a static archive. */
static void start_file(const char* new_filename) {
    ++file_index;
    set_current_filename(new_filename);
    if (pass != 0) {
        labels_clear();
    }
}

static void save_file_state(void) {
    // We store the state of all these things so we can restore them later.
    if (0 != fgetpos(input_file, &file_start_pos))
        fatal("Failed to seek input file.");
    file_start_address = current_address;
    file_first_char = current_char;
}

/** Restarts a file for another parsing pass. */
static void restore_file_state(void) {
    if (0 != fsetpos(input_file, &file_start_pos))
        fatal("Failed to seek input file.");
    current_line = 1;
    current_address = file_start_address;
    current_char = file_first_char;
    emit_source_location(current_filename, 1);
}

static void next_char(void) {
    //printf("last char %x\n",current_char);
    //printf("reading char...\n");
    if (1 != fread(&current_char, 1, 1, input_file)) {
        //printf("EOF\n");
        if (!feof(input_file)) {
            fatal("Failed to read input file.");
        }
        current_char = EOF;
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
        fatal("Bytes cannot appear outside of a symbol.");
    }

    current_address = (current_address + 1);
    emit_byte(value);
    return true;
}

static bool try_parse_invoke(void) {
    char type;
    type = current_char;
    if ((type != '&') && (type != '^') && (type != '<') && (type != '>')) {
        return false;
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
    if (pass != 3) {
        return true;
    }

    // find the label or symbol address
    int address;
    label_t* label = labels_find(buffer, buffer_length);
    if (label) {
        address = label->symbol->address + label->address;
        //printf("invocation %c is label: sym %i + label %i == %i\n", type, (int)label->symbol->address, (int)label->address, address);
    } else {
        symbol_t* symbol = symbols_find(buffer, buffer_length, file_index);
        if (!symbol) {
            fatal("Definition not found: %s", buffer);
        }
        address = symbol->address;
        //printf("invocation %c is symbol: %i\n", type, address);
    }

    // emit the address
    if (type == '^') {
        emit_int(address);
    }
    if (type == '<') {
        emit_short(address >> 16);
    }
    if (type == '>') {
        emit_short(address);
    }
    if (type == '&') {
        int offset = address - (current_address + current_symbol->address);
        if ((offset < -0x8000) | (offset > 0xFFFF)) {
            fatal("Relative invocation out of bounds.");
        }
        if (offset & 0x3) {
            fatal("Relative invocation is misaligned.");
        }
        offset >>= 2;
        emit_short(offset);
    }
    return true;
}

static void assign_current_symbol_size(void) {
    if (current_symbol != NULL) {
        //printf("Assigning size %i to symbol %s\n", current_address, current_symbol->name->bytes);
        current_symbol->size = current_address;
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
    bool zero = false;
    bool constructor = false;
    bool destructor = false;
    int constructor_priority;
    int destructor_priority;

    // parse flags
    for (;;) {
        switch (current_char) {
            case '?':
                if (weak)
                    fatal("Duplicate `?` flag on symbol definition.");
                weak = true;
                next_char();
                break;
            case '+':
                if (zero)
                    fatal("Duplicate `+` flag on symbol definition.");
                zero = true;
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
    //printf("define symbol %c%s %i\n",type,buffer,file_index);

    // symbols are defined only on the first pass. on subsequent passes we
    // restore the current symbol and pad to a word boundary.
    if (pass != 0) {
        for (int i = current_address; i & 3; ++i) {
            emit_byte(0);
        }
        current_address = 0;
        current_symbol = symbols_find(buffer, buffer_length, file_index);
        assert(current_symbol != NULL);
        emit_symbol(buffer);
        return true;
    }

    // assign the size of the previous symbol
    assign_current_symbol_size();

    // define the new symbol
    //printf("symbol %s index %i\n", buffer,(type == '=') ? -1 : file_index);
    symbol_t* symbol = symbols_define(buffer, buffer_length,
            (type == '=') ? -1 : file_index);

    symbol->weak = weak;
    symbol->zero = zero;
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

    // locals are defined in pass 2.
    if (pass != 2) {
        return true;
    }

    // check that this label isn't already defined
    if (labels_find(buffer, buffer_length) != 0) {
        fatal("Duplicate label definition");
    }
    if (symbols_find(buffer, buffer_length, file_index) != 0) {
        fatal("Label is already defined as a symbol");
    }

    // define the label
    label_t* label = labels_define(buffer, buffer_length);
    label->symbol = current_symbol;
    label->address = current_address;
    return true;
}

/** Parses a metadata line (file delimiter) in a static archive. */
static bool try_parse_archive(void) {
    if (current_char != '%') {
        return false;
    }
    //printf("found archive line, pass %i\n", pass);

    // If we're in pass 2, we jump back to the top and start pass 3.
    if (pass == 2) {
        //printf("restoring file state, moving to pass 2\n");
        pass = 3;
        restore_file_state();

        // We haven't parsed the line ending of the archive metadata yet so we
        // start at line 0.
        current_line = 0;
        return true;
    }

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

    // If we're in pass 3, we'll start pass 2 for the next file
    if (pass == 3) {
        //printf("saving file state, moving to pass 1\n");
        pass = 2;
        save_file_state();
    }

    // We haven't parsed the line ending of the archive metadata yet so we
    // start at line 0.
    current_line = 0;
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
    if (try_parse_invoke()) {
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
    save_file_state();
    current_line = 1;

    while (1) {
        if (current_char == EOF) {
            if (pass == 2) {
                pass = 3;
                restore_file_state();
                continue;
            }
            if (pass == 3) {
                // Pass three of this file or archive is done. Switch back to
                // pass two for the next file.
                pass = 2;
            }
            break;
        }
        parse();
    }

    symbols_emit_generated();
    fclose(input_file);
}

void perform_pass(const char** input_filenames, size_t input_filenames_count) {
    current_address = 0;
    file_index = -1;

    for (size_t i = 0; i < input_filenames_count; ++i) {
        perform_pass_input(input_filenames[i]);
    }

    // assign the size of the last symbol
    if (pass == 0) {
        assign_current_symbol_size();
    }
}
