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

#include "node.h"

#include <stdlib.h>

#include "common.h"
#include "type.h"
#include "token.h"
#include "symbol.h"
#include "options.h"
#include "lexer.h"

const char* node_kind_to_string(node_kind_t kind) {
    switch (kind) {
        case NODE_INVALID:           return "INVALID";
        case NODE_FUNCTION:          return "FUNCTION";
        case NODE_PARAMETER:         return "PARAMETER";
        case NODE_VARIABLE:          return "VARIABLE";
        case NODE_INITIALIZER_LIST:  return "INITIALIZER_LIST";
        case NODE_TYPE:              return "TYPE";
        case NODE_WHILE:             return "WHILE";
        case NODE_DO:                return "DO";
        case NODE_FOR:               return "FOR";
        case NODE_SWITCH:            return "SWITCH";
        case NODE_CASE:              return "CASE";
        case NODE_DEFAULT:           return "DEFAULT";
        case NODE_BREAK:             return "BREAK";
        case NODE_CONTINUE:          return "CONTINUE";
        case NODE_RETURN:            return "RETURN";
        case NODE_GOTO:              return "GOTO";
        case NODE_ASSIGN:            return "ASSIGN";
        case NODE_ADD_ASSIGN:        return "ADD_ASSIGN";
        case NODE_SUB_ASSIGN:        return "SUB_ASSIGN";
        case NODE_MUL_ASSIGN:        return "MUL_ASSIGN";
        case NODE_DIV_ASSIGN:        return "DIV_ASSIGN";
        case NODE_MOD_ASSIGN:        return "MOD_ASSIGN";
        case NODE_AND_ASSIGN:        return "AND_ASSIGN";
        case NODE_OR_ASSIGN:         return "OR_ASSIGN";
        case NODE_XOR_ASSIGN:        return "XOR_ASSIGN";
        case NODE_SHL_ASSIGN:        return "SHL_ASSIGN";
        case NODE_SHR_ASSIGN:        return "SHR_ASSIGN";
        case NODE_LOGICAL_OR:        return "LOGICAL_OR";
        case NODE_LOGICAL_AND:       return "LOGICAL_AND";
        case NODE_BIT_OR:            return "BIT_OR";
        case NODE_BIT_XOR:           return "BIT_XOR";
        case NODE_BIT_AND:           return "BIT_AND";
        case NODE_EQUAL:             return "EQUAL";
        case NODE_NOT_EQUAL:         return "NOT_EQUAL";
        case NODE_LESS:              return "LESS";
        case NODE_GREATER:           return "GREATER";
        case NODE_LESS_OR_EQUAL:     return "LESS_OR_EQUAL";
        case NODE_GREATER_OR_EQUAL:  return "GREATER_OR_EQUAL";
        case NODE_SHL:               return "SHL";
        case NODE_SHR:               return "SHR";
        case NODE_ADD:               return "ADD";
        case NODE_SUB:               return "SUB";
        case NODE_MUL:               return "MUL";
        case NODE_DIV:               return "DIV";
        case NODE_MOD:               return "MOD";
        case NODE_CAST:              return "CAST";
        case NODE_SIZEOF:            return "SIZEOF";
        case NODE_TYPEOF:            return "TYPEOF";
        case NODE_TYPEOF_UNQUAL:     return "TYPEOF_UNQUAL";
        case NODE_UNARY_PLUS:        return "UNARY_PLUS";
        case NODE_UNARY_MINUS:       return "UNARY_MINUS";
        case NODE_BIT_NOT:           return "BIT_NOT";
        case NODE_LOGICAL_NOT:       return "LOGICAL_NOT";
        case NODE_DEREFERENCE:       return "DEREFERENCE";
        case NODE_ADDRESS_OF:        return "ADDRESS_OF";
        case NODE_IF:                return "IF";
        case NODE_SEQUENCE:          return "SEQUENCE";
        case NODE_ARRAY_INDEX:       return "ARRAY_INDEX";
        case NODE_MEMBER_VAL:        return "MEMBER_VAL";
        case NODE_MEMBER_PTR:        return "MEMBER_PTR";
        case NODE_POST_INC:          return "POST_INC";
        case NODE_POST_DEC:          return "POST_DEC";
        case NODE_CHARACTER:         return "CHARACTER";
        case NODE_STRING:            return "STRING";
        case NODE_NUMBER:            return "NUMBER";
        case NODE_ACCESS:            return "ACCESS";
        case NODE_CALL:              return "CALL";
        default: break;
    }
    return "<unknown>";
}

