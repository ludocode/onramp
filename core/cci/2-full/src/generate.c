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

#include "common.h"
#include "record.h"
#include "node.h"
#include "type.h"
#include "symbol.h"
#include "emit.h"
#include "function.h"
#include "block.h"
#include "instruction.h"
#include "common.h"
#include "options.h"
#include "generate_ops.h"
#include "generate_stmt.h"
#include "token.h"

//#define GENERATE_DEBUG

function_t* current_function;
block_t* current_block;
int next_label;

#ifdef CCI2_IR
static vector_t* temporary_list;
static table_t* temporary_table;

typedef struct temporary_t {
    table_entry_t entry;
    int id;
    string_t* name;
} temporary_t;

static void clear_temporaries(void) {
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
    temporary_t* temporary = vector_at(temporary_list, id);
    //fprintf(stderr,"TEMPORARY %i %s\n", temporary->id, temporary->name->bytes);
    return temporary->name;
}
#endif

#ifndef CCI2_IR
int register_next;       // next register to allocate
int register_loop_count; // number of times we've looped back to r0 while allocating registers
#endif

static void generate_location_array_subscript(node_t* node, int reg_out);
static void generate_access_location(token_t* token, symbol_t* symbol, int reg_out);
static void generate_builtin(node_t* node, int reg_out);
static void generate_builtin_location(node_t* node, int reg_out);
//static void generate_initializer(node_t* variable, int reg_loc);

void generate_setup(void) {
    #ifndef CCI2_IR
    register_next = R0;
    #endif

    #ifdef CCI2_IR
    temporary_list = vector_new();
    vector_reserve(temporary_list, 128);
    temporary_table = table_new();
    table_reserve_bits(temporary_table, 7); // start with 128 buckets
    #endif
}

void generate_teardown(void) {
    #ifdef CCI2_IR
    clear_temporaries();
    table_delete(temporary_table);
    vector_delete(temporary_list);
    #endif
}

#ifdef CCI2_IR
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

int generate_temporary(string_t* /*nullable*/ name, bool variable) {
    // TODO asprintf calls are slow

    if (name) {
        // Try to insert with the preferred name
        char* preferred_cstr;
        asprintf(&preferred_cstr,
                "%s%s",
                variable ? "%%" : "%",
                name->bytes);
        string_t* preferred_str = string_intern_cstr(preferred_cstr);
        free(preferred_cstr);
        if (!find_temporary(preferred_str)) {
            return temporary_new(preferred_str)->id;
        }
    }

    char* cstr;
    asprintf(&cstr, "%s%zu%s%s",
            variable ? "%%" : "%",
            vector_count(temporary_list),
            name ? "_" : "",
            name ? name->bytes : "");
    string_t* str = string_intern_cstr(cstr);
    free(cstr);
    return temporary_new(str)->id;
}
#endif

#ifndef CCI2_IR

int register_alloc(token_t* /*nullable*/ token) {
    //printf("register alloc %i\n", register_next);
    int reg = register_next;
    if (register_loop_count) {
        block_append(current_block, token, PUSH, reg);
    }

    if (register_next < R9) {
        ++register_next;
    } else {
        register_next = R0;
        ++register_loop_count;
    }

    return reg;
}

void register_free(token_t* /*nullable*/ token, int reg) {
    //printf("register free %i\n", reg);
    if (register_next == R0) {
        register_next = R9;
        --register_loop_count;
    } else {
        --register_next;
    }

    if (reg != register_next) {
        fatal_token(token, "Internal error: incorrect register free");
    }

    if (register_loop_count)
        block_append(current_block, token, POP, reg);
}

#endif // CCI2_IR

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
        #ifndef CCI2_IR
        generate_location(node->last_child, reg_last);
        #endif
        #ifdef CCI2_IR
        fatal("TODO generate location IR");
        #endif
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
        #ifndef CCI2_IR
        int temp = register_alloc(node->token);
        block_append(current_block, node->token, IMW, ARGTYPE_NUMBER, temp, u64_low(llong));
        block_append(current_block, node->token, STW, temp, reg_out, 0);
        block_append(current_block, node->token, IMW, ARGTYPE_NUMBER, temp, u64_high(llong));
        block_append(current_block, node->token, STW, temp, reg_out, 4);
        register_free(node->token, temp);
        #endif
        #ifdef CCI2_IR
        // store low
        instruction_t* instruction = block_append(current_block, node->token, STW, 2);
        instruction_set_arg_number(instruction, 0, u64_low(llong));
        instruction_set_arg_temporary(instruction, 1, reg_out);

        // calc high address
        int temp = generate_temporary(NULL, false);
        instruction = block_append(current_block, node->token, ADD, 3);
        instruction_set_arg_temporary(instruction, 0, temp);
        instruction_set_arg_temporary(instruction, 1, reg_out);
        instruction_set_arg_number(instruction, 2, 4);

        // store high
        instruction = block_append(current_block, node->token, STW, 2);
        instruction_set_arg_number(instruction, 0, u64_low(llong));
        instruction_set_arg_temporary(instruction, 1, temp);
        #endif
    } else {
        #ifndef CCI2_IR
        block_append(current_block, node->token, IMW, ARGTYPE_NUMBER, reg_out, node->u32);
        #endif
        #ifdef CCI2_IR
        instruction_t* instruction = block_append(current_block, node->token, MOV, 2);
        instruction_set_arg_temporary(instruction, 0, reg_out);
        instruction_set_arg_number(instruction, 1, node->u32);
        #endif
    }
}

