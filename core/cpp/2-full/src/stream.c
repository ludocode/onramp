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

#include "stream.h"

#include <stdlib.h>

#include "lexer.h"
#include "macro.h"

void stream_init_lexer(stream_t* stream, struct lexer_t* lexer) {
    stream->lexer = lexer;
    stream->start = NULL;
    stream->end = NULL;
}

void stream_init_buffer(stream_t* stream, vector_t* buffer) {
    stream->lexer = NULL;
    stream->start = (token_t**)vector_start(buffer);
    stream->end = (token_t**)vector_end(buffer);
}

#ifdef DISABLED__
stream_t* stream_new_file(string_t* filename, token_t* /*nullable*/ source) {
    stream_t* stream = malloc(sizeof(stream_t));
    stream_init_file(stream, filename, source);
    return stream;
}

stream_t* stream_new_macro(macro_t* macro) {
    stream_t* stream = malloc(sizeof(stream_t));
    stream_init_macro(stream, macro);
    return stream;
}

void stream_delete(stream_t* stream) {
    stream_destroy(stream);
    free(stream);
}
#endif

token_t* stream_peek(stream_t* stream) {
    if (stream->lexer) {
        //printf("peek returning lexer %p\n", (void*)lexer_peek(stream->lexer));
        return lexer_peek(stream->lexer);
    }
    if (stream->start != stream->end) {
        //printf("peek returning buffer %p\n", vector_last(&stream->buffer));
        return *stream->start;
    }
    //printf("peek returning end %p\n", (void*)token_end);
    return token_end;
}

void stream_consume(stream_t* stream) {
    if (stream->lexer) {
        lexer_consume(stream->lexer);
    } else if (stream->start != stream->end) {
        // TODO do we deref tokens as we walk?
        ++stream->start;
    }
}

token_t* stream_take(stream_t* stream) {
    if (stream->lexer) {
        return lexer_take(stream->lexer);
    }
    if (stream->start != stream->end) {
        // TODO do we deref tokens as we walk?
        return token_ref(*stream->start++);
    }
    return token_ref(token_end);
}

void stream_dump_tokens(stream_t* stream) {
    printf("dumping %s stream tokens:\n", stream->lexer ? "lexer" : "buffer");

    if (stream_peek(stream)->type == token_type_end) {
        printf("    stream empty\n");
        return;
    }

    do {
        printf("    ");
        token_print(stream_peek(stream));
        stream_consume(stream);
    } while (stream_peek(stream)->type != token_type_end);
}

void stream_skip_horizontal_space(stream_t* stream) {
    assert(stream);
    fflush(stdout);
    while (stream_peek(stream)->type == token_type_space) {
        stream_consume(stream);
    }
}

void stream_skip_whitespace(stream_t* stream) {
    for (;;) {
        token_type_t type = stream_peek(stream)->type;
        if (type != token_type_space && type != token_type_newline)
            break;
        stream_consume(stream);
    }
}

bool stream_is(stream_t* stream, string_t* string) {
    token_t* token = stream_peek(stream);

    // quicker to check the string first since it's just a pointer comparison
    if (!string_equal(token->value, string))
        return false;

    // check the type
    switch (token->type) {
        case token_type_alphanumeric:
        case token_type_punctuation:
            break;
        default:
            return false;
    }

    return true;
}

void stream_expect(stream_t* stream, string_t* string, const char* error_message) {
    if (!stream_is(stream, string)) {
        fatal_token(stream_peek(stream), "%s", error_message);
    }
    stream_consume(stream);
}

bool stream_accept(stream_t* stream, string_t* string) {
    if (!stream_is(stream, string)) {
        return false;
    }
    stream_consume(stream);
    return true;
}

bool stream_accept_newline(stream_t* stream) {
    if (stream_peek(stream)->type != token_type_newline) {
        return false;
    }
    stream_consume(stream);
    return true;
}
