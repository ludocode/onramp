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

#include "libo-unicode.h"
#include "stream.h"
#include "token.h"
#include "strings.h"
#include "macro.h"

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
    //trace("binary evaluate %u %s %u\n", (unsigned)left->u, operator->value->bytes, (unsigned)right->u);

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
    if (token_is_punctuation(operator, STR_AMPERSAND_AMPERSAND)) {
        left->is_signed = true;
        left->u = left->u && right->u;
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

    fatal("Internal error: cannot evaluate unrecognized binary operator %s", operator->value->bytes);
}

static void expression_parse_defined(stream_t* stream, number_t* out) {
    stream_skip_horizontal_space(stream);
    //trace("Parsing defined. Next token: "); token_print(stream_peek(stream));

    token_t* identifier;
    if (stream_peek(stream)->type == token_type_alphanumeric) {
        identifier = stream_take(stream);
        //trace("Defined found non-paren identifier: "); token_print(identifier);
    } else if (stream_accept(stream, STR_PAREN_OPEN)) {
        stream_skip_horizontal_space(stream);
        identifier = stream_take(stream);
        //trace("Found paren identifier: "); token_print(identifier);
        stream_skip_horizontal_space(stream);
        stream_expect(stream, STR_PAREN_CLOSE,
                "Expected closing parenthesis after macro name in `defined(`.");
    } else {
        //trace("Defined paren identifier not found!\n");
        identifier = token_end;
    }

    if (identifier->type != token_type_alphanumeric) {
        fatal_token(identifier, "Expected macro name after `defined` in expression of #if/#elif directive.");
    }

    out->is_signed = true;
    out->s = NULL != macro_find(identifier->value);
    token_deref(identifier);
}

static bool expression_try_parse_number(stream_t* stream, number_t* out) {
    token_t* token = stream_peek(stream);
    if (token->type != token_type_number)
        return false;

    const char* p = string_cstr(token->value);
    //trace("expression parsing number token %s\n", p);
    unsigned base = 0;

    // TODO this is copy-pasted from cci/2. Probably should be shared in libo,
    // but it's not straightforward because it needs to be compilable without
    // `long long`, and we don't really want to move all of cpp/2's u64_t
    // arithmetic into libo. plus all the error handling is an issue.

    // TODO this is written in omC. it should be normal C.

    // detecting leading 0x/0X for hex
    if (*p == '0') {
        char x = *(p + 1);
        if ((x == 'x') | (x == 'X')) {
            base = 16;
            p = (p + 2);
        }
    }

    // detect leading binary 0b/0B for binary
    if (base == 0) {
        if (*p == '0') {
            char b = *(p + 1);
            if ((b == 'b') | (b == 'B')) {
                // TODO binary number literals are C23 only
                base = 2;
                p = (p + 2);
            }
        }
    }

    // detect leading 0 for octal
    if (base == 0) {
        if (*p == '0') {
            base = 8;
        }
    }

    // otherwise assume decimal
    if (base == 0) {
        base = 10;
    }

    // an octal constant is allowed to have a digit separator after the 0
    // prefix. other prefixes are not.
    if (base != 8 && *p == '\'') {
        // TODO this should probably be a warning
        fatal_token(token, "A digit separator is not allowed between an 0x/0b prefix and the first digit.");
    }

    // accumulate digits
    bool was_separator;
    uint64_t value = 0;
    while (1) {
        if (*p == '\'') {
            // TODO digit separators are C23 only
            p = (p + 1);
            was_separator = true;
            continue;
        }

        // TODO hex_to_int in libo
        unsigned digit = 99;
        if ((*p >= '0') & (*p <= '9')) {
            digit = (*p - '0');
        }
        if ((*p >= 'a') & (*p <= 'f')) {
            digit = ((*p - 'a') + 10);
        }
        if ((*p >= 'A') & (*p <= 'F')) {
            digit = ((*p - 'A') + 10);
        }
        if (digit >= base) {
            break;
        }
        was_separator = false;

        // Add the digit, checking for overflow
        uint64_t temp = value * base;
        if (temp < value) {
            goto out_of_range;
        }
        temp += digit;
        if (temp < value) {
            goto out_of_range;
        }
        value = temp;

        p = (p + 1);
    }

    if (was_separator) {
        // TODO this should probably be a warning
        fatal_token(token, "A digit separator is not allowed at the end of a number.");
    }

    // parse out the suffix
    bool suffix_unsigned = false;
    bool suffix_long = false;
    bool suffix_long_long = false;
    while (*p) {

        // parse long
        if ((*p == 'l') | (*p == 'L')) {
            if (suffix_long_long) {
                fatal_token(token, "`long long long` integer suffix is not supported.");
            }
            if (suffix_long) {
                suffix_long = false;
                suffix_long_long = true;
            } else {
                suffix_long = true;
            }
            p = (p + 1);
            continue;
        }

        // parse unsigned
        if ((*p == 'u') | (*p == 'U')) {
            if (suffix_unsigned) {
                fatal_token(token, "Redundant `u` suffix on integer literal.");
            }
            suffix_unsigned = true;
            p = (p + 1);
            continue;
        }

        // unrecognized. try to give slightly better error mesagge
        if (((*p == '.') | ((*p == 'e') | (*p == 'E'))) |
                ((*p == 'p') | (*p == 'P')))
        {
            fatal_token(token, "TODO floating point literals are not yet supported");
        }
        fatal_token(token, "Malformed number literal.");
    }

    out->u = value;
    out->is_signed = !suffix_unsigned;

    //trace("expression parsed %" PRIi64 "\n", out->s);
    stream_consume(stream);
    return true;

out_of_range:
    fatal_token(token, "Number does not fit in a 64-bit integer.");
}