#ifndef CCI2_IR

static void generate_character(node_t* node, int reg_out) {
    assert(node->kind == NODE_CHARACTER);
    assert(node->first_child == NULL);
    block_append(current_block, node->token, MOV, reg_out, node->u32);
}

static void generate_string(node_t* node, int reg_out) {
    assert(node->kind == NODE_STRING);
    assert(node->first_child == NULL);
    block_append(current_block, node->token, IMW, ARGTYPE_GENERATED, reg_out, '^', STRING_LABEL_PREFIX, node->string_label);
    block_append(current_block, node->token, ADD, reg_out, RPP, reg_out);
}

#endif

// Generates access using the given opcode.
// The opcode can be ADD to generate a location, or LDB/LDS/LDW to generate a load.
// TODO once we're fully IR fix this, ADD is either SYM for a global or MOV for
// a local, code is simple enough we won't need this function
static void generate_access_impl(token_t* token, int opcode, symbol_t* symbol, int reg_out) {
    assert(opcode == ADD || !type_is_passed_indirectly(symbol->type));
    if (symbol_is_global(symbol)) {
        #ifndef CCI2_IR
        block_append(current_block, token, IMW, ARGTYPE_NAME, reg_out, '^', string_cstr(symbol->asm_name));
        block_append(current_block, token, opcode, reg_out, RPP, reg_out);
        #endif
        #ifdef CCI2_IR
        instruction_t* instruction = block_append(current_block, token, (opcode == ADD) ? SYM : opcode, 2);
        instruction_set_arg_temporary(instruction, 0, reg_out);
        instruction_set_arg_absolute(instruction, 1, symbol->asm_name);
        #endif
    } else {
        #ifndef CCI2_IR
        if (symbol->offset <= 127 && symbol->offset >= -112) {
            block_append(current_block, token, opcode, reg_out, RFP, symbol->offset);
        } else {
            block_append(current_block, token, IMW, ARGTYPE_NUMBER, reg_out, symbol->offset);
            block_append(current_block, token, opcode, reg_out, RFP, reg_out);
        }
        #endif

        #ifdef CCI2_IR
        instruction_t* instruction = block_append(current_block, token, (opcode == ADD) ? MOV : opcode, 2);
        instruction_set_arg_temporary(instruction, 0, reg_out);
        instruction_set_arg_temporary(instruction, 1, symbol->temporary);
        #endif
    }
}

static void generate_access_location(token_t* token, symbol_t* symbol, int reg_out) {
    generate_access_impl(token, ADD, symbol, reg_out);
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
        #ifndef CCI2_IR
        block_append(current_block, node->token, IMW, ARGTYPE_NUMBER, reg_out, symbol->u32);
        #endif
        #ifdef CCI2_IR
        instruction_t* instruction = block_append(current_block, node->token, MOV, 2);
        instruction_set_arg_temporary(instruction, 0, reg_out);
        instruction_set_arg_number(instruction, 1, symbol->u32);
        #endif
        return;
    }

    if (type_is_array(type)) {
        // An array access is a pointer to the first element.
        generate_access_location(node->token, node->symbol, reg_out);
        return;
    }

    if (type_is_passed_indirectly(type)) {
    #ifdef CCI2_IR
    fatal("TODO IR generate_access() indirect");
    #endif
    #ifndef CCI2_IR
        #ifndef CCI2_IR
        int reg_temp = register_alloc(node->token);
        #endif
        #ifdef CCI2_IR
        int reg_temp = generate_temporary(NULL, false);
        #endif
        generate_access_location(node->token, node->symbol, reg_temp);
        generate_copy(node->token, type, 1, reg_temp, reg_out);
        #ifndef CCI2_IR
        register_free(node->token, reg_temp);
        #endif
        return;
    #endif
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
    generate_access_impl(node->token, opcode, node->symbol, reg_out);
}

#ifdef CCI2_IR
static void generate_variable(node_t* node) {
    assert(node->kind == NODE_VARIABLE);

    node->symbol->temporary = generate_temporary(NULL, false);

    int temp = node->symbol->temporary;
    instruction_t* instruction = block_append(current_block, node->token, VAR, 3);
    instruction_set_arg_temporary(instruction, 0, temp);
    instruction_set_arg_number(instruction, 1, type_size(node->symbol->type));
    instruction_set_arg_sentinel(instruction, 2); // TODO alignment

    if (node->first_child) {
        fatal("TODO IR generate_initializer()");
        //generate_initializer(node, temp);
    }
}
#endif

#ifndef CCI2_IR

/**
 * Generates offsets for function parameters, returning the necessary amount of
 * space in the stack frame.
 */
