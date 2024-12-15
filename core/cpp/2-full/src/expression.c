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

#include "expression.h"

#include <stdint.h>
#include <inttypes.h>

#include "stream.h"
#include "token.h"
#include "strings.h"

typedef struct number_t {
    bool is_signed;
    union {
        int64_t s;  // signed
        uint64_t u; // unsigned
    };
} number_t;

static bool number_is_true(number_t* number) {
    return (bool)number->u;
}

static void expression_parse(stream_t* stream, number_t* out);

/**
 * Returns the precedence of the given binary operator token, or -1 if it's not
 * a binary operator.
 */
static int expression_binary_precedence(token_t* operator) {
    if (operator->type != token_type_punctuation)
        return -1;

    if (token_is_punctuation(operator, STR_PIPE_PIPE))            return 1; // logical or
    if (token_is_punctuation(operator, STR_AMPERSAND_AMPERSAND))  return 2; // logical and
    if (token_is_punctuation(operator, STR_PIPE))                 return 3; // bitwise inclusive or
    if (token_is_punctuation(operator, STR_CARET))                return 4; // bitwise exclusive or
    if (token_is_punctuation(operator, STR_AMPERSAND))            return 5; // bitwise and
    if (token_is_punctuation(operator, STR_EQUAL_EQUAL))          return 6; // equality
    if (token_is_punctuation(operator, STR_NOT_EQUAL))            return 6; // ...
    if (token_is_punctuation(operator, STR_LESS))                 return 7; // relational
    if (token_is_punctuation(operator, STR_GREATER))              return 7; // ...
    if (token_is_punctuation(operator, STR_LESS_EQUAL))           return 7; // ...
    if (token_is_punctuation(operator, STR_GREATER_EQUAL))        return 7; // ...
    if (token_is_punctuation(operator, STR_LESS_LESS))            return 8; // shift
    if (token_is_punctuation(operator, STR_GREATER_GREATER))      return 8; // ...
    if (token_is_punctuation(operator, STR_PLUS))                 return 9; // additive
    if (token_is_punctuation(operator, STR_MINUS))                return 9; // ...
    if (token_is_punctuation(operator, STR_ASTERISK))             return 10; // multiplicative
    if (token_is_punctuation(operator, STR_SLASH))                return 10; // ...
    if (token_is_punctuation(operator, STR_PERCENT))              return 10; // ...

    return -1;
}

/**
 * Evaluates the given binary operator on the given values, leaving the result
 * in the left number.
 */
static void expression_binary_evaluate(token_t* operator, number_t* left, const number_t* right) {

    // The result of ||, &&, != and == is a boolean. This doesn't depend on the
    // sign of the operands and it promotes to signed regardless of the
    // signedness of the operands so we handle those first.
    if (token_is_punctuation(operator, STR_EQUAL_EQUAL)) {
        left->is_signed = true;
        left->u = left->u == right->u;
        return;
    }
    if (token_is_punctuation(operator, STR_NOT_EQUAL)) {
        left->is_signed = true;
        left->u = left->u != right->u;
        return;
    }
    if (token_is_punctuation(operator, STR_PIPE_PIPE)) {
        left->is_signed = true;
        left->u = left->u || right->u;
        return;
    }
    if (token_is_punctuation(operator, STR_PIPE_PIPE)) {
        left->is_signed = true;
        left->u = left->u || right->u;
        return;
    }

    // Otherwise we follow the normal type promotion rules: the result is
    // signed if and only if both operands are signed.
    bool is_signed = left->is_signed || right->is_signed;

    // Conditional operators evaluate differently based on the signedness of
    // the operands but the result is always a boolean which promotes to signed.
    if (token_is_punctuation(operator, STR_LESS)) {
        left->is_signed = true;
        if (is_signed) {
            left->s = left->s < right->s;
        } else {
            left->u = left->u < right->u;
        }
        return;
    }
    if (token_is_punctuation(operator, STR_GREATER)) {
        left->is_signed = true;
        if (is_signed) {
            left->s = left->s > right->s;
        } else {
            left->u = left->u > right->u;
        }
        return;
    }
    if (token_is_punctuation(operator, STR_LESS_EQUAL)) {
        left->is_signed = true;
        if (is_signed) {
            left->s = left->s <= right->s;
        } else {
            left->u = left->u <= right->u;
        }
        return;
    }
    if (token_is_punctuation(operator, STR_GREATER_EQUAL)) {
        left->is_signed = true;
        if (is_signed) {
            left->s = left->s >= right->s;
        } else {
            left->u = left->u >= right->u;
        }
        return;
    }

    // The remaining operators follow the sign promotion rules.
    left->is_signed = is_signed;

    // The calculation of these operators doesn't differ base on signedness but
    // the result has to follow promotion rules.
    if (token_is_punctuation(operator, STR_PIPE)) {
        left->u |= right->u;
        return;
    }
    if (token_is_punctuation(operator, STR_CARET)) {
        left->u ^= right->u;
        return;
    }
    if (token_is_punctuation(operator, STR_AMPERSAND)) {
        left->u &= right->u;
        return;
    }
    if (token_is_punctuation(operator, STR_PLUS)) {
        left->u += right->u;
        return;
    }
    if (token_is_punctuation(operator, STR_MINUS)) {
        left->u -= right->u;
        return;
    }
    if (token_is_punctuation(operator, STR_ASTERISK)) {
        left->u *= right->u;
        return;
    }
    if (token_is_punctuation(operator, STR_LESS_LESS)) {
        left->u <<= right->u;
        return;
    }

    // The remainng operators must be computed differently based on sign.
    if (is_signed) {
        if (token_is_punctuation(operator, STR_GREATER_GREATER)) {
            left->s >>= right->s;
            return;
        }
        if (token_is_punctuation(operator, STR_SLASH)) {
            left->s /= right->s;
            return;
        }
        if (token_is_punctuation(operator, STR_PERCENT)) {
            left->s %= right->s;
            return;
        }
    } else {
        if (token_is_punctuation(operator, STR_GREATER_GREATER)) {
            left->u >>= right->u;
            return;
        }
        if (token_is_punctuation(operator, STR_SLASH)) {
            left->u /= right->u;
            return;
        }
        if (token_is_punctuation(operator, STR_PERCENT)) {
            left->u %= right->u;
            return;
        }
    }

    fatal("Internal error: cannot evaluate unrecognized binary operator");
}

