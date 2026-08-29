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

#include "generate.h"

#include <stdlib.h>

#include "block.h"
#include "common.h"
#include "common.h"
#include "emit.h"
#include "function.h"
#include "generate_ops.h"
#include "generate_stmt.h"
#include "instruction.h"
#include "libo-util.h"
#include "node.h"
#include "options.h"
#include "record.h"
#include "symbol.h"
#include "token.h"
#include "type.h"

//#define GENERATE_DEBUG

function_t* current_function;
block_t* current_block;

static vector_t* temporary_list;
static table_t* temporary_table;

typedef struct temporary_t {
    table_entry_t entry;
    int id;
    string_t* name;
} temporary_t;

void clear_temporaries(void) {
    table_remove_all(temporary_table);
    for (size_t i = vector_count(temporary_list); i-- != 0;) {
        temporary_t* temporary = vector_at(temporary_list, i);
        //fprintf(stderr,"DELETING TEMPORARY %p %i %s\n", (void*)temporary,temporary->id, temporary->name->bytes);
        string_deref(temporary->name);
        free(temporary);
    }
    vector_remove_all(temporary_list);
}

static temporary_t* find_temporary(const string_t* name) {
    for (table_entry_t* entry = table_bucket(temporary_table, string_hash(name));
            entry; entry = table_entry_next(entry))
    {
        temporary_t* temporary = (temporary_t*)entry;
        if (string_equal(temporary->name, name)) {
            return temporary;
        }
    }
    return NULL;
}

string_t* temporary_name(int id) {
    assert(id != -1);
    temporary_t* temporary = vector_at(temporary_list, id);
    //fprintf(stderr,"TEMPORARY %i %s\n", temporary->id, temporary->name->bytes);
    return temporary->name;
}

static void generate_location_array_subscript(node_t* node, int reg_out);
static void generate_access_location(token_t* token, symbol_t* symbol, int reg_out);
static void generate_builtin(node_t* node, int reg_out);
static void generate_builtin_location(node_t* node, int reg_out);
static void generate_initializer(node_t* variable);

void generate_setup(void) {
    temporary_list = vector_new();
    vector_reserve(temporary_list, 128);
    temporary_table = table_new();
    table_reserve_bits(temporary_table, 7); // start with 128 buckets
}

void generate_teardown(void) {
    clear_temporaries();
    table_delete(temporary_table);
    vector_delete(temporary_list);
}

/**
 * Creates a temporary.
 *
 * Takes ownership of name.
 */
temporary_t* temporary_new(string_t* name) {
    temporary_t* temporary = malloc(sizeof(temporary_t));
    temporary->name = name;
    temporary->id = vector_count(temporary_list);
    //fprintf(stderr,"CREATING TEMPORARY %p %i %s\n", (void*)temporary,temporary->id, name->bytes);
    vector_append(temporary_list, temporary);
    table_put(temporary_table, &temporary->entry, string_hash(name));
    return temporary;
}

int generate_temporary(string_t* /*nullable*/ name) {
    size_t name_length = name ? string_length(name) : 0;

    if (name) {
        // Generate preferred name as a string_t
        char* cstr = malloc(name_length + 2);
        if (!cstr) {
            fatal("Out of memory.");
        }
        cstr[0] = '%';
        memcpy(cstr + 1, name->bytes, name_length);
        cstr[name_length + 1] = 0;
        string_t* str = string_intern_cstr(cstr);
        free(cstr);

        // Try to insert with the preferred name
        if (!find_temporary(str)) {
            return temporary_new(str)->id;
        }
        string_deref(str);
    }

    // Generate numbered temporary (with name suffix if provided)
    // (There is no chance of collision with user-defined names because user
    // identifiers cannot start with decimal.)
    char* cstr = malloc(1 + 16 + 1 + name_length);
    if (!cstr) {
        fatal("Out of memory.");
    }
    cstr[0] = '%';
    char* p = itoa_d(vector_count(temporary_list), cstr + 1);
    p = cstr + strlen(cstr); // TODO itoa_d() needs to return end of buffer. once it does this line can be removed
    if (name) {
        *p++ = '_';
        memcpy(p, name->bytes, name_length);
    }
    string_t* str = string_intern_bytes(cstr, p - cstr + name_length);
    free(cstr);
    assert(!find_temporary(str));
    return temporary_new(str)->id;
}

int generate_temporary_cstr(const char* cname) {
    string_t* name = string_intern_cstr(cname);
    int temporary = generate_temporary(name);
    string_deref(name);
    return temporary;
}

static void generate_sequence(node_t* node, bool location, int reg_out) {
    assert(node->kind == NODE_SEQUENCE);
    if (node->first_child == NULL)
        return;

    bool has_defer = false;

    // Generate all but last child, ignoring the result
    for (node_t* child = node->first_child; child != node->last_child; child = child->right_sibling) {

        // Defer nodes are generated at the end
        if (child->kind == NODE_DEFER) {
            has_defer = true;
            continue;
        }

        assert(type_matches_base(child->type, BASE_VOID));
        generate_node(child, -1);
    }

    // Make sure the last child has the same type as this sequence. This
    // ensures that it's safe to generate into the output register.
    assert(type_equal(node->type, node->last_child->type));

    // If we have a defer node before the last node and it's passed indirectly,
    // we can't generate in place because we have to run defer before storing
    // the result. We need to make stack space for it.
    int reg_last = reg_out;
    // TODO I don't think this is actually necessary, we never generate directly in place
    /*
    if (has_defer && indirect) {
        reg_last = register_alloc(node->token);
        block_sub_rsp(current_block, node->token, type_size(node->type));
        block_append(current_block, node->token, MOV, reg_last, RSP);
    }
    */

    // Generate the last child. (Note that it may itself be a defer node.)
    if (node->last_child->kind == NODE_DEFER) {
        if (location) {
            fatal("Internal error: cannot generate the location of a sequence that ends in a defer node.");
        }
        generate_defer(node->last_child);
    } else if (location) {
        generate_location(node->last_child, reg_last);
    } else {
        generate_node(node->last_child, reg_last);
    }

    if (has_defer) {

        node_t* child = node->last_child;
        do {
            child = child->left_sibling;
            if (child->kind == NODE_DEFER) {
                generate_defer(child);
            }
        } while (child != node->first_child);

        // If we generated the last node into temporary stack space, copy it
        // and free the space
        // TODO as above I don't think this is necessary
        /*
        if (indirect) {
            generate_copy(node->last_child->token, node->type, 1, reg_last, reg_out);
        }
        */
    }
}

static void generate_number(node_t* node, int reg_out) {
    assert(node->kind == NODE_NUMBER);
    assert(node->first_child == NULL);

    if (type_is_long_long(node->type)) {
        u64_t* llong = &node->u64;

        // store low
        instruction_t* instruction = block_append(current_block, node->token, STW, 2);
        instruction_set_arg_number(instruction, 0, u64_low(llong));
        instruction_set_arg_temporary(instruction, 1, reg_out);

        // calc high address
        int temp = generate_temporary(NULL);
        instruction = block_append(current_block, node->token, ADD, 3);
        instruction_set_arg_temporary(instruction, 0, temp);
        instruction_set_arg_temporary(instruction, 1, reg_out);
        instruction_set_arg_number(instruction, 2, 4);

        // store high
        instruction = block_append(current_block, node->token, STW, 2);
        instruction_set_arg_number(instruction, 0, u64_high(llong));
        instruction_set_arg_temporary(instruction, 1, temp);
    } else {
        instruction_t* instruction = block_append(current_block, node->token, MOV, 2);
        instruction_set_arg_temporary(instruction, 0, reg_out);
        instruction_set_arg_number(instruction, 1, node->u32);
    }
}