static int generate_parameter_offsets(function_t* function) {

    // The frame pointer points to the previous frame pointer. The arguments
    // are above it.
    int indirect_offset = 8;
    int frame_size = 0;

    // If the return value is indirect, the return value pointer is above it.
    if (type_is_passed_indirectly(function->root->type)) {
        indirect_offset += 4;
    }

    // All nodes except the last child of the function are parameters. The
    // first four register-size arguments are passed in registers; other
    // arguments are passed on the stack.
    size_t reg_count = 0;
    for (node_t* param = function->root->first_child;
            param != function->root->last_child;
            param = param->right_sibling)
    {
        assert(param->kind == NODE_PARAMETER);

        type_t* type = param->type;
        int size = (int)type_size(type);

        if (reg_count == 4 || type_is_passed_indirectly(type)) {
            // The argument is already on the stack. All arguments are
            // word-aligned. (The symbol won't exist if the parameter is
            // unnamed.)
            if (param->symbol) {
                param->symbol->offset = indirect_offset;
            }
            indirect_offset += (size + 3) & ~3;
        } else {
            // This argument is in a register. Generate a local variable on the
            // stack for it. (It will be moved in later after the function
            // preamble is generated once we know the full frame size.)
            // TODO for now we just word-align every parameter, we don't bother
            // to pack them.
            frame_size += 4;
            if (param->symbol) {
                param->symbol->offset = -frame_size;
            }
            ++reg_count;
        }
        //printf("assigned offset %i to param %s of size %i\n", param->symbol->offset, param->symbol->name->bytes, (int)type_size(param->symbol->type));
    }

    function->variadic_offset = indirect_offset;
    return frame_size;
}

/**
 * Generates offsets for all local variables, calculating the total size of the
 * stack frame.
 */
static int generate_variable_offsets(node_t* node, int offset, int frame_size) {
    for (node_t* child = node->first_child; child; child = child->right_sibling) {
        if (child->kind == NODE_VARIABLE && child->symbol->linkage == symbol_linkage_none) {
            int size = (int)type_size(child->symbol->type);

            // align the value
            if (size == 2)
                offset &= ~1;
            else if (size > 2)
                offset &= ~3;

            offset -= size;
            child->symbol->offset = offset;
            //printf("assigned offset %i to var %s of size %i\n", offset, child->symbol->name->bytes, (int)type_size(child->symbol->type));
            if (frame_size < -offset) {
                frame_size = -offset;
            }
        }

        int child_frame_size = generate_variable_offsets(child, offset, frame_size);
        if (child_frame_size > frame_size) {
            frame_size = child_frame_size;
        }
    }

    return frame_size;
}

#endif // !CCI2_IR

void generate_function(function_t* function) {
    node_t* root = function->root;
    emit_source_location(root->token);

    #ifndef CCI2_IR
    // walk the tree, genererating frame offsets for each variable
    int frame_size = generate_parameter_offsets(function);
    frame_size = generate_variable_offsets(root, -frame_size, frame_size);
    frame_size = (frame_size + 3) & ~3;
    #endif

    // create the entry block
    current_function = function;
    current_block = block_new(next_label++);
    function_add_block(function, current_block);

    #ifndef CCI2_IR
    // create the stack frame
    block_append(current_block, root->token, ENTER);
    // note: we don't use block_append_op_imm() because we haven't allocated
    // any registers or saved our arguments yet, so if the stack size is too
    // big, it will clobber r0.
    if (frame_size <= 127) {
        if (frame_size != 0) {
            block_append(current_block, root->token, SUB, RSP, RSP, frame_size);
        }
    } else {
        block_append(current_block, root->token, IMW, ARGTYPE_NUMBER, R9, frame_size);
        block_append(current_block, root->token, SUB, RSP, RSP, R9);
    }

    // move register arguments into local variables
    int param_reg = R0;
    for (node_t* param = root->first_child;
            param != root->last_child && param_reg != R4;
            param = param->right_sibling)
    {
        assert(param->kind == NODE_PARAMETER);
        if (!type_is_passed_indirectly(param->type)) {
            if (param->symbol) {
                int offset = -(param_reg - R0 + 1) * 4;
                block_append(current_block, param->token, STW, param_reg, RFP, offset);
            }
            ++param_reg;
        }
    }

    // generate the function contents
    int reg = register_alloc(root->token);
    generate_node(root->last_child, reg);
    register_free(root->token, reg);
    #endif // CCI2_IR

    #ifdef CCI2_IR
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
    #endif

    #ifndef CCI2_IR
    // If the last block doesn't end in 'ret', we add a return. If the function
    // is main, we have to return 0.
    size_t count = block_count(current_block);
    if (count == 0 || block_at(current_block, count - 1)->opcode != RET) {
        token_t* end_token = root->first_child->end_token;
        if (string_equal_cstr(function->asm_name, "main")) {
            block_append(current_block, end_token, ZERO, R0);
        }
        block_append(current_block, end_token, LEAVE);
        block_append(current_block, end_token, RET);
    }
    #endif
}