static void expression_parse_primary(stream_t* stream, number_t* out) {
    stream_skip_horizontal_space(stream);

    // Parse `defined`
    if (stream_accept(stream, STR_DEFINED)) {
        expression_parse_defined(stream, out);
        return;
    }

    // Parse a number
    if (expression_try_parse_number(stream, out)) {
        return;
    }

    // Parse parens
    if (stream_accept(stream, STR_PAREN_OPEN)) {
        //trace("Found open paren");
        expression_parse(stream, out);
        stream_expect(stream, STR_PAREN_CLOSE,
                "Expected closing parenthesis in expression of #if/#elif directive.");
        return;
    }

    // Parse literal character
    if (stream_peek(stream)->type == token_type_character) {
        token_t* token = stream_take(stream);
        // TODO fix const in unicode funcs
        char8_t* p = (char8_t*)string_cstr(token->value);
        char8_t* end = p + string_length(token->value);

        char32_t value = utf8_decode(&p, end);
        if (value == '\\') {
            // TODO share escape sequence parsing code with other parts of
            // cpp/2 that need it (e.g. emit_pragma())
            if (p == end) {
                fatal_token(token, "Truncated character escape sequence.");
            }
            value = utf8_decode(&p, end);
            switch (value) {
                case 'a':   value = '\a';  break;  // bell
                case 'b':   value = '\b';  break;  // backspace
                case 't':   value = '\t';  break;  // horizontal tab
                case 'n':   value = '\n';  break;  // line feed
                case 'v':   value = '\v';  break;  // vertical tab
                case 'f':   value = '\f';  break;  // form feed
                case 'r':   value = '\r';  break;  // carriage return
                case 'e':   value = 27;    break;  // escape (extension, not standard C)
                case '"':   value = '"';   break;  // double quote
                case '\'':  value = '\'';  break;  // single quote
                case '?':   value = '?';   break;  // question mark
                case '\\':  value = '\\';  break;  // backslash

                case '0': case '1': case '2': case '3':
                case '4': case '5': case '6': case '7':
                case 'x': case 'X':
                case 'u': case 'U':
                    fatal_token(token, "Octal, hexadecimal and unicode escape sequences are not implemented in preprocessor expressions.");
                    break;

                default:
                    fatal_token(token, "Unrecognized character escape sequence in preprocessor expression.");
                    break;
            }
        }

        out->u = (uint32_t)value;
        token_deref(token);
        return;
    }

    // Any identifier that wasn't expanded as a macro is treated as a zero.
    // TODO warn -Wundef
    if (stream_peek(stream)->type == token_type_alphanumeric) {
        stream_consume(stream);
        out->u = 0;
        return;
    }

    fatal_token(stream_peek(stream), "Expected primary expression in #if/#elif directive.");
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
        //trace("precedence %i for %s, min_precedence %i\n", precedence, operator->value->bytes, min_precedence);
        if (precedence < min_precedence)
            break;
        token_ref(operator);
        stream_consume(stream);

        // Parse the right-hand side
        number_t temp;
        expression_parse_binary(stream, &temp, precedence + 1);

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
    if (!stream_accept(stream, STR_QUESTION))
        return;
    //trace("found ?\n");

    // Parse true expression
    bool predicate = number_is_true(out);
    number_t temp;
    expression_parse(stream, predicate ? out : &temp);

    // Collect ':'
    stream_skip_horizontal_space(stream);
    stream_expect(stream, STR_COLON,
        "Expected : after ? expression in #if/#elif directive");

    // Parse false expression
    expression_parse_conditional(stream, predicate ? &temp : out);
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
