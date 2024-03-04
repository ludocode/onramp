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



// TODO asprintf() is nonstandard (is it in C23?)
#define _GNU_SOURCE

#include "debug.h"
#include "libo-string.h"



/*
 * Debug file parsing
 *
 * We store each run of bytes in a particular source location as a "debug
 * block". Blocks are stored in order in a flat array which we can binary
 * search. Strings are interned (or they are string literals) to avoid
 * duplicating memory for every block.
 *
 * TODO need to be able to load and unload at runtime. Will be adding an
 * optional syscall to load/unload debug info for spawned programs. Need to
 * replace the below string interning with libo which supports deleting
 * interned strings (when refcount reaches zero.)
 */

// TODO be able to unload debug info, properly deref these strings
// TODO libo now has an intern table, just use that
static const char* intern(const char* cstr) {
    string_t* string = string_intern_cstr(cstr);
    return string->bytes;
}

typedef struct debug_block_t {
    const char* symbol;
    const char* filename;
    int line;
    size_t start_address;
    size_t end_address;
} debug_block_t;

debug_block_t* debug_blocks;
size_t debug_blocks_capacity;
size_t debug_blocks_count;

static void debug_add_block(const char* symbol, const char* filename,
        int line, size_t address, size_t byte_count)
{
    if (byte_count == 0) {
        return;
    }
    //printf("add block: %s %s:%i 0x%zX %zi\n", symbol, filename, line, address, byte_count);

    // grow if needed
    if (debug_blocks_count == debug_blocks_capacity) {
        size_t new_capacity = debug_blocks_capacity * 2;
        if (new_capacity < debug_blocks_capacity) {
            fatal("Out of memory.");
        }
        debug_block_t* new_blocks = realloc(debug_blocks, new_capacity * sizeof(debug_block_t));
        if (new_blocks == NULL) {
            fatal("Out of memory.");
        }
        debug_blocks = new_blocks;
        debug_blocks_capacity = new_capacity;
    }

    debug_block_t* block = debug_blocks + debug_blocks_count++;
    block->filename = filename;
    block->symbol = symbol;
    block->line = line;
    block->start_address = address;
    block->end_address = address + byte_count;
}

void debug_load(const char* executable_filename, size_t address) {
    char* debug_filename = 0;
    if (-1 == asprintf(&debug_filename, "%s.od", executable_filename)) {
        fatal("Out of memory.");
    }
    FILE* file = fopen(debug_filename, "r");
    free(debug_filename);

    if (file == NULL) {
        //printf("No debug info.\n");
        return;
    }
    if (debug_blocks_count > 0) {
        fatal("Debug info already loaded.");
    }

    const char* current_filename = "<unknown>";
    const char* current_symbol = "<unknown>";
    int current_linenum = 0;
    size_t current_address = address;

    /*
    size_t buffer_capacity = 256;
    size_t buffer_size = 0;
    char* buffer = malloc(buffer_capacity);
    if (buffer == NULL) {
        fatal("Out of memory.");
    }
    */
    // TODO for now we just use a fixed line size
    char buffer[256];
    size_t byte_count = 0;
    int debug_linenum = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        ++debug_linenum;
        char* line = buffer;

        // skip leading whitespace
        while (isspace(*line)) {
            ++line;
        }

        // skip comments
        if (line[0] == ';') {
            continue;
        }

        // parse a number
        if (line[0] != '#') {
            int value;
            int chars;
            if (1 != sscanf(line, "%d%n", &value, &chars)) {
                fprintf(stderr, "ERROR: Invalid number line in debug file at line %i\n", debug_linenum);
                fatal("Invalid debug info");
            }
            line += chars;
            byte_count += value;

            // make sure there's nothing else besides the number
            while (isspace(*line)) {
                ++line;
            }
            if (*line != 0) {
                fprintf(stderr, "ERROR: Invalid number line in debug file at line %i\n", debug_linenum);
                fatal("Invalid debug info");
            }
            continue;
        }

        // skip #
        ++line;
        while (isspace(*line)) {
            ++line;
        }

        // if it's a bare #, it's a line increment. finish the previous block.
        if (*line == 0) {
            debug_add_block(current_symbol, current_filename, current_linenum, current_address, byte_count);
            current_address += byte_count;
            byte_count = 0;
            ++current_linenum;
        }

        // see if it's a symbol directive
        if (0 == strncmp(line, "symbol", 6)) {
            line += 6;
            if (!isspace(*line)) {
                fprintf(stderr, "ERROR: Invalid directive in debug file at line %i\n", debug_linenum);
                fatal("Invalid debug info");
            }

            // skip whitespace
            while (isspace(*line)) {
                ++line;
            }

            // finish the previous block
            debug_add_block(current_symbol, current_filename, current_linenum, current_address, byte_count);
            current_address += byte_count;
            byte_count = 0;

            // following identifier is the symbol name
            const char* new_symbol = line;
            while (!isspace(*line)) {
                ++line;
            }
            *line = 0;
            current_symbol = intern(new_symbol);
            continue;
        }

        // make sure "line" is next
        if (0 != strncmp(line, "line", 4)) {
            // not a line directive. ignore it.
            continue;
        }
        line += 4;

        // parse the new line number
        int new_linenum;
        int chars;
        if (1 != sscanf(line, "%d%n", &new_linenum, &chars)) {
            fprintf(stderr, "ERROR: Invalid number line in debug file at line %i\n", debug_linenum);
            fatal("Invalid debug info");
        }
        line += chars;
        while (isspace(*line)) {
            ++line;
        }

        // parse optional filename
        char* new_filename = NULL;
        if (*line == '"') {
            new_filename = line + 1;
            char* end = strchr(new_filename, '"');
            if (end == NULL) {
                fprintf(stderr, "ERROR: Invalid filename in debug file at line %i\n", debug_linenum);
                fatal("Invalid debug info");
            }
            *end = 0;
            line = end + 1;
        }

        // skip whitespace
        while (isspace(*line)) {
            ++line;
        }

        // this should be the end of the line.
        if (*line != 0) {
            fprintf(stderr, "ERROR: Invalid #line directive in debug file at line %i\n", debug_linenum);
            fatal("Invalid debug info");
        }

        // finish the previous block
        debug_add_block(current_symbol, current_filename, current_linenum, current_address, byte_count);
        current_address += byte_count;
        byte_count = 0;
        current_filename = intern(new_filename);
        current_linenum = new_linenum;
    }

    fclose(file);
}

