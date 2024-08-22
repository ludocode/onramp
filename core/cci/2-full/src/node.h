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
#include "type.h"
#include "u64.h"

struct type_t;
struct token_t;
struct symbol_t;

/**
 * The kind of node.
 */
typedef enum node_kind_t {
    NODE_INVALID,
    NODE_NOOP, // No operation. Always void, no children.

    // definitions
    NODE_FUNCTION,          // The root of all functions, no parent. Children are parameters and sequence
    NODE_PARAMETER,         // Function parameter. No children.
    NODE_VARIABLE,          // Variable definition. Optional child is initializer
    NODE_INITIALIZER_LIST,  // A compound (braced) initializer. Sparse vector of children
    NODE_TYPE,              // A type, for example as the argument to sizeof

    // statements
    NODE_WHILE,     // Two children: condition, statement
    NODE_DO,        // Two children: statement, condition
    NODE_FOR,       // Four children: initialization, condition, increment, statement
    NODE_SWITCH,    // Two children: condition, statement
    NODE_BREAK,     // No children
    NODE_CONTINUE,  // No children
    NODE_RETURN,    // Zero or one children, optional expression
    NODE_GOTO,      // No children, value is a string label

    // labels
    NODE_LABEL,     // No children, token is name
    NODE_CASE,      // Children may be constant expressions (when debugging) or no children
    NODE_DEFAULT,   // No children

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
    NODE_SIZEOF,  // one child TYPE
    NODE_TYPEOF,
    NODE_TYPEOF_UNQUAL,
    NODE_UNARY_PLUS,   // unary +
    NODE_UNARY_MINUS,  // unary -
    NODE_BIT_NOT,      // unary ~
    NODE_LOGICAL_NOT,  // unary !
    NODE_DEREFERENCE,  // unary *
    NODE_ADDRESS_OF,   // unary & or implicit function/array decay
    NODE_PRE_INC,      // unary prefix ++
    NODE_PRE_DEC,      // unary prefix --

    // Postfix operators
    NODE_POST_INC,           // One child
    NODE_POST_DEC,           // One child
    NODE_ARRAY_SUBSCRIPT,    // Two children: ptr/array, index
    NODE_MEMBER_VAL,         // Member access by value (`.`), one child: record (left expression) plus member name
    NODE_MEMBER_PTR,         // Member access by pointer (`->`), one child: record (left expression) plus member name

    // other expressions
    NODE_IF,        // `if` or `?`. Two or three children: condition, `if` block, optional `else` block.
    NODE_SEQUENCE,  // Compound statement, comma operator, statement expression, etc.
    NODE_CHARACTER, // No children, value is a character
    NODE_STRING,    // No children, value is a list of string fragments
    NODE_NUMBER,    // No children, value is a number
    NODE_ACCESS,    // Name of a variable or uncalled function in an expression. Children are extra fragments to concatenate.
    NODE_CALL,      // Function call. First child is function, rest are arguments.
    NODE_BUILTIN,   // Builtin. Children depend on which one.

} node_kind_t;

const char* node_kind_to_string(node_kind_t kind);

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
 * Some nodes may have unlimited children, for example sequences.
 *
 * Child nodes for most types are stored using the intrusive tree links, but
 * for initializers the child nodes are stored in a vector because we need O(1)
 * indexing. TODO probably we should convert all types to use a vector but
 * I haven't bothered yet. We should add some internal space in libo-vector for
 * two elements to avoid an extra allocation per node because most nodes have
 * one or two children.
 */
typedef struct node_t {
    struct node_t* parent;
    struct node_t* left_sibling;
    struct node_t* right_sibling;
    struct node_t* first_child;
    struct node_t* last_child;

    node_kind_t kind;
    struct token_t* /*nullable*/ token;
    struct token_t* /*nullable*/ end_token; // e.g. closing brace of block
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
        uint32_t u32; // 32-bit float, int or character
        u64_t u64;    // 64-bit double or long long
        int string_label; // generated name of symbol for NODE_STRING
        builtin_t builtin;
        vector_t initializers; // sparse array of non-constant nodes for initializer list

        // Contents of a case label
        struct {
            struct node_t* next_case; // linked list of case statements in a switch
            union {
                struct {
                    uint32_t start32;
                    uint32_t end32;
                };
                struct {
                    u64_t start64;
                    u64_t end64;
                };
            };
        };
    };

    // labels for break/continue, case/default, etc.
    int jump_label;
    int break_label;
    int continue_label;
} node_t;

/**
 * Creates a node without a token.
 */
node_t* node_new(node_kind_t kind);

/**
 * Creates an empty node.
 */
node_t* node_new_noop(void);

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
 * Removes the given node from its parent.
 */
void node_detach(node_t* node);

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
 *
 * If the token is NULL, this is an implicit cast (which can trigger additional
 * errors and warnings.) If non-null, it's an explicit cast.
 */
node_t* node_cast(node_t* node, struct type_t* type, struct token_t* /*nullable*/ token);

/**
 * Inserts a cast for the given node to the given base type if necessary.
 *
 * If the token is NULL, this is an implicit cast (which can trigger additional
 * errors and warnings.) If non-null, it's an explicit cast.
 */
node_t* node_cast_base(node_t* node, base_t base, struct token_t* /*nullable*/ token);

/**
 * Evaluates a constant expression of an integer up to 32 bits in size. The
 * type may be signed or unsigned, and it may be long, int, short or char.
 */
uint32_t node_eval_32(node_t* node);

/**
 * Evaluates a constant expression of a 64-bit integer size. The type may be
 * signed or unsigned long long.
 *
 * 64-bit math is fairly expensive on Onramp since it's emulated with 32-bit
 * instructions. For this reason we separate out 64-bit evaluation so we can
 * avoid it wherever possible.
 */
void node_eval_64(node_t* node, u64_t* out);

/**
 * Returns true if the given node is null or a literal zero, possibly cast to a
 * pointer.
 */
bool node_is_null(node_t* node);

/**
 * Decays array and function types to pointers.
 *
 * If this is a function, it is wrapped in an address-of node with type
 * pointer-to-function. (This is the same as the address-of operator.)
 *
 * If this is an array, it is wrapped in an address-of node with a type of
 * pointer to its first element. (This is different from the address-of
 * operator which would have type pointer-to-array.)
 *
 * Otherwise, the node is returned unchanged.
 *
 * See "Array to pointer conversion" and "Function to pointer conversion":
 *     https://en.cppreference.com/w/c/language/conversion
 */
node_t* node_decay(node_t* node);

#endif