static void generate_character(node_t* node, int reg_out) {
    assert(node->kind == NODE_CHARACTER);
    assert(node->first_child == NULL);
    instruction_t* instruction = block_append(current_block, node->token, MOV, 2);
    instruction_set_arg_temporary(instruction, 0, reg_out);
    instruction_set_arg_number(instruction, 1, node->u32);
}

static void generate_string(node_t* node, int reg_out) {
    assert(node->kind == NODE_STRING);
    assert(node->first_child == NULL);

    string_t* name = string_label_name(node->string_label);
    instruction_t* instruction = block_append(current_block, node->token, SYM, 2);
    instruction_set_arg_temporary(instruction, 0, reg_out);
    instruction_set_arg_absolute(instruction, 1, name);
    function_take_string(current_function, name);
}

static void generate_access_location(token_t* token, symbol_t* symbol, int reg_out) {
    bool redirection = symbol_is_global(symbol) && type_is_redirected(symbol->type);
    int reg_loc = redirection ? generate_temporary(NULL) : reg_out;

    // get the symbol address
    opcode_t opcode = symbol_is_global(symbol) ? SYM : MOV;
    instruction_t* instruction = block_append(current_block, token, opcode, 2);
    instruction_set_arg_temporary(instruction, 0, reg_loc);
    if (symbol_is_global(symbol)) {
        instruction_set_arg_absolute(instruction, 1, symbol->asm_name);
    } else {
        instruction_set_arg_temporary(instruction, 1, symbol->temporary);
    }

    // if this is a redirected global, load it
    if (redirection) {
        instruction_set_args_tt(block_append(current_block, token, LDW, 2),
                reg_out, reg_loc);
    }
}

static void generate_access(node_t* node, int reg_out) {
    symbol_t* symbol = node->symbol;
    type_t* type = symbol->type;

    if (symbol->kind == symbol_kind_function) {
        // We should be generating the location of the function, not the
        // function itself.
        fatal("Internal error: Cannot generate access to function.");
    }

    if (symbol->kind == symbol_kind_constant) {
        // TODO for now we only support enum values
        if (!type_matches_base(type, BASE_ENUM)) {
            fatal("TODO: Constants other than enum values are not yet supported.");
        }
        instruction_t* instruction = block_append(current_block, node->token, MOV, 2);
        instruction_set_arg_temporary(instruction, 0, reg_out);
        instruction_set_arg_number(instruction, 1, symbol->u32);
        return;
    }

    if (type_is_array(type)) {
        // An array access is a pointer to the first element.
        generate_access_location(node->token, node->symbol, reg_out);
        return;
    }

    int reg_loc = generate_temporary(NULL);
    generate_access_location(node->token, node->symbol, reg_loc);

    if (type_is_passed_indirectly(type)) {
        generate_copy(node->token, type, 1, reg_loc, reg_out);
        return;
    }

    opcode_t opcode;
    size_t size = type_size(type);
    if (size == 1) {
        opcode = LDB;
    } else if (size == 2) {
        opcode = LDS;
    } else if (size == 4) {
        opcode = LDW;
    } else {
        // Larger types are passed indirectly. This should have been handled
        // above.
        fatal("Internal error: generate_access() direct has impossible size");
    }

    instruction_t* instruction = block_append(current_block, node->token, opcode, 2);
    instruction_set_arg_temporary(instruction, 0, reg_out);
    instruction_set_arg_temporary(instruction, 1, reg_loc);
}

static void generate_variable(node_t* node) {
    assert(node->kind == NODE_VARIABLE);
    symbol_t* symbol = node->symbol;
    assert(symbol);

    symbol->temporary = generate_temporary(symbol->name);

    if (symbol_is_global(symbol)) {
        if (type_is_redirected(symbol->type)) {
            int addr = generate_temporary(NULL);
            instruction_t* call = block_append(current_block, node->token, CALL, 3);
            instruction_set_arg_temporary(call, 0, addr);
            instruction_set_arg_absolute_cstr(call, 1, "__malloc_bss");
            instruction_set_arg_number(call, 2, type_size(symbol->type));

            int loc = generate_temporary(NULL);
            instruction_t* instruction = block_append(current_block, node->token, SYM, 2);
            instruction_set_arg_temporary(instruction, 0, loc);
            instruction_set_arg_absolute(instruction, 1, symbol->asm_name);

            instruction_set_args_tt(
                    block_append(current_block, node->token, STW, 2),
                    addr,
                    loc);
        }
    } else {
        function_add_variable(current_function, symbol->temporary,
                symbol->type, node->token);
    }

    if (node->first_child) {
        generate_initializer(node);
    }
}

void generate_function(function_t* function) {
    node_t* root = function->root;
    emit_source_location(root->token);

    // create the entry block
    current_function = function;
    current_block = block_new(next_label++);
    function_add_block(function, current_block);

    // generate a temporary for the return value parameter (if indirect)
    if (type_is_passed_indirectly(function->root->type)) {
        function->return_temporary = generate_temporary_cstr("_Ret");
    }

    // generate a temporary for each parameter
    for (node_t* param = root->first_child;
            param != root->last_child;
            param = param->right_sibling)
    {
        assert(param->kind == NODE_PARAMETER);
        symbol_t* symbol = param->symbol;
        if (symbol) {
            symbol->temporary = generate_temporary(symbol->name);

            // Indirect parameters have an additional indirection.
            if (type_is_passed_indirectly(symbol->type)) {
                symbol->indirect_parameter_temporary = generate_temporary(symbol->name);
                instruction_set_args_tt(block_append(current_block, root->token,
                        LDW, 2), symbol->temporary, symbol->indirect_parameter_temporary);
            }
        }
    }
    if (function->type->is_variadic) {
        function->variadic_temporary = generate_temporary_cstr("_Vargs");
    }

    // generate the function contents
    generate_node(root->last_child, -1);

    // We always add a return at the end of the function in case control flow
    // falls off the end. If the function is main, we have to return 0.
    token_t* ret_token = root->first_child->end_token;
    instruction_t* ret = block_append(current_block, ret_token, RET, 1);
    if (string_equal_cstr(function->asm_name, "main")) {
        instruction_set_arg_number(ret, 0, 0);
    } else {
        instruction_set_arg_sentinel(ret, 0);
    }
}

/**
 * Generates a function call.
 *
 * The return value is placed in reg_out, or if passed indirectly, reg_out
 * contains a pointer to storage for the return value.
 */
