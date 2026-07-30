/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2026 Fraser Heavy Software
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

void stream_init(stream_t* stream, bool use_lexer, vector_t* /*nullable*/ buffer) {
    vector_init(&stream->stack);
    if (buffer) {
        stream->buffer_start = vector_start(buffer);
        stream->buffer_end = vector_end(buffer);
    } else {
        stream->buffer_start = NULL;
        stream->buffer_end = NULL;
    }
    stream->use_lexer = use_lexer;
}

void stream_destroy(stream_t* stream) {
    // It probably doesn't make sense to ever destroy a stack that still has
    // tokens in it.
    if (!vector_is_empty(&stream->stack)) {
        fatal("Internal error: cannot destroy stream with non-empty stack");
    }
    /*
    for (size_t i = vector_count(&stream->stack); i-- > 0;) {
        token_deref(vector_at(&stream->stack, i));
    }
    */
    vector_destroy(&stream->stack);
}

token_t* stream_peek(stream_t* stream) {
    if (!vector_is_empty(&stream->stack)) {
        return vector_last(&stream->stack);
    }
    if (stream->buffer_start != stream->buffer_end) {
        return *stream->buffer_start;
    }
    if (stream->use_lexer) {
        return lexer_peek(lexer_current);
    }
    return token_end;
}

void stream_consume(stream_t* stream) {
    if (!vector_is_empty(&stream->stack)) {
        token_deref(vector_remove_last(&stream->stack));
        return;
    }
    if (stream->buffer_start != stream->buffer_end) {
        ++stream->buffer_start;
        return;
    }
    if (stream->use_lexer) {
        lexer_consume(lexer_current);
        return;
    }
    fatal("Internal error: Stream cannot consume end token.");
}

token_t* stream_take(stream_t* stream) {
    if (!vector_is_empty(&stream->stack)) {
        return vector_remove_last(&stream->stack);
    }
    if (stream->buffer_start != stream->buffer_end) {
        return token_ref(*stream->buffer_start++);
    }
    if (stream->use_lexer) {
        return lexer_take(lexer_current);
    }
    fatal("Internal error: Stream cannot take end token.");
}

void stream_print_stack(stream_t* stream) {
    if (vector_is_empty(&stream->stack)) {
        printf("    stack is empty.\n");
    } else for (size_t i = vector_count(&stream->stack); i-- != 0;) {
        printf("    ");
        token_print(vector_at(&stream->stack, i));
    }
}

void stream_dump_tokens(stream_t* stream) {
    printf("dumping stream tokens:\n");

    printf("  stack:\n");
    if (vector_is_empty(&stream->stack)) {
        printf("    stack is empty.\n");
    } else while (!vector_is_empty(&stream->stack)) {
        printf("    ");
        token_print(stream_peek(stream));
        stream_consume(stream);
    }

    printf("  buffer:\n");
    if (stream->buffer_start == stream->buffer_end) {
        printf("    buffer is empty.\n");
    } else while (stream->buffer_start != stream->buffer_end) {
        printf("    ");
        token_print(stream_peek(stream));
        stream_consume(stream);
    }

    printf("  lexer:\n");
    if (!stream->use_lexer) {
        printf("    lexer is disabled.\n");
    } else if (stream_peek(stream)->type == token_type_end) {
        printf("    lexer is empty.\n");
    } else do {
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

void stream_push(stream_t* stream, struct token_t* token) {
    //trace("STREAM PUSH "); token_print(token);
    vector_append(&stream->stack, token);
}

void stream_reserve(stream_t* stream, size_t count) {
    vector_reserve(&stream->stack, vector_count(&stream->stack) + count);
}

void stream_forward_spaces(stream_t* stream, vector_t* /*nullable*/ output) {
    while (stream_peek(stream)->type == token_type_space) {
        output_token(output, stream_peek(stream));
        stream_consume(stream);
    }
}