#ifndef CCI2_IR
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

    // push all registers (except for the return register)
    int last_pushed_register = register_loop_count ? R9 : register_next - 1;
    for (int i = R0; i <= last_pushed_register; ++i) {
        if (i != reg_out) {
            block_append(current_block, call->token, PUSH, i);
        }
    }

    // clear register allocator
    int old_register_next = register_next;
    int old_register_loop_count = register_loop_count;
    register_next = R0;
    register_loop_count = 0;

    // if the return value is passed indirectly, stash its pointer for now. (it
    // goes at the top of the stack after pushing args.)
    bool return_indirect = type_is_passed_indirectly(call->type);
    int reg_return_indirect = -1;
    if (return_indirect) {
        reg_return_indirect = register_alloc(call->token);
        block_append(current_block, call->token, MOV, reg_return_indirect, reg_out);
    }

    // find the last argument passed in a register
    node_t* last_register_arg = NULL;
    int register_args = 0;
    uint32_t arg_count = 0;
    for (node_t* arg = call->first_child->right_sibling; arg; arg = arg->right_sibling) {
        // to be passed by register, the argument must be a named parameter
        // (not variadic) and must fit in a register
        if (arg_count < function_type->count && !type_is_passed_indirectly(arg->type)) {
            last_register_arg = arg;
            if (++register_args == 4)
                break;
        }
        ++arg_count;
    }

    // push all indirect args right-to-left
    // (All arguments after last_register_arg are passed on the stack. All
    // arguments before and including last_register_arg are passed on the stack
    // if and only if they do not fit in a register.)
    int stack_space = 0;
    bool last_register_arg_found = false;
    int reg_arg = register_alloc(call->token);
    for (node_t* arg = call->last_child; arg != call->first_child; arg = arg->left_sibling) {
        if (arg == last_register_arg)
            last_register_arg_found = true;

        if (type_is_passed_indirectly(arg->type)) {
            // argument does not fit in a register. make stack space for it and
            // generate it.
            size_t size = type_size(arg->type);
            block_sub_rsp(current_block, arg->token, size);
            block_append(current_block, arg->token, MOV, reg_arg, RSP);
            generate_node(arg, reg_arg);
            stack_space += size;

        } else if (!last_register_arg_found) {
            // argument fits in a register but we have too many register-size
            // arguments. generate and push it.
            generate_node(arg, reg_arg);
            block_append(current_block, arg->token, PUSH, reg_arg);
            stack_space += 4;
        }
    }
    register_free(call->token, reg_arg);

    // push the return pointer (if indirect)
    if (return_indirect) {
        block_append(current_block, call->token, PUSH, reg_return_indirect);
        register_free(call->token, reg_return_indirect);
        stack_space += 4;
    }

    // compute register args into registers r0-r3
    for (node_t* arg = call->first_child->right_sibling; arg; arg = arg->right_sibling) {
        if (type_is_passed_indirectly(arg->type))
            continue;
        generate_node(arg, register_alloc(arg->token));
        if (arg == last_register_arg)
            break;
    }

    // call the function directly if we can
    if (function->kind == NODE_ACCESS && type_is_function(function->type)) {
        block_append(current_block, call->token, CALL, ARGTYPE_NAME, '^', string_cstr(function->symbol->asm_name));

    // otherwise call it indirectly
    } else {
        int reg_func = register_alloc(function->token);
        if (type_is_function(function->type)) {
            generate_location(function, reg_func);
        } else if (type_is_pointer(function->type) && type_is_function(function->type->ref)) {
            generate_node(function, reg_func);
        } else {
            fatal("Internal error: call target is neither pointer nor function pointer");
        }
        block_append(current_block, call->token, CALL, ARGTYPE_REGISTER, reg_func);
        register_free(function->token, reg_func);
    }

    // Move the return value where it goes. (This is necessary for both direct
    // and indirect.) Often this is r0 so the value is already there; we let
    // the optimizer delete the instruction if it's useless.
    block_append(current_block, call->token, MOV, reg_out, R0);

    // pop all argument stack usage
    block_add_rsp(current_block, call->token, stack_space);

    // restore the register allocator (we don't bother to free the argument registers first)
    register_next = old_register_next;
    register_loop_count = old_register_loop_count;

    // pop all registers
    for (int i = last_pushed_register; i >= R0; --i) {
        if (i != reg_out) {
            block_append(current_block, call->token, POP, i);
        }
    }
}
#endif // !CCI2_IR

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

    #ifdef CCI2_IR
    instruction_t* instruction;
    #endif // CCI2_IR

    switch (target) {
        case BASE_BOOL:
            if (source == BASE_SIGNED_CHAR || source == BASE_UNSIGNED_CHAR) {
                #ifndef CCI2_IR
                block_append(current_block, token, TRB, reg, reg);
                #endif // !CCI2_IR
                #ifdef CCI2_IR
                instruction = block_append(current_block, token, TRB, 2);
                instruction_set_arg_temporary(instruction, 0, reg);
                instruction_set_arg_temporary(instruction, 1, reg);
                #endif // CCI2_IR
            } else if (source == BASE_SIGNED_SHORT || source == BASE_UNSIGNED_SHORT) {
                #ifndef CCI2_IR
                block_append(current_block, token, TRS, reg, reg);
                #endif // !CCI2_IR
                #ifdef CCI2_IR
                instruction = block_append(current_block, token, TRS, 2);
                instruction_set_arg_temporary(instruction, 0, reg);
                instruction_set_arg_temporary(instruction, 1, reg);
                #endif // CCI2_IR
            }
            #ifndef CCI2_IR
            block_append(current_block, token, BOOL, reg, reg);
            #endif // !CCI2_IR
            #ifdef CCI2_IR
            instruction = block_append(current_block, token, BOOL, 2);
            instruction_set_arg_temporary(instruction, 0, reg);
            instruction_set_arg_temporary(instruction, 1, reg);
            #endif // CCI2_IR
            break;

        case BASE_SIGNED_INT:
        case BASE_UNSIGNED_INT:
            if (source == BASE_SIGNED_SHORT) {
                #ifndef CCI2_IR
                block_append(current_block, token, SXS, reg, reg);
                #endif // !CCI2_IR
                #ifdef CCI2_IR
                instruction = block_append(current_block, token, SXS, 2);
                instruction_set_arg_temporary(instruction, 0, reg);
                instruction_set_arg_temporary(instruction, 1, reg);
                #endif // CCI2_IR
                break;
            }
            if (source == BASE_UNSIGNED_SHORT) {
                #ifndef CCI2_IR
                block_append(current_block, token, TRS, reg, reg);
                #endif // !CCI2_IR
                #ifdef CCI2_IR
                instruction = block_append(current_block, token, TRS, 2);
                instruction_set_arg_temporary(instruction, 0, reg);
                instruction_set_arg_temporary(instruction, 1, reg);
                #endif // CCI2_IR
                break;
            }
            // fallthrough

        case BASE_SIGNED_SHORT:
        case BASE_UNSIGNED_SHORT:
            if (source == BASE_SIGNED_CHAR) {
                #ifndef CCI2_IR
                block_append(current_block, token, SXB, reg, reg);
                #endif // !CCI2_IR
                #ifdef CCI2_IR
                instruction = block_append(current_block, token, SXB, 2);
                instruction_set_arg_temporary(instruction, 0, reg);
                instruction_set_arg_temporary(instruction, 1, reg);
                #endif // CCI2_IR
                break;
            }
            if (source == BASE_UNSIGNED_CHAR || source == BASE_BOOL) {
                #ifndef CCI2_IR
                block_append(current_block, token, TRB, reg, reg);
                #endif // !CCI2_IR
                #ifdef CCI2_IR
                instruction = block_append(current_block, token, TRB, 2);
                instruction_set_arg_temporary(instruction, 0, reg);
                instruction_set_arg_temporary(instruction, 1, reg);
                #endif // CCI2_IR
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

    #ifdef CCI2_IR
    fatal("TODO generate_cast_indirect_to_indirect() IR not implemented");
    #endif // CCI2_IR

    #ifndef CCI2_IR

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

    #endif // !CCI2_IR
}

static void generate_cast_indirect_to_direct(node_t* node,
        type_t* source, type_t* target, int reg_out)
{
    assert(type_is_passed_indirectly(source));
    assert(!type_is_passed_indirectly(target));

    #ifdef CCI2_IR
    fatal("TODO generate_cast_indirect_to_indirect() IR not implemented");
    #endif // CCI2_IR

    #ifndef CCI2_IR

    size_t source_size = type_size(source);
    size_t target_size = type_size(target);

    // The source is indirect but the target is direct. The source is
    // either a 64-bit value or a record (being cast to void), and the
    // target fits in a register.
    assert(target_size <= 4);

    base_t source_base = cast_base(source);
    base_t target_base = cast_base(target);

    if (target_base == BASE_VOID) {
        // The expression result is ignored; we don't need to make stack space
        // for it.
        generate_node(node->first_child, -1);
        return;
    }

    // We need to generate the source into stack space. We can re-use
    // the same register.
    block_sub_rsp(current_block, node->token, source_size);
    block_append(current_block, node->token, MOV, reg_out, RSP);
    generate_node(node->first_child, reg_out);

    // convert source to target
    if (target_base == BASE_VOID) {
        // nothing to do.
    } else if (source_base == BASE_DOUBLE) {
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
                // For bool we need to load both words and 'or' them together.
                int reg_temp = register_alloc(node->token);
                block_append(current_block, node->token, LDW, reg_temp, reg_out, 4);
                block_append(current_block, node->token, LDW, reg_out, reg_out, 0);
                block_append(current_block, node->token, OR, reg_out, reg_out, reg_temp);
                block_append(current_block, node->token, BOOL, reg_out, reg_out);
                register_free(node->token, reg_temp);
            } else {
                // Otherwise we can just load the low word.
                block_append(current_block, node->token, LDW, reg_out, reg_out, 0);
            }
        }
    } else {
        fatal("Internal error: unrecognized indirect to direct cast.");
    }

    block_add_rsp(current_block, node->token, source_size);

    #endif // !CCI2_IR
}