static void generate_call(node_t* call, int reg_out) {
    node_t* function = call->first_child;
    type_t* function_type = function->type;
    if (type_is_pointer(function_type))
        function_type = function_type->ref;
    if (!type_is_function(function_type))
        fatal_token(function->token, "Internal error: cannot generate call for non-function");

    // TODO the way we attach instructions to blocks is poor at the moment; we
    // can't create an instruction until we append it. We'd like to create the
    // instruction first and append it later. In the meantime we collect the
    // args in this args array.
    bool return_indirect = type_is_passed_indirectly(call->type);
    size_t arg_count = 2 // return value and function name
            + (node_child_count(call) - 1)
            + (return_indirect ? 1 : 0)
            + (function_type->is_variadic ? 1 : 0);
    int* args = malloc(arg_count * sizeof(int));

    // If the return value is passed indirectly, storage is passed as an
    // additional argument to the function.
    int arg = 2;
    if (return_indirect) {
        args[0] = TEMPORARY_INVALID;
        args[2] = reg_out;
        ++arg;
    } else {
        args[0] = reg_out; // might be TEMPORARY_INVALID, will be treated as sentinel
    }

    // The second argument is the function to call. Call it directly if we can
    if (function->kind == NODE_ACCESS && type_is_function(function->type)) {
        args[1] = TEMPORARY_INVALID; // will be replaced with asm name
    } else {
        // We need to load it into a register.
        int temp = generate_temporary(NULL);
        if (type_is_function(function->type)) {
            generate_location(function, temp);
        } else if (type_is_pointer(function->type) && type_is_function(function->type->ref)) {
            generate_node(function, temp);
        } else {
            fatal("Internal error: call target is neither pointer nor function pointer");
        }
        args[1] = temp;
    }

    // Subsequent arguments must be generated into temporaries.
    bool passed_vararg = false;
    size_t real_arg_count = 0;
    for (node_t* node = call->first_child->right_sibling; node; node = node->right_sibling) {
        int temp_arg = generate_temporary(NULL);

        if (type_is_passed_indirectly(node->type)) {
            // We have to allocate storage to pass indirectly.
            function_add_variable(current_function, temp_arg, node->type, node->token);
        }

        generate_node(node, temp_arg);
        args[arg] = temp_arg;
        ++arg;
        ++real_arg_count;

        // For variadic functions we need to insert the "varargs" keyword.
        if (function_type->is_variadic && real_arg_count == function_type->count) {
            args[arg++] = TEMPORARY_INVALID;
            passed_vararg = true;
        }
    }

    // If we haven't inserted "varargs" yet make sure it's there.
    if (function_type->is_variadic && !passed_vararg) {
        args[arg] = TEMPORARY_INVALID;
    }

    // Finally create the call instruction and copy the args in
    instruction_t* instruction = block_append(current_block, call->token, CALL, arg_count);
    if (args[0] == TEMPORARY_INVALID) {
        instruction_set_arg_sentinel(instruction, 0);
    } else {
        instruction_set_arg_temporary(instruction, 0, args[0]);
    }
    if (args[1] == TEMPORARY_INVALID) {
        instruction_set_arg_absolute(instruction, 1, function->symbol->asm_name);
    } else {
        instruction_set_arg_temporary(instruction, 1, args[1]);
    }
    for (size_t i = 2; i < arg_count; ++i) {
        if (args[i] == TEMPORARY_INVALID) {
            instruction_set_arg_varargs(instruction, i);
        } else {
            instruction_set_arg_temporary(instruction, i, args[i]);
        }
    }
    free(args);
}

/**
 * Generates a cast between integers in a register.
 *
 * This uses the "cast base" (see cast_base() below) so we don't need to worry
 * about long, and long long doesn't fit in a register.
 *
 * Note that we don't set the upper bits in a shortening cast. For example
 * casting an int to a char emits no instructions. The sign extension occurs
 * only when necessary, for example when promoting to int for arithmetic. If
 * the result of the cast is just to store a byte in memory for example, we
 * don't care about the upper bits, so sign extension is unnecessary.
 */
void generate_int_cast(token_t* token, int reg, base_t source, base_t target) {
    if (source == target)
        return;

    instruction_t* instruction;

    switch (target) {
        case BASE_BOOL:
            if (source == BASE_SIGNED_CHAR || source == BASE_UNSIGNED_CHAR) {
                instruction = block_append(current_block, token, TRB, 2);
                instruction_set_arg_temporary(instruction, 0, reg);
                instruction_set_arg_temporary(instruction, 1, reg);
            } else if (source == BASE_SIGNED_SHORT || source == BASE_UNSIGNED_SHORT) {
                instruction = block_append(current_block, token, TRS, 2);
                instruction_set_arg_temporary(instruction, 0, reg);
                instruction_set_arg_temporary(instruction, 1, reg);
            }
            instruction = block_append(current_block, token, BOOL, 2);
            instruction_set_arg_temporary(instruction, 0, reg);
            instruction_set_arg_temporary(instruction, 1, reg);
            break;

        case BASE_SIGNED_INT:
        case BASE_UNSIGNED_INT:
            if (source == BASE_SIGNED_SHORT) {
                instruction = block_append(current_block, token, SXS, 2);
                instruction_set_arg_temporary(instruction, 0, reg);
                instruction_set_arg_temporary(instruction, 1, reg);
                break;
            }
            if (source == BASE_UNSIGNED_SHORT) {
                instruction = block_append(current_block, token, TRS, 2);
                instruction_set_arg_temporary(instruction, 0, reg);
                instruction_set_arg_temporary(instruction, 1, reg);
                break;
            }
            // fallthrough

        case BASE_SIGNED_SHORT:
        case BASE_UNSIGNED_SHORT:
            if (source == BASE_SIGNED_CHAR) {
                instruction = block_append(current_block, token, SXB, 2);
                instruction_set_arg_temporary(instruction, 0, reg);
                instruction_set_arg_temporary(instruction, 1, reg);
                break;
            }
            if (source == BASE_UNSIGNED_CHAR || source == BASE_BOOL) {
                instruction = block_append(current_block, token, TRB, 2);
                instruction_set_arg_temporary(instruction, 0, reg);
                instruction_set_arg_temporary(instruction, 1, reg);
                break;
            }
            break;

        default:
            break;
    }
}

// Returns the "cast base" for the given type, i.e. the underlying type we'll
// cast to. Pointers decay to unsigned int, enums decay to signed int, etc.
static base_t cast_base(type_t* type) {
    if (type_is_declarator(type)) {
        // Function types aren't allowed in a cast operator. All other
        // declarator types decay to pointers.
        return BASE_UNSIGNED_INT;
    }
    switch (type->base) {
        case BASE_CHAR:
            return BASE_SIGNED_CHAR;
        case BASE_ENUM:
        case BASE_SIGNED_LONG:
            return BASE_SIGNED_INT;
        case BASE_UNSIGNED_LONG:
            return BASE_UNSIGNED_INT;
        case BASE_LONG_DOUBLE:
            return BASE_DOUBLE;
        default:
            break;
    }
    return type->base;
}

static void generate_cast_indirect_to_indirect(node_t* node, type_t* source, type_t* target, int reg_out);
static void generate_cast_indirect_to_direct(node_t* node, type_t* source, type_t* target, int reg_out);
static void generate_cast_direct_to_indirect(node_t* node, type_t* source, type_t* target, int reg_out);
static void generate_cast_direct_to_direct(node_t* node, type_t* source, type_t* target, int reg_out);

void generate_cast(node_t* node, int reg_out) {
    type_t* source = node->first_child->type;
    type_t* target = node->type;

    base_t source_base = cast_base(source);
    base_t target_base = cast_base(target);
    if (source_base == target_base) {
        generate_node(node->first_child, reg_out);
        return;
    }

    bool source_indirect = type_is_passed_indirectly(source);
    bool target_indirect = type_is_passed_indirectly(target);
    if (source_indirect) {
        if (target_indirect) {
            generate_cast_indirect_to_indirect(node, source, target, reg_out);
        } else {
            generate_cast_indirect_to_direct(node, source, target, reg_out);
        }
    } else {
        if (target_indirect) {
            generate_cast_direct_to_indirect(node, source, target, reg_out);
        } else {
            generate_cast_direct_to_direct(node, source, target, reg_out);
        }
    }
}