static void expression_parse_number(token_t* token, number_t* out) {
    const char* p = string_cstr(token->value);
    //trace("expression parsing number token %s\n", p);
    unsigned base = 0;

    // TODO assume decimal, need to port over cci/2 number parsing
    out->u = 0;
    base = 10;
    for (;*p;) {
        out->u *= base;
        out->u += *p++ - '0';
    }

    //trace("expression parsed %" PRIi64 "\n", out->s);
}

static void expression_parse_primary(stream_t* stream, number_t* out) {
    stream_skip_horizontal_space(stream);
    token_t* token = stream_peek(stream);

    // defined, parens, literal number, literal char.



    // TODO for now assume number

    // Parse a number
    if (token->type == token_type_number) {
        expression_parse_number(token, out);
        stream_consume(stream);
        return;
    }

    // Parse parens
    if (token_is_punctuation(token, STR_PAREN_OPEN)) {
        trace("Found open paren");
        stream_consume(stream);
        expression_parse(stream, out);
        stream_expect(stream, STR_PAREN_CLOSE,
                "Expected closing parenthesis in expression of #if/#elif directive.");
        return;
    }

    fatal_token(token, "Expected primary expression in #if/#elif directive.");
}

static void expression_parse_unary(stream_t* stream, number_t* out) {
    stream_skip_horizontal_space(stream);

    // Parse unary +
    if (stream_accept(stream, STR_PLUS)) {
        expression_parse_unary(stream, out);
        return;
    }

    // Parse unary -
    if (stream_accept(stream, STR_MINUS)) {
        expression_parse_unary(stream, out);
        if (out->is_signed) {
            out->s = -out->s;
        } else {
            out->u = -out->u;
        }
        return;
    }

    // Parse unary ~
    if (stream_accept(stream, STR_TILDE)) {
        expression_parse_unary(stream, out);
        out->u = -out->u;
        return;
    }

    // Parse unary !
    if (stream_accept(stream, STR_EXCLAMATION)) {
        expression_parse_unary(stream, out);
        out->u = !out->u;
        return;
    }

    expression_parse_primary(stream, out);
}

static void expression_parse_binary(stream_t* stream, number_t* out, int min_precedence) {

    // Parse the potential left-hand side
    expression_parse_unary(stream, out);

    for (;;) {

        // Collect a binary operator
        stream_skip_horizontal_space(stream);
        token_t* operator = stream_peek(stream);
        int precedence = expression_binary_precedence(operator);
        if (precedence < min_precedence)
            break;
        token_ref(operator);
        stream_consume(stream);

        // Parse the right-hand side
        number_t temp;
        expression_parse_binary(stream, &temp, min_precedence + 1);

        // Calculate
        expression_binary_evaluate(operator, out, &temp);
        token_deref(operator);

    }
}

static void expression_parse_conditional(stream_t* stream, number_t* out) {

    // Parse a potential predicate
    expression_parse_binary(stream, out, 0);

    // Collect '?'
    stream_skip_horizontal_space(stream);
    token_t* question = stream_peek(stream);
    if (!token_is_punctuation(question, STR_QUESTION))
        return;
    stream_consume(stream);

    // Parse true expression
    bool predicate = number_is_true(out);
    number_t temp;
    expression_parse(stream, predicate ? out : &temp);

    // Collect ':'
    stream_skip_horizontal_space(stream);
    token_t* colon = stream_peek(stream);
    if (!token_is_punctuation(question, STR_COLON)) {
        fatal_token(colon, "Expected : after ? expression in #if/#elif directive");
    }
    stream_consume(stream);

    // Parse false expression
    expression_parse_conditional(stream, predicate ? out : &temp);
}

static void expression_parse(stream_t* stream, number_t* out) {
    // The preprocessor doesn't support assignments or the comma operator. The
    // lowest priority operator supported is the ternary conditional. (GCC
    // supports comma but Clang does not, and the standard explicitly says
    // comma is not supported so we don't support it. I don't think there is
    // any way it could be useful.)
    expression_parse_conditional(stream, out);
}

bool expression_evaluate(stream_t* stream) {
    number_t number;
    expression_parse(stream, &number);

    // Make sure there are no extra tokens
    stream_skip_horizontal_space(stream);
    if (stream_peek(stream)->type != token_type_end) {
        fatal_token(stream_peek(stream), "Extra token after expression in #if/#elif directive");
    }

    return number_is_true(&number);
}