bool debug_find(size_t address, const char** out_symbol, const char** out_filename, int* out_line) {
    *out_filename = "<unknown>";
    *out_line = 0;

    // make sure we have debug info
    if (debug_blocks_count == 0) {
        return false;
    }

    // check if the address is in range of our debug info
    int left = 0;
    int right = debug_blocks_count - 1;
    if (address < debug_blocks[left].start_address || address >= debug_blocks[right].end_address) {
        return false;
    }

    // binary search
    while (left != right) {
        int mid = (left + right) / 2;
        if (address < debug_blocks[mid].end_address) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }

    // found
    debug_block_t* block = debug_blocks + left;
    *out_symbol = block->symbol;
    *out_filename = block->filename;
    *out_line = block->line;
    return true;
}



/*
 * Stack frame tracking
 */

typedef struct debug_frame_t {
    uint32_t source_address;
    uint32_t return_address;
    bool tail_call;
} debug_frame_t;

debug_frame_t* debug_frames;
size_t debug_frames_capacity;
size_t debug_frames_count;

void debug_callstack_push(uint32_t source_address, uint32_t return_address, bool tail_call) {
    //printf("DEBUG CALLSTACK PUSH\n");

    // grow if needed
    if (debug_frames_count == debug_frames_capacity) {
        size_t new_capacity = debug_frames_capacity * 2;
        if (new_capacity <= debug_frames_capacity) {
            fatal("Out of memory.");
        }
        debug_frame_t* new_frames = realloc(debug_frames, new_capacity * sizeof(debug_frame_t));
        if (new_frames == NULL) {
            fatal("Out of memory.");
        }
        debug_frames = new_frames;
        debug_frames_capacity = new_capacity;
    }

    debug_frame_t* frame = debug_frames + debug_frames_count++;
    frame->source_address = source_address;
    frame->return_address = return_address;
    frame->tail_call = tail_call;
}

void debug_callstack_pop(uint32_t return_address) {
    //printf("DEBUG CALLSTACK POP\n");
    while (debug_frames_count > 0 && debug_frames[debug_frames_count - 1].return_address == return_address) {
        --debug_frames_count;
    }
}

bool debug_print_location(uint32_t address, bool hide_path) {
    const char* filename;
    const char* symbol;
    int line;
    if (!debug_find(address, &symbol, &filename, &line)) {
        return false;
    }

    // remove path if requested
    if (hide_path) {
        const char* short_filename = strrchr(filename, '/');
        if (short_filename) {
            filename = short_filename + 1;
        }
    }

    printf("%s() %s", symbol, filename);
    if (line > 0) {
        printf(":%i", line);
    }
    return true;
}

void debug_callstack_print_line(uint32_t address, bool tail_call) {
    printf("  %s 0x%X ", tail_call ? "*" : " ", address);
    debug_print_location(address, false);
    putchar('\n');
}

void debug_callstack_print(uint32_t current_address) {
    //printf("stack trace:\n");
    debug_callstack_print_line(current_address, false);
    for (size_t i = debug_frames_count; i > 0;) {
        --i;
        debug_frame_t* frame = &debug_frames[i];
        debug_callstack_print_line(frame->source_address, frame->tail_call);
    }
}

bool debug_stack_has_return(uint32_t return_address) {
    if (debug_frames_count == 0) {
        return false;
    }
    return return_address == debug_frames[debug_frames_count - 1].return_address;
}



/*
 * Init
 */

void debug_init(void) {
    debug_blocks_capacity = 128;
    debug_blocks = malloc(debug_blocks_capacity * sizeof(debug_block_t));
    debug_frames_capacity = 16;
    debug_frames = malloc(debug_frames_capacity * sizeof(debug_frame_t));
}

void debug_destroy(void) {
    free(debug_blocks);
    free(debug_frames);
}