node_t* node_new(node_kind_t kind) {
    node_t* node = calloc(1, sizeof(node_t));
    if (!node) {
        fatal("Out of memory.");
    }
    node->kind = kind;
    return node;
}

node_t* node_new_token(node_kind_t kind, token_t* token) {
    node_t* node = node_new(kind);
    if (token) {
        node->token = token_ref(token);
    }
    return node;
}

node_t* node_new_lexer(node_kind_t kind) {
    node_t* node = node_new(kind);
    node->token = lexer_take();
    return node;
}

void node_delete(node_t* node) {
    node_t* child = node->first_child;
    while (child) {
        node_t* next = child->right_sibling;
        node_delete(child);
        child = next;
    }

    if (node->token) {
        token_deref(node->token);
    }
    if (node->type) {
        type_deref(node->type);
    }

    switch (node->kind) {
        case NODE_FUNCTION:
            //string_deref(node->string);
            break;
        case NODE_ACCESS:
        case NODE_PARAMETER:
        case NODE_VARIABLE:
            // the symbol can be null, e.g. for unnamed parameters.
            // TODO maybe we should still create a symbol, just not add it to
            // the scope. the node type should be null and the node symbol type
            // should be the parameter type. need to fix this.
            if (node->symbol) {
                symbol_deref(node->symbol);
            }
            break;
        case NODE_MEMBER_VAL:
        case NODE_MEMBER_PTR:
            token_deref(node->member);
            break;
        default:
            break;
    }

    free(node);
}

void node_append(node_t* parent, node_t* child) {
    assert(parent);
    child->parent = parent;
    if (parent->last_child) {
        child->left_sibling = parent->last_child;
        parent->last_child->right_sibling = child;
    } else {
        parent->first_child = child;
    }
    parent->last_child = child;
}

char node_print_buffer[512];

// These are the characters used for drawing the graph. If node_print() is
// called without -dump-ast being provided, it's probably because it's being
// used for debugging so we default to unicode.

// TODO replace with hex escapes, need to add them to cci/1 so it doesn't need
// to parse any unicode characters
// https://en.wikipedia.org/wiki/Box-drawing_characters
// In the meantime we just disable unicode during bootstrapping
#ifdef __onramp_cci_opc__
    #define NODE_PRINT_UNICODE_BAR   "|"
    #define NODE_PRINT_UNICODE_CROSS "|"
    #define NODE_PRINT_UNICODE_CURVE "'"
    #define NODE_PRINT_UNICODE_DASH  "-"
#endif
#ifndef __onramp_cci_opc__
    #define NODE_PRINT_UNICODE_BAR   "│"
    #define NODE_PRINT_UNICODE_CROSS "├"
    #define NODE_PRINT_UNICODE_CURVE "└"
    #define NODE_PRINT_UNICODE_DASH  "─"
#endif

#define NODE_PRINT_ASCII_BAR   "|"
#define NODE_PRINT_ASCII_CROSS "|"
#define NODE_PRINT_ASCII_CURVE "'"
#define NODE_PRINT_ASCII_DASH  "-"