static void generate_cast_direct_to_indirect(node_t* node,
        type_t* source, type_t* target, int reg_out)
{
    assert(!type_is_passed_indirectly(source));
    assert(type_is_passed_indirectly(target));

    #ifdef CCI2_IR
    fatal("TODO generate_cast_indirect_to_indirect() IR not implemented");
    #endif // CCI2_IR

    #ifndef CCI2_IR

    // The source is direct but the target is indirect. Records cannot
    // be cast so the source fits in a register and the only
    // possibility for target is a 64-bit value.
    assert(type_size(source) <= 4);
    assert(type_size(target) == 8);

    // The register contains a pointer to 64-bit space. We need an
    // auxiliary register to generate the word-size source.
    int reg_src = register_alloc(node->token);
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
            block_append(current_block, node->token, STW, reg_src, reg_out, 0); // store the low word
            if (type_is_signed_integer(source)) {
                // sign extend
                block_append(current_block, node->token, SHRU, reg_src, reg_src, 31); // get the sign bit
                block_append(current_block, node->token, SUB, reg_src, 0, reg_src); // fill the register with the sign bit
                block_append(current_block, node->token, STW, reg_src, reg_out, 4); // store the high word
            } else {
                block_append(current_block, node->token, STW, 0, reg_out, 4); // clear the high word
            }
        }
    }

    register_free(node->token, reg_src);

    #endif // !CCI2_IR
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

