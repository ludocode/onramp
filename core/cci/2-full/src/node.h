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

#ifndef NODE_H_INCLUDED
#define NODE_H_INCLUDED

#include "libo-string.h"
#include "libo-vector.h"

struct type_t;
struct token_t;
struct symbol_t;

/**
 * The kind of node.
 */
typedef enum node_kind_t {
    NODE_INVALID,

    // definitions
    NODE_FUNCTION,          // The root of all functions, no parent. Children are parameters and sequence
    NODE_PARAMETER,         // Function parameter. No children.
    NODE_VARIABLE,          // Variable definition. Optional child is initializer
    NODE_INITIALIZER_LIST,  // A compound (braced) initializer. Any number of children (including none)
    NODE_TYPE,              // A type, for example as the argument to sizeof

    // statements
    NODE_WHILE,     // Two children: condition, statement
    NODE_DO,        // Two children: statement, condition
    NODE_FOR,       // Four children: initialization, condition, increment, statement
    NODE_SWITCH,    // Two children: condition, statement
    NODE_CASE,      // One child: the constant expression
    NODE_DEFAULT,   // No children
    NODE_BREAK,     // No children
    NODE_CONTINUE,  // No children
    NODE_RETURN,    // Zero or one children, optional expression
    NODE_GOTO,      // No children, value is a string label

    // assignment expressions. Two children: location, expression
    NODE_ASSIGN,
    NODE_ADD_ASSIGN,
    NODE_SUB_ASSIGN,
    NODE_MUL_ASSIGN,
    NODE_DIV_ASSIGN,
    NODE_MOD_ASSIGN,
    NODE_AND_ASSIGN,
    NODE_OR_ASSIGN,
    NODE_XOR_ASSIGN,
    NODE_SHL_ASSIGN,
    NODE_SHR_ASSIGN,

    // Other binary expressions. Two children: left, right.
    NODE_LOGICAL_OR,
    NODE_LOGICAL_AND,
    NODE_BIT_OR,
    NODE_BIT_XOR,
    NODE_BIT_AND,
    NODE_EQUAL,
    NODE_NOT_EQUAL,
    NODE_LESS,
    NODE_GREATER,
    NODE_LESS_OR_EQUAL,
    NODE_GREATER_OR_EQUAL,
    NODE_SHL,
    NODE_SHR,
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    NODE_MOD,

    // Unary expressions. One child.
    NODE_CAST,
    NODE_SIZEOF,
    NODE_TYPEOF,
    NODE_TYPEOF_UNQUAL,
    NODE_UNARY_PLUS,   // unary +
    NODE_UNARY_MINUS,  // unary -
    NODE_BIT_NOT,      // unary ~
    NODE_LOGICAL_NOT,  // unary !
    NODE_DEREFERENCE,  // unary *
    NODE_ADDRESS_OF,   // unary &
    NODE_PRE_INC,      // unary prefix ++
    NODE_PRE_DEC,      // unary prefix --

    // Postfix operators
    NODE_POST_INC,  // One child
    NODE_POST_DEC,  // One child
    NODE_ARRAY_INDEX, // Array indexing expression, two children: array, index
    NODE_MEMBER_VAL, // Member access by value (`.`), one child: record (left expression) plus member name
    NODE_MEMBER_PTR, // Member access by pointer (`->`), one child: record (left expression) plus member name

    // other expressions
    NODE_IF,        // `if` or `?`. Two or three children: condition, `if` block, optional `else` block.
    NODE_SEQUENCE,  // Compound statement, comma operator, statement expression, etc.
    NODE_CHARACTER, // No children, value is a character
    NODE_STRING,    // No children, value is a list of string fragments
    NODE_NUMBER,    // No children, value is a number
    NODE_ACCESS,    // Name of a variable or uncalled function in an expression. Children are extra fragments to concatenate.
    NODE_CALL,      // Function call. First child is function, rest are arguments.

} node_kind_t;