static void generate_cast_indirect_to_indirect(node_t* node,
        type_t* source, type_t* target, int reg_out)
{
    assert(type_is_passed_indirectly(source));
    assert(type_is_passed_indirectly(target));

    // Both the source and target are indirect. Records cannot be cast
    // so the only possibility is a 64-bit value.
    assert(type_size(source) == 8);
    assert(type_size(target) == 8);

    // The register already contains a pointer to 64-bit space. We can
    // use it to generate our source, then convert to target in-place.
    generate_node(node->first_child, reg_out);

    // The only possibility are casting signed to unsigned llong or
    // vice versa; or casting signed or unsigned llong to double or
    // vice versa.

    base_t source_base = cast_base(source);
    base_t target_base = cast_base(target);

    if (source_base == BASE_DOUBLE &&
            (target_base == BASE_SIGNED_LONG_LONG || target_base == BASE_UNSIGNED_LONG_LONG)) {
        fatal("TODO cast from double to llong, emit function call");
    } else if ((source_base == BASE_SIGNED_LONG_LONG || source_base == BASE_UNSIGNED_LONG_LONG) &&
            target_base == BASE_DOUBLE) {
        fatal("TODO cast from llong to double, emit function call");
    } else {
        // casting between signed and unsigned. nothing to do.
        assert(source_base == BASE_SIGNED_LONG_LONG || source_base == BASE_UNSIGNED_LONG_LONG);
        assert(target_base == BASE_SIGNED_LONG_LONG || target_base == BASE_UNSIGNED_LONG_LONG);
    }
}

static void generate_cast_indirect_to_direct(node_t* node,
        type_t* source, type_t* target, int reg_out)
{
    assert(type_is_passed_indirectly(source));
    assert(!type_is_passed_indirectly(target));

    // The source is indirect but the target is direct. The source is
    // either a 64-bit value or a record (being cast to void), and the
    // target fits in a register.
    assert(type_size(target) <= 4);

    base_t source_base = cast_base(source);
    base_t target_base = cast_base(target);

    if (target_base == BASE_VOID) {
        // The expression result is ignored; we don't need to make stack space
        // for it.
        generate_node(node->first_child, -1);
        return;
    }

    // We need to generate the source into stack space.
    int temp_value = generate_temporary(NULL);
    function_add_variable(current_function, temp_value, source, node->token);
    generate_node(node->first_child, temp_value);

    // convert source to target
    if (source_base == BASE_DOUBLE) {
        // It's a double. We're either casting to float or to an
        // integer type.
        if (target_base == BASE_FLOAT) {
            fatal("TODO cast from double to float, emit function call");
        } else if (target_base == BASE_BOOL) {
            fatal("TODO cast from double to bool");
        } else if (type_is_signed_integer(target)) {
            fatal("TODO cast from double to signed int, emit function call");
        } else {
            assert(type_is_integer(target));
            fatal("TODO cast from double to unsigned int");
        }
    } else if (source_base == BASE_SIGNED_LONG_LONG || source_base == BASE_UNSIGNED_LONG_LONG) {
        // It's llong.
        if (target_base == BASE_FLOAT) {
            fatal("TODO cast from long long to float, emit function call");
        } else {
            assert(type_is_integer(source));
            // Cast from llong to a register-size or smaller integer

            if (target_base == BASE_BOOL) {
                // For bool we need to load both words and 'or' them together,
                // then run a 'bool' instruction on the result.

                // get low word
                int temp_low = generate_temporary(NULL);
                instruction_set_args_tt(block_append(current_block, node->token,
                        LDW, 2), temp_low, temp_value);

                // get high word
                int temp_high_addr = generate_temporary(NULL);
                instruction_set_args_ttn(block_append(current_block, node->token,
                        ADD, 3), temp_high_addr, temp_value, 4);
                int temp_high = generate_temporary(NULL);
                instruction_set_args_tt(block_append(current_block, node->token,
                        LDW, 2), temp_high, temp_high_addr);

                // 'or' the words together
                int temp_or = generate_temporary(NULL);
                instruction_set_args_ttt(block_append(current_block, node->token,
                        OR, 3), temp_or, temp_low, temp_high);

                // 'bool' the result
                instruction_set_args_tt(block_append(current_block, node->token,
                        BOOL, 2), reg_out, temp_or);
            } else {
                // Otherwise we can just load the low word.
                instruction_set_args_tt(block_append(current_block, node->token,
                        LDW, 2), reg_out, temp_value);
            }
        }
    } else {
        fatal("Internal error: unrecognized indirect to direct cast.");
    }
}

static void generate_cast_direct_to_indirect(node_t* node,
        type_t* source, type_t* target, int reg_out)
{
    assert(!type_is_passed_indirectly(source));
    assert(type_is_passed_indirectly(target));

    // The source is direct but the target is indirect. Records cannot
    // be cast so the source fits in a register and the only
    // possibility for target is a 64-bit value.
    assert(type_size(source) <= 4);
    assert(type_size(target) == 8);

    // The source fits in a temporary.
    int reg_src = generate_temporary(NULL);
    generate_node(node->first_child, reg_src);

    base_t source_base = cast_base(source);
    base_t target_base = cast_base(target);

    if (target_base == BASE_DOUBLE) {
        if (source_base == BASE_FLOAT) {
            fatal("TODO cast from float to double, emit function call");
        } else {
            assert(type_is_integer(source));
            fatal("TODO cast from register integer to double, sign-extend and emit function call");
        }
    } else {
        assert(type_is_integer(target));
        if (source_base == BASE_FLOAT) {
            fatal("TODO cast from float to long long, emit function call");
        } else {
            // Cast from register integer to long long.
            assert(type_is_integer(source));
            generate_int_cast(node->token, reg_src, source_base, BASE_UNSIGNED_INT); // set the upper bits (if necessary)

            // store the low word
            instruction_set_args_tt(block_append(current_block, node->token,
                    STW, 2), reg_src, reg_out);

            // get the high address
            int temp_high_addr = generate_temporary(NULL);
            instruction_set_args_ttn(block_append(current_block, node->token,
                    ADD, 3), temp_high_addr, reg_out, 4);

            if (type_is_signed_integer(source)) {
                // sign extension. get the sign bit
                int temp_sign = generate_temporary(NULL);
                instruction_set_args_ttn(block_append(current_block, node->token,
                        SHRU, 3), temp_sign, reg_src, 31);

                // fill the register with the sign bit
                int temp_high = generate_temporary(NULL);
                instruction_t* instruction = block_append(current_block, node->token, SUB, 3);
                instruction_set_arg_temporary(instruction, 0, temp_high);
                instruction_set_arg_number(instruction, 1, 0);
                instruction_set_arg_temporary(instruction, 2, temp_sign);

                // store the high word
                instruction_set_args_tt(block_append(current_block, node->token,
                        STW, 2), temp_high, temp_high_addr);
            } else {
                // clear the high word
                instruction_t* instruction = block_append(current_block, node->token, STW, 2);
                instruction_set_arg_number(instruction, 0, 0);
                instruction_set_arg_temporary(instruction, 1, temp_high_addr);
            }
        }
    }
}

static void generate_cast_direct_to_direct(node_t* node,
        type_t* source, type_t* target, int reg_out)
{
    assert(!type_is_passed_indirectly(source));
    assert(!type_is_passed_indirectly(target));

    // The to and from types both fit in registers. We can use the same
    // register for both and convert in place.
    generate_node(node->first_child, reg_out);

    base_t source_base = cast_base(source);
    base_t target_base = cast_base(target);

    if (target_base == BASE_FLOAT) {
        if (type_is_signed_integer(source)) {
            generate_int_cast(node->token, reg_out, source_base, BASE_SIGNED_INT);
            fatal("TODO cast signed integer to float");
        } else {
            generate_int_cast(node->token, reg_out, source_base, BASE_UNSIGNED_INT);
            fatal("TODO cast unsigned integer to float");
        }
    } else if (source_base == BASE_FLOAT) {
        if (type_is_signed_integer(target)) {
            fatal("TODO cast float to signed integer");
        } else {
            fatal("TODO cast float to unsigned integer");
        }
    } else {
        generate_int_cast(node->token, reg_out, source_base, target_base);
    }
}