#ifndef CCI2_IR

static void generate_initializer_scalar(node_t* expr, type_t* target, int reg_base, size_t offset) {

    if (type_is_array(target)) {
        if (expr->kind != NODE_STRING) {
            fatal("Internal error: Cannot initialize an array with a non-string scalar");
        }
        int reg_val = register_alloc(expr->token);
        generate_node(expr, reg_val);

        // TODO need to handle wide string arrays, currently we assume char
        size_t array_count = target->count;
        size_t string_count = expr->type->count;

        // Copy bytes from the string to fill the array
        size_t copy_count = array_count < string_count ? array_count : string_count;
        int reg_loc = register_alloc(expr->token);
        block_append(current_block, expr->token, IMW, ARGTYPE_NUMBER, reg_loc, offset);
        block_append(current_block, expr->token, ADD, reg_loc, reg_loc, reg_base);
        generate_copy(expr->token, target->ref, copy_count, reg_val, reg_loc);

        // If we're initializing a char array with too short a string, we need
        // to zero out the rest of the array.
        if (array_count > string_count) {
            block_append_op_imm(current_block, expr->token, ADD, reg_loc, reg_loc, string_count);
            generate_zero_array(expr->token, target->ref, array_count - string_count, reg_loc);
        }

        register_free(expr->token, reg_loc);
        register_free(expr->token, reg_val);

    } else if (type_is_passed_indirectly(target)) {
        if (offset == 0) {
            generate_node(expr, reg_base);
        } else {
            int reg_loc = register_alloc(expr->token);
            block_append(current_block, expr->token, IMW, ARGTYPE_NUMBER, reg_loc, offset);
            block_append(current_block, expr->token, ADD, reg_loc, reg_loc, reg_base);
            generate_node(expr, reg_loc);
            register_free(expr->token, reg_loc);
        }

    } else {
        int reg_val = register_alloc(expr->token);
        generate_node(expr, reg_val);
        generate_store_offset(expr->token, target, reg_val, reg_base, offset);
        register_free(expr->token, reg_val);
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
        int reg_loc = register_alloc(list->token);
        block_append(current_block, list->token, IMW, ARGTYPE_NUMBER, reg_loc,
                base_offset + i * type_size(type->ref));
        block_append(current_block, list->token, ADD, reg_loc, reg_loc, reg_base);
        generate_zero_array(list->token, type->ref, type->count - i, reg_loc);
        register_free(list->token, reg_loc);

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
 * statement of a statement expression) so the given register is available for
 * our use. We use it as a pointer to the current location in the variable
 * being initialized.
 */
static void generate_initializer(node_t* variable, int reg_loc) {
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
    block_append_op_imm(current_block, node->token, ADD, reg_ptr, reg_ptr, offset);

    // if this is an array, the pointer to it is already in the source
    // register, so we just need to move it to the destination.
    if (type_is_array(node->type)) {
        block_append(current_block, node->token, MOV, reg_out, reg_ptr);
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
    block_append(current_block, node->token, opcode, reg_out, 0, reg_ptr);
}

static void generate_dereference(node_t* node, int reg_out) {

    // When passing directly, we can use the same register for location and
    // value; otherwise we need to generate in a temporary register.
    int reg_loc;
    bool indirect = !type_is_array(node->type) && type_is_passed_indirectly(node->type);
    if (indirect) {
        reg_loc = register_alloc(node->token);
    } else {
        reg_loc = reg_out;
    }

    generate_node(node->first_child, reg_loc);
    generate_dereference_impl(node, reg_out, reg_loc, 0);

    if (indirect) {
        register_free(node->token, reg_loc);
    }
}

static void generate_array_subscript(node_t* node, int reg_out) {

    // When passing directly, we can use the same register for location and
    // value; otherwise we need to generate in a temporary register.
    int reg_loc;
    bool indirect = !type_is_array(node->type) && type_is_passed_indirectly(node->type);
    if (indirect) {
        reg_loc = register_alloc(node->token);
    } else {
        reg_loc = reg_out;
    }

    generate_location_array_subscript(node, reg_loc);
    generate_dereference_impl(node, reg_out, reg_loc, 0);

    if (indirect) {
        register_free(node->token, reg_loc);
    }
}

static void generate_member_val(node_t* node, int reg_out) {

    // When passing directly, we can use the same register for location and
    // value; otherwise we need to generate in a temporary register.
    int reg_loc;
    bool indirect = !type_is_array(node->type) && type_is_passed_indirectly(node->type);
    if (indirect) {
        reg_loc = register_alloc(node->token);
    } else {
        reg_loc = reg_out;
    }

    generate_location(node->first_child, reg_loc);
    generate_dereference_impl(node, reg_out, reg_loc, node->member_offset);

    if (indirect) {
        register_free(node->token, reg_loc);
    }
}

static void generate_member_ptr(node_t* node, int reg_out) {

    // When passing directly, we can use the same register for location and
    // value; otherwise we need to generate in a temporary register.
    int reg_loc;
    bool indirect = !type_is_array(node->type) && type_is_passed_indirectly(node->type);
    if (indirect) {
        reg_loc = register_alloc(node->token);
    } else {
        reg_loc = reg_out;
    }

    generate_node(node->first_child, reg_loc);
    generate_dereference_impl(node, reg_out, reg_loc, node->member_offset);

    if (indirect) {
        register_free(node->token, reg_loc);
    }
}

static void generate_location_member_val(node_t* node, int reg_out) {
    generate_location(node->first_child, reg_out);
    block_append_op_imm(current_block, node->token, ADD, reg_out, reg_out, node->member_offset);
}

static void generate_location_member_ptr(node_t* node, int reg_out) {
    generate_node(node->first_child, reg_out);
    block_append_op_imm(current_block, node->token, ADD, reg_out, reg_out, node->member_offset);
}

static void generate_location_array_subscript(node_t* node, int reg_out) {
    generate_indirection_add_sub(node, reg_out);
}

static void generate_sizeof(node_t* node, int reg_out) {
    unsigned size = type_size(node->first_child->type);
    block_append(current_block, node->token, IMW, ARGTYPE_NUMBER, reg_out, size);
}

static void generate_address_of(node_t* node, int reg_out) {
    generate_location(node->first_child, reg_out);
}

#ifdef GENERATE_DEBUG
int debug_depth;
#endif

#endif // !CCI2_IR

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
    // we don't overflow the stack; see test `decl/struct-assign-large.c`. For
    // most nodes we really only need to evaluate the children for side
    // effects.
    int reg_out = reg_out_opt;
    if (reg_out == -1 || type_matches_base(node->type, BASE_VOID)) {

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
            // TODO we're only using -1 if it's indirect, because if it's
            // direct we want the register to be available as a temporary. This
            // won't be necessary when we're generating IR.
            #ifndef CCI2_IR
            case NODE_ASSIGN:
                if (type_is_passed_indirectly(node->type)) {
                    generate_assign(node, -1);
                    #ifdef GENERATE_DEBUG
                    --debug_depth;
                    #endif
                    return;
                }
                break;
            #endif // CCI2_IR
            case NODE_SEQUENCE:
                if (type_is_passed_indirectly(node->type)) {
                    generate_sequence(node, false, -1);
                    #ifdef GENERATE_DEBUG
                    --debug_depth;
                    #endif
                    return;
                }
                break;

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

            // For any case not handled above, we will have to create stack
            // space to store a temporary value.
            default:
                break;
        }

        if (reg_out == -1) {
            // Allocate space to store the result.
            #ifndef CCI2_IR
            reg_out = register_alloc(node->token);
            if (type_is_passed_indirectly(node->type)) {
                block_sub_rsp(current_block, node->token, type_size(node->type));
                block_append(current_block, node->token, MOV, reg_out, RSP);
            }
            #endif // !CCI2_IR
            #ifdef CCI2_IR
            reg_out = generate_temporary(NULL, false);
            if (type_is_passed_indirectly(node->type)) {
                fatal("TODO IR generate var for indirect node");
            }
            #endif // CCI2_IR
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
            #ifdef CCI2_IR
            generate_variable(node);
            #endif
            #ifndef CCI2_IR
            if (node->first_child) {
                generate_initializer(node, reg_out);
            }
            #endif
            break;

        // statements
        case NODE_WHILE: generate_while(node, reg_out); break;
        case NODE_DO: generate_do(node, reg_out); break;
        case NODE_FOR: generate_for(node, reg_out); break;
        case NODE_BREAK: generate_break(node, reg_out); break;
        case NODE_CONTINUE: generate_continue(node, reg_out); break;
        case NODE_RETURN: generate_return(node, reg_out); break;
        #ifndef CCI2_IR
        case NODE_SWITCH: generate_switch(node, reg_out); break;
        case NODE_GOTO: generate_goto(node, reg_out); break;

        // labels
        case NODE_LABEL: generate_label(node, reg_out); break;
        case NODE_CASE: // fallthrough
        case NODE_DEFAULT: generate_case_or_default(node, reg_out); break;

        // assignment expressions
        #endif // !CCI2_IR
        case NODE_ASSIGN: generate_assign(node, reg_out); break;
        #ifndef CCI2_IR
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
        #endif // !CCI2_IR
        case NODE_EQUAL: generate_equal(node, reg_out); break;
        case NODE_NOT_EQUAL: generate_not_equal(node, reg_out); break;
        #ifndef CCI2_IR
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
        #endif // !CCI2_IR
        case NODE_CAST: generate_cast(node, reg_out); break;
        #ifndef CCI2_IR
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
        #endif // !CCI2_IR

        // other expressions
        case NODE_IF: generate_if(node, reg_out); break;
        case NODE_SEQUENCE: generate_sequence(node, false, reg_out); break;
        #ifndef CCI2_IR
        case NODE_CHARACTER: generate_character(node, reg_out); break;
        case NODE_STRING: generate_string(node, reg_out); break;
        #endif // !CCI2_IR
        case NODE_NUMBER: generate_number(node, reg_out); break;
        #ifndef CCI2_IR
        case NODE_ACCESS: generate_access(node, reg_out); break;
        case NODE_CALL: generate_call(node, reg_out); break;
        case NODE_BUILTIN: generate_builtin(node, reg_out); break;
        #endif // !CCI2_IR

        #ifdef CCI2_IR
        default:
            fprintf(stderr, "TODO IR unimplemented node %s\n", node_kind_to_string(node->kind));
            fatal("TODO");
        #endif // !CCI2_IR
    }

    #ifndef CCI2_IR
    if (reg_out_opt == -1) {
        if (type_is_passed_indirectly(node->type)) {
            block_add_rsp(current_block, node->token, type_size(node->type));
        }
        register_free(node->token, reg_out);
    }
    #endif // !CCI2_IR

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
        #ifndef CCI2_IR
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
        #endif
        default:
            #ifdef CCI2_IR
            fatal("TODO unimplemented generate_location() %s", node_kind_to_string(node->kind));
            #endif
            fatal("Internal error, cannot generate location of non-value node: %s.", node_kind_to_string(node->kind));
            break;
    }

    #ifdef GENERATE_DEBUG
    --debug_depth;
    #endif
}