void node_print(node_t* node) {
    printf("%s", node_kind_to_string(node->kind));
    if (node->token)
        printf(" `%s`", string_cstr(node->token->value));
    if (node->type) {
        // (the type should never be null, but we want to be able to print a node on a null type assertion)
        putchar(' ');
        //putchar('`');
        type_print(node->type);
        //putchar('`');
    }
    
    switch (node->kind) {
        case NODE_VARIABLE:
            fputs(" : ", stdout);
            type_print(node->symbol->type);
            break;
        case NODE_MEMBER_PTR:
        case NODE_MEMBER_VAL:
            printf(" `%s`", string_cstr(node->member->value));
        default:
            break;
    }
}

// Prints a node and its children. The prefix (graph) to the node has already
// been printed. The given length is the length of the buffer leading up to but
// not including this node.
static void node_print_tree_impl(node_t* node, size_t length) {
    node_print(node);
    putchar('\n');

    // replace the prefix with one appropriate for our children
    // TODO this check is wrong, printing the tree only works properly on the root
    if (node->parent) {
        if (node->right_sibling) {
            const char* bar = (dump_ast == DUMP_AST_ASCII) ? NODE_PRINT_ASCII_BAR : NODE_PRINT_UNICODE_BAR;
            strcpy(node_print_buffer + length, bar);
            strcat(node_print_buffer + length, " ");
        } else {
            strcpy(node_print_buffer + length, "  ");
        }

        // stop incrementing length if we're going to overflow our buffer. this
        // means our graph will be corrupted if it's really deep (>100 nodes.)
        if (length < sizeof(node_print_buffer) - 5) {
            length += strlen(node_print_buffer + length);
        }
    }

    for (node_t* child = node->first_child; child; child = child->right_sibling) {

        // generate and output the prefix for this child
        const char* cross = (dump_ast == DUMP_AST_ASCII) ? NODE_PRINT_ASCII_CROSS : NODE_PRINT_UNICODE_CROSS;
        const char* curve = (dump_ast == DUMP_AST_ASCII) ? NODE_PRINT_ASCII_CURVE : NODE_PRINT_UNICODE_CURVE;
        const char* dash = (dump_ast == DUMP_AST_ASCII) ? NODE_PRINT_ASCII_DASH : NODE_PRINT_UNICODE_DASH;
        strcpy(node_print_buffer + length, child->right_sibling ? cross : curve);
        strcat(node_print_buffer + length, dash);
        fwrite(node_print_buffer, 1, length + strlen(node_print_buffer + length), stdout);

        node_print_tree_impl(child, length);
    }
}

void node_print_tree(node_t* node) {
    node_print_tree_impl(node, 0);
}

node_kind_t node_kind_of_binary_operator(token_t* token) {
    if (token->type != token_type_punctuation)
        return NODE_INVALID;
    const char* op = string_cstr(token->value);

    // TODO fix this to use the strings in strings.h

    switch (*op) {
        case '|':
            if (0 == strcmp(op, "||"))   return NODE_LOGICAL_OR;
            if (0 == strcmp(op, "|"))    return NODE_BIT_OR;
            break;
        case '&':
            if (0 == strcmp(op, "&&"))   return NODE_LOGICAL_AND;
            if (0 == strcmp(op, "&"))    return NODE_BIT_AND;
            break;
        case '^':
            if (0 == strcmp(op, "^"))    return NODE_BIT_XOR;
            break;
        case '=':
            if (0 == strcmp(op, "=="))   return NODE_EQUAL;
            break;
        case '!':
            if (0 == strcmp(op, "!="))   return NODE_NOT_EQUAL;
            break;
        case '<':
            if (0 == strcmp(op, "<"))    return NODE_LESS;
            if (0 == strcmp(op, "<="))   return NODE_LESS_OR_EQUAL;
            if (0 == strcmp(op, "<<"))   return NODE_SHL;
            break;
        case '>':
            if (0 == strcmp(op, ">"))    return NODE_GREATER;
            if (0 == strcmp(op, ">="))   return NODE_GREATER_OR_EQUAL;
            if (0 == strcmp(op, ">>"))   return NODE_SHR;
            break;
        case '+':
            if (0 == strcmp(op, "+"))    return NODE_ADD;
            break;
        case '-':
            if (0 == strcmp(op, "-"))    return NODE_SUB;
            break;
        case '*':
            if (0 == strcmp(op, "*"))    return NODE_MUL;
            break;
        case '/':
            if (0 == strcmp(op, "/"))    return NODE_DIV;
            break;
        case '%':
            if (0 == strcmp(op, "%"))    return NODE_MOD;
            break;
        default:
            break;
    }

    return NODE_INVALID;
}