static void generate_initializer_scalar(node_t* expr, type_t* target, int reg_base, size_t offset) {

    if (type_is_array(target)) {
        if (expr->kind != NODE_STRING) {
            fatal("Internal error: Cannot initialize an array with a non-string scalar");
        }
        int reg_val = generate_temporary(NULL);
        generate_node(expr, reg_val);

        // TODO need to handle wide string arrays, currently we assume char
        size_t array_count = target->count;
        size_t string_count = expr->type->count;

        // Copy bytes from the string to fill the array
        size_t copy_count = array_count < string_count ? array_count : string_count;
        int reg_loc = generate_temporary(NULL);
        instruction_t* add = block_append(current_block, expr->token, ADD, 3);
        instruction_set_arg_temporary(add, 0, reg_loc);
        instruction_set_arg_temporary(add, 1, reg_base);
        instruction_set_arg_number(add, 2, offset);
        generate_copy(expr->token, target->ref, copy_count, reg_val, reg_loc);

        // If we're initializing a char array with too short a string, we need
        // to zero out the rest of the array.
        if (array_count > string_count) {
            instruction_t* add = block_append(current_block, expr->token, ADD, 3);
            instruction_set_arg_temporary(add, 0, reg_loc);
            instruction_set_arg_temporary(add, 1, reg_loc);
            instruction_set_arg_number(add, 2, string_count);
            generate_zero_array(expr->token, target->ref, array_count - string_count, reg_loc);
        }

    } else if (type_is_passed_indirectly(target)) {
        if (offset == 0) {
            generate_node(expr, reg_base);
        } else {
            int reg_loc = generate_temporary(NULL);
            instruction_t* add = block_append(current_block, expr->token, ADD, 3);
            instruction_set_arg_temporary(add, 0, reg_loc);
            instruction_set_arg_temporary(add, 1, reg_base);
            instruction_set_arg_number(add, 2, offset);
            generate_node(expr, reg_loc);
        }

    } else {
        int reg_val = generate_temporary(NULL);
        generate_node(expr, reg_val);
        generate_store_offset(expr->token, target, reg_val, reg_base, offset);
    }
}

/*
 * Generates an initializer list.
 *
 * The address of the variable being initialized is (reg_base + offset).
 */
static void generate_initializer_list(node_t* list, type_t* type, int reg_base, size_t base_offset) {
    size_t i;
    for (i = 0; i < vector_count(&list->children); ++i) {

        // Get the type of the child we're initializing
        type_t* child_type;
        size_t offset;
        if (type_matches_base(type, BASE_RECORD)) {
            member_t* member = vector_at(&type->record->member_list, i);
            child_type = member->type;
            offset = base_offset + member->offset;
        } else if (type_is_array(type)) {
            child_type = type->ref;
            offset = base_offset + i * type_size(child_type);
        } else {
            child_type = type;
            offset = base_offset;
        }

        node_t* child = vector_at(&list->children, i);
        if (!child) {
            generate_zero_scalar(list->token, type, reg_base, offset);
        } else if (child->kind == NODE_INITIALIZER_LIST) {
            generate_initializer_list(child, child_type, reg_base, offset);
        } else {
            generate_initializer_scalar(child, child_type, reg_base, offset);
        }
    }

    // Zero out the rest of the array
    if (type_is_array(type) && i < type->count) {
        int reg_loc = generate_temporary(NULL);
        instruction_t* add = block_append(current_block, list->token, ADD, 3);
        instruction_set_arg_temporary(add, 0, reg_loc);
        instruction_set_arg_temporary(add, 1, reg_base);
        instruction_set_arg_number(add, 2, base_offset + i * type_size(type->ref));

        generate_zero_array(list->token, type->ref, type->count - i, reg_loc);

    // Zero out the rest of the struct
    } else if (type_matches_base(type, BASE_RECORD) && type->record->is_struct) {
        for (; i < record_member_count(type->record); ++i) {
            member_t* member = vector_at(&type->record->member_list, i);
            size_t offset = base_offset + member->offset;
            generate_zero_scalar(list->token, member->type, reg_base, offset);
        }
    }
}

/*
 * Generates an initializer for the given VARIABLE node.
 *
 * Variable declarations are never expressions (not even if they are the last
 * statement of a statement expression) so we don't need a temporary in which
 * to return a value.
 */
static void generate_initializer(node_t* variable) {
    int reg_loc = generate_temporary(NULL);
    generate_access_location(variable->token, variable->symbol, reg_loc);

    node_t* initializer = variable->first_child;
    if (initializer->kind == NODE_INITIALIZER_LIST) {
        generate_initializer_list(variable->first_child, variable->symbol->type, reg_loc, 0);
    } else {
        type_t* init_type = variable->first_child->type;
        type_t* var_type = variable->symbol->type;

        if (type_is_array(init_type)) {
            // Array lengths don't have to match in initializers. The string
            // will be truncated or padded with zeroes.
            assert(type_is_array(var_type));
        } else {
            assert(type_equal(init_type, var_type));
        }

        generate_initializer_scalar(variable->first_child, var_type, reg_loc, 0);
    }
}

/**
 * Given a pointer and member offset, performs a dereference operation. This
 * includes unary `*`, binary `.`, binary `->`, and array indexing (after
 * addition.)
 *
 * The pointer in reg_ptr is shifted by the given member offset, then loaded
 * into reg_out.
 */
void generate_dereference_impl(node_t* node, int reg_out, int reg_ptr, int offset) {

    // TODO the shift and load could be done together if small enough. could optimize this later

    // shift the pointer by the member offset
    if (offset != 0) {
        instruction_t* instruction = block_append(current_block, node->token, ADD, 3);
        instruction_set_arg_temporary(instruction, 0, reg_ptr);
        instruction_set_arg_temporary(instruction, 1, reg_ptr);
        instruction_set_arg_number(instruction, 2, offset);
    }

    // if this is an array, the pointer to it is already in the source
    // register, so we just need to move it to the destination.
    if (type_is_array(node->type)) {
        instruction_t* instruction = block_append(current_block, node->token, MOV, 2);
        instruction_set_arg_temporary(instruction, 0, reg_out);
        instruction_set_arg_temporary(instruction, 1, reg_ptr);
        return;
    }

    if (type_is_passed_indirectly(node->type)) {
        generate_copy(node->token, node->type, 1, reg_ptr, reg_out);
        return;
    }

    // TODO should move this to a block_append helper
    size_t size = type_size(node->type);
    opcode_t opcode;
    if (size == 1) {
        opcode = LDB;
    } else if (size == 2) {
        opcode = LDS;
    } else if (size == 4) {
        opcode = LDW;
    } else {
        fatal_token(node->token, "Internal error: unknown size cannot be dereferenced in a register");
    }
    instruction_set_args_tt(block_append(current_block, node->token,
                opcode, 2), reg_out, reg_ptr);
}

static void generate_dereference(node_t* node, int reg_out) {
    assert(reg_out != -1);
    int reg_loc = generate_temporary(NULL);
    generate_node(node->first_child, reg_loc);
    generate_dereference_impl(node, reg_out, reg_loc, 0);
}