#ifndef CCI2_IR

/**
 * Generates an initializer for a variable with static storage duration (i.e. a
 * global or static local variable.)
 *
 * We emit a static function definition with a unique name and a constructor
 * of priority 50. This runs before main(), and the minimum priority for
 * __attribute__((constructor(<priority>))) is 101 so this also runs before any
 * user constructor functions in GNU C. The variable is therefore initialized
 * before any user C code.
 */
static void generate_static_initializer(struct symbol_t* varsym, struct node_t* initializer) {

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
    token_t* name = token_new_at(name_str, initializer->token);

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

    // Add a variable node for the initializer
    node_t* variable = node_new(NODE_VARIABLE);
    variable->symbol = symbol_ref(varsym);
    variable->type = type_ref(void_t);
    node_append(root, variable);
    node_append(variable, initializer);

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
    #ifdef CCI2_IR
    clear_temporaries();
    #endif
}

#endif // !CCI2_IR

void generate_static_variable(struct symbol_t* symbol, struct node_t* /*nullable*/ initializer) {

    // TODO if this is a tentative definition and -fcommon is specified, we should emit weak.

    emit_source_location(symbol->token);
    emit_char(symbol->linkage == symbol_linkage_internal ? '@' : '=');
    emit_string(symbol->asm_name);

    // TODO emit a zero symbol. Linker and libc don't support them yet. For
    // now we just emit a bunch of zeroes.
    for (size_t count = (type_size(symbol->type) + 3) >> 2; count-- > 0;) {
        if (!(count & 15)) {
            emit_newline();
            emit_cstr(ASM_INDENT);
        } else {
            emit_char(' ');
        }
        emit_char('0');
    }
    emit_newline();

    if (initializer) {
        #ifndef CCI2_IR
        emit_newline();
        generate_static_initializer(symbol, initializer);
        #endif
        #ifdef CCI2_IR
        fatal("TODO IR generate_static_variable() initializer");
        #endif
    }

    emit_global_divider();
}