node_kind_t node_kind_of_assignment_operator(token_t* token) {
    if (token->type != token_type_punctuation)
        return NODE_INVALID;
    const char* op = string_cstr(token->value);

    switch (*op) {
        case '=':
            if (0 == strcmp(op, "="))    return NODE_ASSIGN;
            break;
        case '+':
            if (0 == strcmp(op, "+="))   return NODE_ADD_ASSIGN;
            break;
        case '-':
            if (0 == strcmp(op, "-="))   return NODE_SUB_ASSIGN;
            break;
        case '*':
            if (0 == strcmp(op, "*="))   return NODE_MUL_ASSIGN;
            break;
        case '/':
            if (0 == strcmp(op, "/="))   return NODE_DIV_ASSIGN;
            break;
        case '%':
            if (0 == strcmp(op, "%="))   return NODE_MOD_ASSIGN;
            break;
        case '&':
            if (0 == strcmp(op, "&="))   return NODE_AND_ASSIGN;
            break;
        case '|':
            if (0 == strcmp(op, "|="))   return NODE_OR_ASSIGN;
            break;
        case '^':
            if (0 == strcmp(op, "^="))   return NODE_XOR_ASSIGN;
            break;
        case '<':
            if (0 == strcmp(op, "<<="))  return NODE_SHL_ASSIGN;
            break;
        case '>':
            if (0 == strcmp(op, ">>="))  return NODE_SHR_ASSIGN;
            break;
    }

    return NODE_INVALID;
}

node_kind_t node_kind_of_unary_operator(token_t* token) {
    const char* op = string_cstr(token->value);

    switch (*op) {
        case '+':
            if (op[1] == 0) return NODE_UNARY_PLUS;
            if (op[1] == '+' && op[2] == 0) return NODE_PRE_INC;
            break;
        case '-':
            if (op[1] == 0) return NODE_UNARY_MINUS;
            if (op[1] == '-' && op[2] == 0) return NODE_PRE_DEC;
            break;
        case '!':
            if (op[1] == 0) return NODE_LOGICAL_NOT;
            break;
        case '~':
            if (op[1] == 0) return NODE_BIT_NOT;
            break;
        case '*':
            if (op[1] == 0) return NODE_DEREFERENCE;
            break;
        case '&':
            if (op[1] == 0) return NODE_ADDRESS_OF;
            break;
        default:
            break;
    }

    return NODE_INVALID;
}

int node_kind_precedence_of_binary_operator(node_kind_t kind) {
    switch (kind) {
        case NODE_LOGICAL_OR:        return 1; // logical or
        case NODE_LOGICAL_AND:       return 2; // logical and
        case NODE_BIT_OR:            return 3; // bitwise inclusive or
        case NODE_BIT_XOR:           return 4; // bitwise exclusive or
        case NODE_BIT_AND:           return 5; // bitwise and
        case NODE_EQUAL:             return 6; // equality
        case NODE_NOT_EQUAL:         return 6; // ...
        case NODE_LESS:              return 7; // relational
        case NODE_GREATER:           return 7; // ...
        case NODE_LESS_OR_EQUAL:     return 7; // ...
        case NODE_GREATER_OR_EQUAL:  return 7; // ...
        case NODE_SHL:               return 8; // shift
        case NODE_SHR:               return 8; // ...
        case NODE_ADD:               return 9; // additive
        case NODE_SUB:               return 9; // ...
        case NODE_MUL:               return 10; // multiplicative
        case NODE_DIV:               return 10; // ...
        case NODE_MOD:               return 10; // ...
        default: break;
    }
    return -1;
}