static void generate_array_subscript(node_t* node, int reg_out) {
    assert(reg_out != -1);
    int reg_loc = generate_temporary(NULL);
    generate_location_array_subscript(node, reg_loc);
    generate_dereference_impl(node, reg_out, reg_loc, 0);
}

static void generate_member_val(node_t* node, int reg_out) {
    assert(reg_out != -1);
    int reg_loc = generate_temporary(NULL);
    generate_location(node->first_child, reg_loc);
    generate_dereference_impl(node, reg_out, reg_loc, node->member_offset);
}

static void generate_member_ptr(node_t* node, int reg_out) {
    assert(reg_out != -1);
    int reg_loc = generate_temporary(NULL);
    generate_node(node->first_child, reg_loc);
    generate_dereference_impl(node, reg_out, reg_loc, node->member_offset);
}

static void generate_location_member_val(node_t* node, int reg_out) {
    generate_location(node->first_child, reg_out);
    instruction_t* add = block_append(current_block, node->token, ADD, 3);
    instruction_set_arg_temporary(add, 0, reg_out);
    instruction_set_arg_temporary(add, 1, reg_out);
    instruction_set_arg_number(add, 2, node->member_offset);
}

static void generate_location_member_ptr(node_t* node, int reg_out) {
    generate_node(node->first_child, reg_out);
    instruction_t* add = block_append(current_block, node->token, ADD, 3);
    instruction_set_arg_temporary(add, 0, reg_out);
    instruction_set_arg_temporary(add, 1, reg_out);
    instruction_set_arg_number(add, 2, node->member_offset);
}

static void generate_location_array_subscript(node_t* node, int reg_out) {
    generate_indirection_add_sub(node, reg_out);
}

static void generate_sizeof(node_t* node, int reg_out) {
    unsigned size = type_size(node->first_child->type);
    instruction_t* instruction = block_append(current_block, node->token, MOV, 2);
    instruction_set_arg_temporary(instruction, 0, reg_out);
    instruction_set_arg_number(instruction, 1, size);
}

static void generate_address_of(node_t* node, int reg_out) {
    generate_location(node->first_child, reg_out);
}

#ifdef GENERATE_DEBUG
int debug_depth;
#endif

void generate_defer(node_t* node) {
    #ifdef GENERATE_DEBUG
    for (int i = 0; i < debug_depth; ++i)
        fputs("  ", stdout);
    printf("%s() %s\n", __func__, node_kind_to_string(node->kind));
    ++debug_depth;
    #endif

    assert(node->kind == NODE_DEFER);

    // defer always has one child node which is a sequence of void type.
    assert(node->first_child != NULL);
    assert(node->first_child == node->last_child);
    assert(node->first_child->kind == NODE_SEQUENCE);
    assert(type_matches_base(node->first_child->type, BASE_VOID));

    generate_node(node->first_child, -1);

    #ifdef GENERATE_DEBUG
    --debug_depth;
    #endif
}

void generate_exit_defers(node_t* node, node_t* container) {
    if (node == container) {
        return;
    }

    // The node should be a jump. It doesn't make sense for it to be a DEFER,
    // or a SEQUENCE or almost anything else.
    assert(node->kind != NODE_DEFER);
    assert(node->kind != NODE_SEQUENCE);

    // We walk up from the node to the given container checking SEQUENCE nodes
    // for DEFER nodes.
    node_t* parent = node;
    do {
        node_t* child = parent;
        parent = child->parent;
        if (parent == NULL) {
            fatal_token(node->token, "Internal error: generating defers, container is not parent of node");
        }
        if (parent->kind == NODE_DEFER) {
            fatal_token(node->token, "Cannot jump out of a `defer` block.");
        }
        if (parent->kind == NODE_SEQUENCE) {
            while (child != parent->first_child) {
                child = child->left_sibling;
                if (child->kind == NODE_DEFER) {
                    generate_defer(child);
                }
            }
        }
    } while (parent != container);
}

void generate_diagnose_defers(node_t* node, node_t* container, token_t* error_token) {
    if (node == container) {
        return;
    }

    // The node should be a case, default or label. It doesn't make sense for
    // it to be a DEFER, or a SEQUENCE or almost anything else.
    assert(node->kind != NODE_DEFER);
    assert(node->kind != NODE_SEQUENCE);

    node_t* parent = node;
    do {
        node_t* child = parent;
        parent = child->parent;
        if (parent == NULL) {
            fatal_token(error_token, "Internal error: diagnosing defers, container is not parent of node");
        }
        if (parent->kind == NODE_DEFER) {
            fatal_token(error_token, "Cannot jump into a `defer` block.");
        }
        if (parent->kind == NODE_SEQUENCE) {
            while (child != parent->first_child) {
                child = child->left_sibling;
                if (child->kind == NODE_DEFER) {
                    fatal_token(error_token, "Cannot jump forward across a `defer` statement.");
                }
            }
        }
    } while (parent != container);
}