#ifndef CCI2_IR

static void generate_builtin_va_arg(node_t* builtin, int reg_out) {

    // load the return value
    int reg_loc = register_alloc(builtin->token);
    generate_location(builtin->first_child, reg_loc);
    int reg_val = register_alloc(builtin->token);
    generate_dereference_impl(builtin->first_child, reg_val, reg_loc, 0);
    generate_dereference_impl(builtin, reg_out, reg_val, 0);

    // increment the va_list
    int reg_size = register_alloc(builtin->token);
    block_append(current_block, builtin->token, IMW, ARGTYPE_NUMBER, reg_size, type_size(builtin->type));
    block_append(current_block, builtin->token, ADD, reg_val, reg_val, reg_size);
    generate_store(builtin->token, builtin->first_child->type, reg_val, reg_loc);

    register_free(builtin->token, reg_size);
    register_free(builtin->token, reg_val);
    register_free(builtin->token, reg_loc);
}

static void generate_builtin_va_start(node_t* builtin, int reg_out) {
    generate_location(builtin->first_child, reg_out);
    int reg_val = register_alloc(builtin->token);
    block_append(current_block, builtin->token, IMW, ARGTYPE_NUMBER, reg_val, current_function->variadic_offset);
    block_append(current_block, builtin->token, ADD, reg_val, RFP, reg_val);
    generate_store(builtin->token, builtin->first_child->type, reg_val, reg_out);
    register_free(builtin->token, reg_val);
}

static void generate_builtin_va_end(node_t* builtin, int reg_out) {
    // nothing
}

static void generate_builtin_va_copy(node_t* builtin, int reg_out) {
    generate_location(builtin->first_child, reg_out);
    int reg_val = register_alloc(builtin->token);
    generate_node(builtin->last_child, reg_val);
    generate_store(builtin->token, builtin->first_child->type, reg_val, reg_out);
    register_free(builtin->token, reg_val);
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

#endif // !CCI2_IR