bool node_is_location(node_t* node) {
    switch (node->kind) {
        case NODE_DEREFERENCE: // fallthrough
        case NODE_ARRAY_INDEX: // fallthrough
        case NODE_MEMBER_VAL: // fallthrough
        case NODE_MEMBER_PTR: // fallthrough
            return true;
        case NODE_ACCESS:
            // TODO symbol cannot be a function
            return true;
        default:
            break;
    }
    return false;
}

node_t* node_promote(node_t* node) {
    type_t* type = node->type;

    if (type == NULL) {
        node_print(node);
        fatal("Internal error: node is missing type!");
    }

    assert(type_is_arithmetic(type)); // this must not be called on non-arithmetic types
    if (!type_is_base(type))
        return node;

    switch (type->base) {
        case BASE_CHAR:
        case BASE_SIGNED_CHAR:
        case BASE_SIGNED_SHORT:
        case BASE_SIGNED_LONG: // note: we cast long to int here
        {
            node_t* cast = node_new(NODE_CAST);
            cast->type = type_new_base(BASE_SIGNED_INT);
            node_append(cast, node);
            return cast;
        }
        case BASE_UNSIGNED_CHAR:
        case BASE_UNSIGNED_SHORT:
        case BASE_UNSIGNED_LONG: // note: we cast long to int here
        {
            node_t* cast = node_new(NODE_CAST);
            cast->type = type_new_base(BASE_SIGNED_INT);
            node_append(cast, node);
            return cast;
        }
        default:
            break;
    }

    return node;
}

node_t* node_make_predicate(node_t* node) {
    type_t* type = node->type;

    // make sure the value is a valid predicate
    if (type_is_base(type)) {
        if (type->base == BASE_RECORD) {
            fatal_token(node->token, "Cannot use a struct or union value as a conditional expression.");
        }
    } else {
        if (type->declarator == DECLARATOR_FUNCTION) {
            fatal_token(node->token, "Cannot use a value of function type as a conditional expression.");
        }
    }

    // down-cast it to register size if necessary
    type_t* signed_int = type_new_base(BASE_SIGNED_INT);
    node = node_cast(node, signed_int, NULL);
    type_deref(signed_int);
    /*
    if (type_size(type) > 4) {
        // TODO need a cast function that makes sure casting is possible.
        // probably just call it node_cast(). then we wouldn't need to do the
        // above checks
        node_t* cast = node_new(NODE_CAST);
        cast->type = type_new_base(BASE_SIGNED_INT);
        node_append(cast, node);
        node = cast;
    }
    */

    return node;
}

size_t node_child_count(node_t* node) {
    size_t i = 0;
    for (node_t* child = node->first_child; child; child = child->right_sibling)
        ++i;
    return i;
}

static void node_check_cast_valid(type_t* type, token_t* /*nullable*/ token) {
    if (type_matches_base(type, BASE_RECORD)) {
        fatal_token(token, "Cannot cast to or from a struct or union type.");
    }
    if (type_is_declarator(type) && type->declarator == DECLARATOR_FUNCTION) {
        fatal_token(token, "Cannot cast to or from a function type. (Did you mean to cast to a function pointer type?)");
    }
}

node_t* node_cast(node_t* node, type_t* type, token_t* /*nullable*/ token) {

    // If the types are equal and the cast is implicit, we skip it. We keep
    // explicit casts in the AST for clarity. (They have no impact on codegen,
    // and hopefully should have no impact on tree optimizations.) The cast is
    // explicit if it has an associated token for the cast operator.
    if (type_equal(node->type, type) && token == NULL)
        return node;

    node_check_cast_valid(node->type, token);
    node_check_cast_valid(type, token);
    // TODO make sure the cast/conversion is legal. we do some checks above but
    // they're not complete.

    node_t* cast = node_new_token(NODE_CAST, token);
    cast->type = type_ref(type);
    node_append(cast, node);
    return cast;
}
