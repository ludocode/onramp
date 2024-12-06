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

#include "libo-reader.h"

#include <string.h>
#include <assert.h>

#include "libo-error.h"
#include "libo-unicode.h"
#include "libo-string.h"

static void reader_open_file(reader_t* reader);
static void reader_fill_chars(reader_t* reader);

void reader_init_file(reader_t* reader, string_t* filename, FILE* file) {
    memset(reader, 0, sizeof(*reader));

    assert(filename);
    reader->filename = string_ref(filename);

    if (file) {
        reader->file = file;
    } else {
        reader_open_file(reader);
    }

    reader_fill_chars(reader);
}

void reader_init_bytes(reader_t* reader, char8_t* bytes, size_t count) {
    memset(reader, 0, sizeof(*reader));
    reader->bytes_start = bytes;
    reader->bytes_end = bytes + count;
    reader_fill_chars(reader);
}

void reader_destroy(reader_t* reader) {
    if (reader->file) {
        fclose(reader->file);
    }
    if (reader->filename != NULL) {
        string_deref(reader->filename);
    }
}

static void reader_open_file(reader_t* reader) {

    // Skip if this is a memory buffer
    if (reader->filename == NULL)
        return;

    assert(reader->file == NULL);
    reader->file = fopen(reader->filename->bytes, "rb");
    if (reader->file == NULL) {
        current_filename = reader->filename->bytes;
        current_line = 0;
        fatal("Failed to open file.");
    }
}

/**
 * Loads as many characters as possible from the file into the bytes[] buffer.
 */
static void reader_fill_bytes(reader_t* reader) {

    // Skip if this is a memory buffer
    if (reader->filename == NULL)
        return;

    // Skip if at end of file
    if (reader->end_of_file)
        return;

    // Reload the file if necessary
    if (reader->file == NULL) {
        reader_open_file(reader);
        fsetpos(reader->file, &reader->file_offset);
    }

    // Move any remaining bytes to the start of the buffer
    if (reader->bytes_start != reader->bytes_end) {
        memmove(reader->bytes, reader->bytes_start, reader->bytes_end - reader->bytes_start);
        reader->bytes_end -= reader->bytes_start - reader->bytes;
        reader->bytes_start = reader->bytes;
    } else {
        reader->bytes_end = reader->bytes;
        reader->bytes_start = reader->bytes;
    }

    // Fill the rest of the buffer
    char8_t* buffer_end = reader->bytes + READER_BYTES_CAPACITY;
    while (reader->bytes_end != buffer_end) {
        size_t step = fread(reader->bytes_end, 1, buffer_end - reader->bytes_end, reader->file);
        if (step == 0) {
            if (!feof(reader->file)) {
                fatal("Failed to read input file.");
            }
            reader->end_of_file = true;
            break;
        }
        reader->bytes_end += step;
    }
}

static void reader_fill_chars(reader_t* reader) {

    // Skip if at end of file
    if (reader->end_of_file && reader->bytes_start == reader->bytes_end)
        return;

    // Move any remaining chars to the start of the buffer
    if (reader->chars_start != reader->chars_end) {
        memmove(reader->chars, reader->chars_start,
                (reader->chars_end - reader->chars_start) * sizeof(char32_t));
        reader->chars_end -= reader->chars_start - reader->chars;
        reader->chars_start = reader->chars;
    } else {
        reader->chars_end = reader->chars;
        reader->chars_start = reader->chars;
    }

    // Decode as many chars as we can
    while (reader->chars_end != reader->chars + READER_CHARS_CAPACITY) {
        if (reader->bytes_end - reader->bytes_start < 4)
            reader_fill_bytes(reader);
        if (reader->bytes_end == reader->bytes_start)
            break;
        *reader->chars_end++ = utf8_decode(&reader->bytes_start, reader->bytes_end);
    }

}

char32_t reader_peek(reader_t* reader, size_t index) {
    if (reader->filename != NULL && index > READER_CHARS_CAPACITY)
        fatal("Internal error: Cannot peek beyond capacity of reader.");

    if (index >= (size_t)(reader->chars_end - reader->chars_start))
        reader_fill_chars(reader);
    if (index >= (size_t)(reader->chars_end - reader->chars_start))
        return READER_EOF;
    return reader->chars_start[index];
}

void reader_consume(reader_t* reader, size_t count) {
    if (reader->filename != NULL && count > READER_CHARS_CAPACITY)
        fatal("Internal error: Cannot consume beyond capacity of reader.");
    if (count > (size_t)(reader->chars_end - reader->chars_start))
        reader_fill_chars(reader);
    if (count > (size_t)(reader->chars_end - reader->chars_start))
        fatal("Internal error: Attempting to consume READER_EOF");
    reader->chars_start += count;
}

char32_t reader_next(reader_t* reader) {
    reader_consume(reader, 1);
    return reader_peek(reader, 0);
}

void reader_unload(reader_t* reader) {
    if (reader->file) {
        if (!reader->end_of_file) {
            if (0 != fgetpos(reader->file, &reader->file_offset)) {
                fatal("Failed to get file position when unloading file.");
            }
        }
        fclose(reader->file);
        reader->file = NULL;
    }
}