/**
 * A node in the abstract syntax tree.
 *
 * A node's children depend on its type. Most node types have a fixed number of
 * children. For example, an `if` node has two or three children: a conditional
 * expression, a block to run if the expression is true, and an optional block
 * to run if the expression is false (i.e. the corresponding `else` clause.)
 *
 * The children are always in parse order. For example a `while` node's
 * children are the condition followed by the block, while the `do` node's
 * children are the block followed by the expression.
 *
 * Some nodes may have unlimited children, for example blocks.
 */
typedef struct node_t {
    struct node_t* parent;
    struct node_t* left_sibling;
    struct node_t* right_sibling;
    struct node_t* first_child;
    struct node_t* last_child;

    node_kind_t kind;
    struct token_t* /*nullable*/ token;
    //string_t* operator; // Operator for certain binary or unary expressions TODO is this useful? just put it in kind?
    struct type_t* type; // The type of this expression or `void`.

    int offset; // offset of storage for value, used in code generation

    unsigned member_offset;

    union {
        // TODO this will do for now but we'll need to support several more
        // value types. e.g. we need wide and unicode characters and strings,
        // we probably need to support a gnu extension for multiple input chars
        // in a character literal, etc.

        // TODO int, long long, float. maybe we should have separate node kinds
        // for each supported input number type, e.g. int, uint, llong, ullong,
        // float, double. TODO probably would also need to differentiate int
        // from long, e.g. for _Generic

        string_t* string; // An arbitrary string value, e.g. a label name, record member name

        struct token_t* member; // The member for NODE_MEMBER_*
        struct symbol_t* symbol; // The symbol for NODE_ACCESS
        struct node_t* container; // loop/switch reference for break/continue
        int int_value; // number or character
        int string_label;
    };

    // labels for break/continue
    int body_label;
    int end_label;
} node_t;

/**
 * Creates a node without a token.
 */
node_t* node_new(node_kind_t kind);

/**
 * Creates a node with the given token.
 */
node_t* node_new_token(node_kind_t kind, struct token_t* token);

/**
 * Creates a new node that references the current token and consumes it.
 */
node_t* node_new_lexer(node_kind_t kind);

/**
 * Deletes this node and its children recursively.
 */
void node_delete(node_t* node);

/**
 * Appends the given node to the children of the given parent.
 */
void node_append(node_t* parent, node_t* child);

/**
 * Returns the number of child nodes.
 */
size_t node_child_count(node_t* node);

/**
 * Prints the given node.
 */
void node_print(node_t* node);

/**
 * Prints the tree of nodes starting at this node.
 */
void node_print_tree(node_t* node);

/**
 * Returns the node kind of the given binary operator token, or NODE_INVALID if
 * it is not a binary operator.
 *
 * Unlike cci/1, this does include && and ||.
 */
node_kind_t node_kind_of_binary_operator(struct token_t* token);

/**
 * Returns the node kind of the given assignment operator token, or
 * NODE_INVALID if it is not an assignment operator.
 */
node_kind_t node_kind_of_assignment_operator(struct token_t* token);

/**
 * Returns the node kind of the given unary operator token, or NODE_INVALID if
 * it is not a unary operator.
 */
node_kind_t node_kind_of_unary_operator(struct token_t* token);

/**
 * Returns the precedence of the given binary operator, or -1 if it is not a
 * binary operator.
 */
int node_kind_precedence_of_binary_operator(node_kind_t kind);

/**
 * Returns true if the given node is a location (i.e. it can be used as the
 * left side of an assignment expression.)
 */
bool node_is_location(node_t* node);

/**
 * Inserts a cast node to promote this arithmetic expression to register width
 * if needed.
 */
node_t* node_promote(node_t* node);

/**
 * Ensures this node is usable as a conditional expression and casts it down to
 * register size if necessary so it can be used as the predicate of a
 * conditional jump.
 */
node_t* node_make_predicate(node_t* node);

/**
 * Inserts a cast for the given node to the given type if necessary.
 */
node_t* node_cast(node_t* node, struct type_t* type, struct token_t* /*nullable*/ token);

#endif