void generate_node(node_t* node, int reg_out_opt) {
    #ifdef GENERATE_DEBUG
    for (int i = 0; i < debug_depth; ++i)
        fputs("  ", stdout);
    printf("%s() %s %x\n", __func__, node_kind_to_string(node->kind), reg_out_opt);
    ++debug_depth;
    #endif

    // If the return value is ignored, we can avoid storing it in a temporary.
    // This is particularly important for passing large structures by value so
    // we don't overflow the stack; see test `expr/struct-assign-large.c`. For
    // most nodes we really only need to evaluate the children for side
    // effects.
    int reg_out = reg_out_opt;
    if (reg_out == TEMPORARY_INVALID || type_matches_base(node->type, BASE_VOID)) {

        switch (node->kind) {
            case NODE_INVALID:
                fatal("Internal error: cannot generate unrecognized node.");
                break;

            case NODE_ACCESS:
            case NODE_CHARACTER:
            case NODE_STRING:
            case NODE_NUMBER:
                // nothing to do.
                #ifdef GENERATE_DEBUG
                --debug_depth;
                #endif
                return;

            // These node types accept an optional register.
            case NODE_ASSIGN:
                generate_assign(node, TEMPORARY_INVALID);
                #ifdef GENERATE_DEBUG
                --debug_depth;
                #endif
                return;
            case NODE_SEQUENCE:
                generate_sequence(node, false, TEMPORARY_INVALID);
                #ifdef GENERATE_DEBUG
                --debug_depth;
                #endif
                return;

            // For these node types, we only need to generate the children for
            // side effects. We don't actually need to perform the operation
            // because we're not using the result.
            case NODE_LOGICAL_OR:
            case NODE_LOGICAL_AND:
            case NODE_BIT_OR:
            case NODE_BIT_XOR:
            case NODE_BIT_AND:
            case NODE_EQUAL:
            case NODE_NOT_EQUAL:
            case NODE_LESS:
            case NODE_GREATER:
            case NODE_LESS_OR_EQUAL:
            case NODE_GREATER_OR_EQUAL:
            case NODE_SHL:
            case NODE_SHR:
            case NODE_ADD:
            case NODE_SUB:
            case NODE_MUL:
            case NODE_DIV:
            case NODE_MOD:
            case NODE_CAST:
            case NODE_SIZEOF:
            case NODE_UNARY_PLUS:
            case NODE_UNARY_MINUS:
            case NODE_BIT_NOT:
            case NODE_LOGICAL_NOT:
            case NODE_DEREFERENCE:
            case NODE_ADDRESS_OF:
            case NODE_ARRAY_SUBSCRIPT:
            case NODE_MEMBER_VAL:
            case NODE_MEMBER_PTR:
                for (node_t* child = node->first_child; child; child = child->right_sibling) {
                    generate_node(child, -1);
                }
                #ifdef GENERATE_DEBUG
                --debug_depth;
                #endif
                return;

            case NODE_CALL:
                if (!type_is_passed_indirectly(node->type)) {
                    generate_call(node, -1);
                    return;
                }

            // For any case not handled above, we will need a temporary for the
            // return value, and if it is indirect, we will have to create
            // stack space to store it.
            default:
                break;
        }

        if (reg_out == -1) {
            // Allocate space to store the result.
            reg_out = generate_temporary(NULL);
            if (type_is_passed_indirectly(node->type)) {
                function_add_variable(current_function, reg_out, node->type, node->token);
            }
        }
    }

    switch (node->kind) {
        case NODE_INVALID:
            fatal("Internal error: cannot generate unrecognized node.");
            break;
        case NODE_NOOP:
            // nothing
            break;

        // These nodes are not expressions. They are handled separately, not
        // generated through generate_node()
        case NODE_FUNCTION:
            fatal("Internal error: cannot generate arbitrary FUNCTION node.");
        case NODE_PARAMETER:
            fatal("Internal error: cannot generate arbitrary PARAMETER node.");
        case NODE_INITIALIZER_LIST:
            fatal("Internal error: cannot generate arbitrary INITIALIZER_LIST node.");
        case NODE_TYPE:
            fatal("Internal error: cannot generate arbitrary TYPE node.");
        case NODE_DEFER:
            fatal("Internal error: cannot generate arbitrary DEFER node.");

        case NODE_VARIABLE:
            generate_variable(node);
            break;

        // statements
        case NODE_WHILE: generate_while(node, reg_out); break;
        case NODE_DO: generate_do(node, reg_out); break;
        case NODE_FOR: generate_for(node, reg_out); break;
        case NODE_BREAK: generate_break(node, reg_out); break;
        case NODE_CONTINUE: generate_continue(node, reg_out); break;
        case NODE_RETURN: generate_return(node, reg_out); break;
        case NODE_SWITCH: generate_switch(node, reg_out); break;
        case NODE_GOTO: generate_goto(node, reg_out); break;

        // labels
        case NODE_LABEL: generate_label(node, reg_out); break;
        case NODE_CASE: // fallthrough
        case NODE_DEFAULT: generate_case_or_default(node, reg_out); break;

        // assignment expressions
        case NODE_ASSIGN: generate_assign(node, reg_out); break;
        case NODE_ADD_ASSIGN: generate_add_assign(node, reg_out); break;
        case NODE_SUB_ASSIGN: generate_sub_assign(node, reg_out); break;
        case NODE_MUL_ASSIGN: generate_mul_assign(node, reg_out); break;
        case NODE_DIV_ASSIGN: generate_div_assign(node, reg_out); break;
        case NODE_MOD_ASSIGN: generate_mod_assign(node, reg_out); break;
        case NODE_AND_ASSIGN: generate_and_assign(node, reg_out); break;
        case NODE_OR_ASSIGN: generate_or_assign(node, reg_out); break;
        case NODE_XOR_ASSIGN: generate_xor_assign(node, reg_out); break;
        case NODE_SHL_ASSIGN: generate_shl_assign(node, reg_out); break;
        case NODE_SHR_ASSIGN: generate_shr_assign(node, reg_out); break;

        // other binary expressions
        case NODE_LOGICAL_OR: generate_logical_or(node, reg_out); break;
        case NODE_LOGICAL_AND: generate_logical_and(node, reg_out); break;
        case NODE_BIT_OR: generate_bit_or(node, reg_out); break;
        case NODE_BIT_XOR: generate_bit_xor(node, reg_out); break;
        case NODE_BIT_AND: generate_bit_and(node, reg_out); break;
        case NODE_EQUAL: generate_equal(node, reg_out); break;
        case NODE_NOT_EQUAL: generate_not_equal(node, reg_out); break;
        case NODE_LESS: generate_less(node, reg_out); break;
        case NODE_GREATER: generate_greater(node, reg_out); break;
        case NODE_LESS_OR_EQUAL: generate_less_or_equal(node, reg_out); break;
        case NODE_GREATER_OR_EQUAL: generate_greater_or_equal(node, reg_out); break;
        case NODE_SHL: generate_shl(node, reg_out); break;
        case NODE_SHR: generate_shr(node, reg_out); break;
        case NODE_ADD: generate_add(node, reg_out); break;
        case NODE_SUB: generate_sub(node, reg_out); break;
        case NODE_MUL: generate_mul(node, reg_out); break;
        case NODE_DIV: generate_div(node, reg_out); break;
        case NODE_MOD: generate_mod(node, reg_out); break;

        // unary expressions
        case NODE_CAST: generate_cast(node, reg_out); break;
        case NODE_SIZEOF: generate_sizeof(node, reg_out); break;
        case NODE_TYPEOF: fatal_token(node->token, "TODO generate TYPEOF");
        case NODE_TYPEOF_UNQUAL: fatal_token(node->token, "TODO generate TYPEOF_UNQUAL");
        case NODE_UNARY_PLUS: generate_unary_plus(node, reg_out); break;
        case NODE_UNARY_MINUS: generate_unary_minus(node, reg_out); break;
        case NODE_BIT_NOT: generate_bit_not(node, reg_out); break;
        case NODE_LOGICAL_NOT: generate_logical_not(node, reg_out); break;
        case NODE_DEREFERENCE: generate_dereference(node, reg_out); break;
        case NODE_ADDRESS_OF: generate_address_of(node, reg_out); break;
        case NODE_PRE_INC: generate_pre_inc(node, reg_out); break;
        case NODE_PRE_DEC: generate_pre_dec(node, reg_out); break;

        // postfix operators
        case NODE_POST_INC: generate_post_inc(node, reg_out); break;
        case NODE_POST_DEC: generate_post_dec(node, reg_out); break;
        case NODE_ARRAY_SUBSCRIPT: generate_array_subscript(node, reg_out); break;
        case NODE_MEMBER_VAL: generate_member_val(node, reg_out); break;
        case NODE_MEMBER_PTR: generate_member_ptr(node, reg_out); break;

        // other expressions
        case NODE_IF: generate_if(node, reg_out); break;
        case NODE_SEQUENCE: generate_sequence(node, false, reg_out); break;
        case NODE_CHARACTER: generate_character(node, reg_out); break;
        case NODE_STRING: generate_string(node, reg_out); break;
        case NODE_NUMBER: generate_number(node, reg_out); break;
        case NODE_ACCESS: generate_access(node, reg_out); break;
        case NODE_CALL: generate_call(node, reg_out); break;
        case NODE_BUILTIN: generate_builtin(node, reg_out); break;
    }

    #ifdef GENERATE_DEBUG
    --debug_depth;
    #endif
}

void generate_location(node_t* node, int reg_out) {
    #ifdef GENERATE_DEBUG
    for (int i = 0; i < debug_depth; ++i)
        fputs("  ", stdout);
    printf("%s() %s %x\n", __func__, node_kind_to_string(node->kind), reg_out);
    ++debug_depth;
    #endif

    switch (node->kind) {
        case NODE_ACCESS: generate_access_location(node->token, node->symbol, reg_out); break;
        case NODE_DEREFERENCE: generate_node(node->first_child, reg_out); break;
        case NODE_MEMBER_VAL: generate_location_member_val(node, reg_out); break;
        case NODE_MEMBER_PTR: generate_location_member_ptr(node, reg_out); break;
        case NODE_ARRAY_SUBSCRIPT: generate_location_array_subscript(node, reg_out); break;
        case NODE_BUILTIN: generate_builtin_location(node, reg_out); break;
        case NODE_STRING: generate_string(node, reg_out); break;
        case NODE_SEQUENCE: generate_sequence(node, true, reg_out); break;
        case NODE_CAST:
            // We can generate the location of a struct or union cast for the
            // purpose of the member-of operator.
            if (type_matches_base(node->type, BASE_RECORD)) {
                generate_location(node->first_child, reg_out);
                break;
            }
            fatal("Internal error, cannot generate location of non-struct cast.");
            break;
        default:
            fatal("Internal error, cannot generate location of non-value node: %s.", node_kind_to_string(node->kind));
            break;
    }

    #ifdef GENERATE_DEBUG
    --debug_depth;
    #endif
}

/**
 * Generates an initializer for a variable with static storage duration (i.e. a
 * global or static local variable.)
 *
 * We emit a static function definition with a unique name and a constructor
 * of priority 50. This runs before main(), and the minimum priority for
 * __attribute__((constructor(<priority>))) is 101 so this also runs before any
 * user constructor functions in GNU C. The variable is therefore initialized
 * before any user C code.
 *
 * The initializer is null if there is no user-specified initializer;
 * redirected variables still need to be allocated.
 */
static void generate_initializer_static_storage(
        struct symbol_t* varsym, struct node_t* /*nullable*/ initializer)
{
    // Generate a name for the function. It's static and we use a unique label
    // for it so the rest of the name doesn't matter; we just append some
    // characters from the name for debugging.
    string_t* name_str;
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s%x_%s", INITIALIZER_LABEL_PREFIX,
                next_label++, varsym->name->bytes);
        buf[sizeof(buf) - 1] = 0;
        name_str = string_intern_cstr(buf);
    }
    token_t* name = token_new_at(name_str,
            initializer ? initializer->token : varsym->token);

    // Create the root node
    type_t* void_t = type_new_base(BASE_VOID);
    type_t* func_t = type_new_function(type_ref(void_t), NULL, NULL, 0, false); // TODO type_ref should not be necessary here
    node_t* root = node_new(NODE_FUNCTION);
    root->type = type_ref(void_t); // return value

    // Create the function
    function_t* function = function_new(func_t, name, name_str, root);
    function->symbol = symbol_new(symbol_kind_function, func_t, name, name_str);
    function->symbol->linkage = symbol_linkage_internal;
    function->symbol->is_constructor = true;
    function->symbol->constructor_priority = 50;

    // Add a node for the variable
    // (This will cause it to be allocated if it is redirected)
    node_t* variable = node_new(NODE_VARIABLE);
    variable->symbol = symbol_ref(varsym);
    variable->type = type_ref(void_t);
    node_append(root, variable);

    // Attach the initializer if it exists
    if (initializer) {
        node_append(variable, initializer);
    }

    // Push the current function (in case we're compiling a local static
    // variable)
    function_t* old_function = current_function;
    current_function = function;

    if (dump_ast) {
        putchar('\n');
        node_print_tree(root);
        putchar('\n');
    }

    // Generate it
    generate_function(function);
    emit_function(function);

    // Clean up
    current_function = old_function;
    symbol_deref(function->symbol);
    function_delete(function);
    type_deref(func_t);
    type_deref(void_t);
    string_deref(name_str);
    token_deref(name);
    clear_temporaries();
}

void generate_variable_static_storage(struct symbol_t* symbol, struct node_t* /*nullable*/ initializer) {

    // TODO if this is a tentative definition and -fcommon is specified, we should emit weak.

    emit_source_location(symbol->token);
    emit_char(symbol->linkage == symbol_linkage_internal ? '@' : '=');
    emit_string(symbol->asm_name);
    emit_newline();

    bool redirected = type_is_redirected(symbol->type);
    if (redirected) {
        // We only emit storage for a pointer. The variable is allocated
        // separately.
        emit_cstr(ASM_INDENT);
        emit_char('0');
    } else {
        for (size_t count = (type_size(symbol->type) + 3) >> 2; count-- > 0;) {
            if ((count & 15) == 15) {
                emit_newline();
                emit_cstr(ASM_INDENT);
            } else {
                emit_char(' ');
            }
            emit_char('0');
        }
    }
    emit_newline();

    if (initializer || redirected) {
        emit_newline();
        generate_initializer_static_storage(symbol, initializer);
    }
    emit_global_divider();
}

static void generate_builtin_va_arg(node_t* builtin, int reg_out) {

    // get the address of the variable containing the va_list
    int temp_var = generate_temporary(NULL);
    generate_location(builtin->first_child, temp_var);

    // get the value of the va_list
    int temp_ptr = generate_temporary(NULL);
    instruction_set_args_tt(block_append(current_block, builtin->token,
                LDW, 2), temp_ptr, temp_var);

    // increment the va_list
    int temp_ptr_updated = generate_temporary(NULL);
    instruction_set_args_ttn(block_append(current_block, builtin->token,
                ADD, 3), temp_ptr_updated, temp_ptr, 4);
    instruction_set_args_tt(block_append(current_block, builtin->token,
                STW, 2), temp_ptr_updated, temp_var);

    // get the real pointer to the value
    int temp_value_ptr;
    if (type_is_passed_indirectly(builtin->type)) {
        // types passed indirectly need an additional load (the argument is a
        // pointer to the value.)
        temp_value_ptr = generate_temporary(NULL);
        instruction_set_args_tt(block_append(current_block, builtin->token,
                    LDW, 2), temp_value_ptr, temp_ptr);
    } else {
        temp_value_ptr = temp_ptr;
    }

    // load the value into the output
    generate_dereference_impl(builtin, reg_out, temp_value_ptr, 0);

}

static void generate_builtin_va_start(node_t* builtin, int reg_out) {
    assert(reg_out != -1);
    generate_location(builtin->first_child, reg_out);
    generate_store(builtin->token, builtin->first_child->type, current_function->variadic_temporary, reg_out);
}

static void generate_builtin_va_end(node_t* builtin, int reg_out) {
    // nothing
}

static void generate_builtin_va_copy(node_t* builtin, int reg_out) {
    assert(reg_out != -1);
    generate_location(builtin->first_child, reg_out);
    int reg_val = generate_temporary(NULL);
    generate_node(builtin->last_child, reg_val);
    generate_store(builtin->token, builtin->first_child->type, reg_val, reg_out);
}

static void generate_builtin_func(node_t* builtin, int reg_out) {
    generate_node(builtin->first_child, reg_out);
}

static void generate_builtin(node_t* node, int reg_out) {
    switch (node->builtin) {
        case BUILTIN_VA_ARG: generate_builtin_va_arg(node, reg_out); return;
        case BUILTIN_VA_START: generate_builtin_va_start(node, reg_out); return;
        case BUILTIN_VA_END: generate_builtin_va_end(node, reg_out); return;
        case BUILTIN_VA_COPY: generate_builtin_va_copy(node, reg_out); return;
        case BUILTIN_FUNC: generate_builtin_func(node, reg_out); return;
    }

    fatal("Internal error: cannot generate unrecognized builtin.");
}

static void generate_builtin_location(node_t* node, int reg_out) {
    switch (node->builtin) {
        case BUILTIN_FUNC: generate_builtin_func(node, reg_out); return;
        default: break;
    }
    fatal("Internal error: cannot generate location of this builtin.");
}
