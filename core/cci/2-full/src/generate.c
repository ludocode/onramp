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

function_t* current_function;
block_t* current_block;
int next_label;
int register_next;       // next register to allocate
int register_loop_count; // number of times we've looped back to r0 while allocating registers

static void generate_location_array_subscript(node_t* node, int reg_out);
static void generate_access_location(token_t* token, symbol_t* symbol, int reg_out);
static void generate_builtin(node_t* node, int reg_out);
static void generate_builtin_location(node_t* node, int reg_out);

void generate_init(void) {
    register_next = R0;
}

void generate_destroy(void) {
    // nothing
}

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

static void generate_sequence(node_t* node, int reg_out) {
    assert(node->kind == NODE_SEQUENCE);
    if (node->first_child == NULL)
        return;

    // If our return value is passed indirectly, the register contains the
    // address where it should be stored. We need to preserve it so we'll need
    // a temporary register. If the return value is passed directly (including
    // void), we can use the same register for everything.
    bool indirect = type_is_passed_indirectly(node->type);
    int reg_val;
    if (indirect) {
        reg_val = register_alloc(node->token);
    } else {
        reg_val = reg_out;
    }

    // Generate all but last child using the temporary register
    for (node_t* child = node->first_child; child != node->last_child; child = child->right_sibling) {
        assert(type_matches_base(child->type, BASE_VOID));
        generate_node(child, reg_val);
    }

    // Free the register
    if (indirect) {
        register_free(node->token, reg_val);
    }

    // Make sure the last child has the same type as this sequence. This
    // ensures that it's safe to generate into the output register.
    assert(type_equal(node->type, node->last_child->type));

    // Generate the last child into the destination
    generate_node(node->last_child, reg_out);
}

static void generate_number(node_t* node, int reg_out) {
    assert(node->kind == NODE_NUMBER);
    assert(node->first_child == NULL);

    if (type_is_long_long(node->type)) {
        u64_t* llong = &node->u64;
        int reg_temp = register_alloc(node->token);
        block_append(current_block, node->token, IMW, ARGTYPE_NUMBER, reg_temp, u64_low(llong));
        block_append(current_block, node->token, STW, reg_temp, reg_out, 0);
        block_append(current_block, node->token, IMW, ARGTYPE_NUMBER, reg_temp, u64_high(llong));
        block_append(current_block, node->token, STW, reg_temp, reg_out, 4);
        register_free(node->token, reg_temp);
    } else {
        int value = (int)node->u32;
        if (value <= 127 && value >= -112) {
            block_append(current_block, node->token, MOV, reg_out, value);
        } else {
            block_append(current_block, node->token, IMW, ARGTYPE_NUMBER, reg_out, value);
        }
    }
}

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

// Generates access using the given opcode.
// The opcode can be ADD to generate a location, or LDB/LDS/LDW to generate a load.
static void generate_access_impl(token_t* token, int opcode, symbol_t* symbol, int reg_out) {
    assert(opcode == ADD || !type_is_passed_indirectly(symbol->type));
    if (symbol_is_global(symbol)) {
        block_append(current_block, token, IMW, ARGTYPE_NAME, reg_out, '^', string_cstr(symbol->asm_name));
        block_append(current_block, token, opcode, reg_out, RPP, reg_out);
    } else {
        if (symbol->offset <= 127 && symbol->offset >= -112) {
            block_append(current_block, token, opcode, reg_out, RFP, symbol->offset);
        } else {
            block_append(current_block, token, IMW, ARGTYPE_NUMBER, reg_out, symbol->offset);
            block_append(current_block, token, opcode, reg_out, RFP, reg_out);
        }
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
        block_append(current_block, node->token, IMW, ARGTYPE_NUMBER, reg_out, symbol->u32);
        return;
    }

    if (type_is_array(type)) {
        // arrays decay to pointers.
        // TODO this is probably the wrong way to do this, see generate_member_val()
        generate_access_location(node->token, node->symbol, reg_out);
        return;
    }

    if (type_is_passed_indirectly(type)) {
        int reg_temp = register_alloc(node->token);
        generate_access_location(node->token, node->symbol, reg_temp);
        generate_copy(node->token, type, 1, reg_temp, reg_out);
        register_free(node->token, reg_temp);
        return;
    }

    // TODO if value is larger than a register, or is a record, need to copy it
    // to where register points. should have a generic copy function in
    // generate that copies of a known size using an extra register.
    opcode_t opcode;
    size_t size = type_size(type);
    if (size == 1) {
        opcode = LDB;
    } else if (size == 2) {
        opcode = LDS;
    } else if (size == 4) {
        opcode = LDW;
    } else {
        fatal("Internal error: generate_access() direct has impossible size");
    }
    generate_access_impl(node->token, opcode, node->symbol, reg_out);
}

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
            param->symbol->offset = -frame_size;
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

void generate_function(function_t* function) {
    node_t* root = function->root;
    emit_source_location(root->token);

    if (dump_ast) {
        putchar('\n');
        node_print_tree(root);
        putchar('\n');
    }

    // walk the tree, genererating frame offsets for each variable
    int frame_size = generate_parameter_offsets(function);
    frame_size = generate_variable_offsets(root, -frame_size, frame_size);
    frame_size = (frame_size + 3) & ~3;

    // generate the preamble
    current_function = function;
    current_block = block_new(-1);
    function_add_block(function, current_block);
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
            param != root->last_child;
            param = param->right_sibling)
    {
        assert(param->kind == NODE_PARAMETER);
        // TODO why would param->symbol be null? if it's because the parameter
        // is unnamed, it should still skip the register if direct
        if (!param->symbol) {
            fatal("TODO unnamed parameter register args are incorrectly handled");
            continue;
        }
        if (param->symbol->offset < 0) {
            int offset = -(param_reg - R0 + 1) * 4;
            block_append(current_block, param->token, STW, param_reg, RFP, offset);
            ++param_reg;
        }
    }

    // generate the function contents
    int reg = register_alloc(root->token);
    generate_node(root->last_child, reg);
    register_free(root->token, reg);

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

    // push all registers (except for the return register)
    int last_pushed_register = register_loop_count ? R9 : reg_out;
    for (int i = R0; i != last_pushed_register; ++i) {
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
        int reg_num = register_alloc(arg->token);
        if (type_is_passed_indirectly(arg->type))
            continue;
        generate_node(arg, reg_num);
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
    for (int i = last_pushed_register; i-- != R0;) {
        if (i != reg_out) {
            block_append(current_block, call->token, POP, i);
        }
    }
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

    switch (target) {
        case BASE_BOOL:
            if (source == BASE_SIGNED_CHAR || source == BASE_UNSIGNED_CHAR) {
                block_append(current_block, token, TRB, reg, reg);
            } else if (source == BASE_SIGNED_SHORT || source == BASE_UNSIGNED_SHORT) {
                block_append(current_block, token, TRS, reg, reg);
            }
            block_append(current_block, token, BOOL, reg, reg);
            break;

        case BASE_SIGNED_INT:
        case BASE_UNSIGNED_INT:
            if (source == BASE_SIGNED_SHORT) {
                block_append(current_block, token, SXS, reg, reg);
                break;
            }
            if (source == BASE_UNSIGNED_SHORT) {
                block_append(current_block, token, TRS, reg, reg);
                break;
            }
            // fallthrough

        case BASE_SIGNED_SHORT:
        case BASE_UNSIGNED_SHORT:
            if (source == BASE_SIGNED_CHAR) {
                block_append(current_block, token, SXB, reg, reg);
                break;
            }
            if (source == BASE_UNSIGNED_CHAR || source == BASE_BOOL) {
                block_append(current_block, token, TRB, reg, reg);
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
    size_t source_size = type_size(source);
    size_t target_size = type_size(target);

    if (source_indirect) {
        if (target_indirect) {

            // Both the source and target are indirect. Records cannot be cast
            // so the only possibility is a 64-bit value.
            assert(source_size == 8);
            assert(target_size == 8);

            // The register already contains a pointer to 64-bit space. We can
            // use it to generate our source, then convert to target in-place.
            generate_node(node->first_child, reg_out);

            // The only possibility are casting signed to unsigned llong or
            // vice versa; or casting signed or unsigned llong to double or
            // vice versa.

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

        } else {

            // The source is indirect but the target is direct. The source is
            // either a 64-bit value or a record (being cast to void), and the
            // target fits in a register.
            assert(target_size <= 4);

            // We need to generate the source into stack space. We can re-use
            // the same register.
            block_sub_rsp(current_block, node->token, source_size);
            block_append(current_block, node->token, MOV, reg_out, RSP);
            generate_node(node->first_child, reg_out);

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
                    fatal("TODO cast from long long to register-size integer");
                }
            } else {
                // The only other possibility is casting a record to void. The
                // value is ignored.
                assert(target_base == BASE_VOID);
                assert(source_base == BASE_RECORD);
            }

            block_append(current_block, node->token, ADD, RSP, RSP, (uint8_t)source_size);
        }
    } else {
        if (target_indirect) {

            // The source is direct but the target is indirect. Records cannot
            // be cast so the source fits in a register and the only
            // possibility for target is a 64-bit value.
            assert(source_size <= 4);
            assert(target_size == 8);

            // The register contains a pointer to 64-bit space. We need an
            // auxiliary register to generate the word-size source.
            int reg_src = register_alloc(node->token);
            generate_node(node->first_child, reg_src);

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
                    assert(type_is_integer(source));
                    fatal("TODO cast from register integer to long long, sign extend and store");
                }
            }

            register_free(node->token, reg_src);

        } else {
            // The to and from types both fit in registers. We can use the same
            // register for both and convert in place.
            generate_node(node->first_child, reg_out);

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
    }
}

void generate_unary_plus(node_t* node, int reg_out) {
    // We don't need to do anything besides generate. Probably unary plus
    // should be an implicit cast, but we should actually insert this cast into
    // the tree, not do it in codegen.
    generate_node(node->first_child, reg_out);
}

void generate_unary_minus(node_t* node, int reg_out) {
    generate_node(node->first_child, reg_out);
    if (type_size(node->type) > 4) {
        fatal("TODO codegen negate long long, float, etc.");
    }
    block_append(current_block, node->token, SUB, reg_out, 0, reg_out);
}

void generate_initializer_scalar(node_t* expr, type_t* target, int reg_base, size_t offset) {
    int reg_val = register_alloc(expr->token);
    generate_node(expr, reg_val);

    if (type_is_array(target) && expr->kind == NODE_STRING) {
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
            block_append(current_block, expr->token, ADD, reg_loc, reg_loc, string_count);
            generate_zero(expr->token, target, array_count - string_count, reg_loc);
        }

        register_free(expr->token, reg_loc);
    } else {
        generate_store_offset(expr->token, target, reg_val, reg_base, offset);
    }

    register_free(expr->token, reg_val);
}

/*
 * Generates an initializer list.
 *
 * The address of the variable being initialized is (reg_loc + offset).
 */
void generate_initializer_list(node_t* list, type_t* type, int reg_loc, size_t base_offset) {

    for (size_t i = 0; i < vector_count(&list->initializers); ++i) {
        node_t* child = vector_at(&list->initializers, i);
        if (!child) {
            // TODO we should be looping over the whole length of the
            // struct/array, not just the length of the initializer list. any
            // initializers that are missing should be zeroed.
            continue;
        }

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
            fatal("Internal error: cannot generate initializer list for non-compound type");
        }

        if (child->kind == NODE_INITIALIZER_LIST) {
            generate_initializer_list(child, child_type, reg_loc, offset);
        } else {
            generate_initializer_scalar(child, child_type, reg_loc, offset);
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
void generate_initializer(node_t* variable, int reg_loc) {
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
            assert(type_is_array(var_type) && type_equal(init_type->ref, var_type->ref));
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

    // if this is an array, the pointer to it is already in the register, so
    // there's nothing to do.
    if (type_is_array(node->type))
        return;

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
    // value.
    if (type_is_array(node->type) || !type_is_passed_indirectly(node->type)) {
        generate_node(node->first_child, reg_out);
        generate_dereference_impl(node, reg_out, reg_out, 0);
        return;
    }

    // Otherwise we need to generate in a temporary register.
    int reg_loc = register_alloc(node->token);
    generate_node(node->first_child, reg_loc);
    generate_dereference_impl(node, reg_out, reg_loc, 0);
    register_free(node->token, reg_loc);
}

static void generate_array_subscript(node_t* node, int reg_out) {
    // TODO we don't always need to alloc a register, see generate_dereference above
    int reg_loc = register_alloc(node->token);
    generate_location_array_subscript(node, reg_loc);
    generate_dereference_impl(node, reg_out, reg_loc, 0);
    register_free(node->token, reg_loc);
}

static void generate_member_val(node_t* node, int reg_out) {
    // Arrays decay to pointers. TODO this is probably the wrong way to go
    // about this. We should probably be inserting & operators into the parse
    // tree. & will generate location and can skip the dereference for arrays.
    if (type_is_array(node->type)) {
        generate_location(node->first_child, reg_out);
        block_append_op_imm(current_block, node->token, ADD, reg_out, reg_out, node->member_offset);
        return;
    }

    // TODO we don't always need to alloc a register, see generate_dereference above
    int reg_loc = register_alloc(node->token);
    generate_location(node->first_child, reg_loc);
    generate_dereference_impl(node, reg_out, reg_loc, node->member_offset);
    register_free(node->token, reg_loc);
}

static void generate_member_ptr(node_t* node, int reg_out) {
    // Arrays decay to pointers. TODO see above
    if (type_is_array(node->type)) {
        generate_node(node->first_child, reg_out);
        block_append_op_imm(current_block, node->token, ADD, reg_out, reg_out, node->member_offset);
        return;
    }

    // TODO we don't always need to alloc a register, see generate_dereference above
    int reg_loc = register_alloc(node->token);
    generate_node(node->first_child, reg_loc);
    generate_dereference_impl(node, reg_out, reg_loc, node->member_offset);
    register_free(node->token, reg_loc);
}

static void generate_location_member_val(node_t* node, int reg_out) {
    generate_location(node->first_child, reg_out);
    block_append(current_block, node->token, ADD, reg_out, reg_out, node->member_offset);
}

static void generate_location_member_ptr(node_t* node, int reg_out) {
    generate_node(node->first_child, reg_out);
    block_append(current_block, node->token, ADD, reg_out, reg_out, node->member_offset);
}

static void generate_location_array_subscript(node_t* node, int reg_out) {
    generate_pointer_add_sub(node, reg_out);
}

static void generate_sizeof(node_t* node, int reg_out) {
    unsigned size = type_size(node->first_child->type);
    if (size < 0x80) {
        block_append(current_block, node->token, MOV, reg_out, size);
    } else {
        block_append(current_block, node->token, IMW, ARGTYPE_NUMBER, reg_out, size);
    }
}

static void generate_address_of(node_t* node, int reg_out) {
    generate_location(node->first_child, reg_out);
}

void generate_node(node_t* node, int reg_out) {
    switch (node->kind) {
        case NODE_INVALID:
            fatal("Internal error: cannot generate unrecognized node.");
            break;
        case NODE_NOOP:
            // nothing
            break;

        // these nodes are handled separately, not generated through generate_node()
        case NODE_FUNCTION:
            fatal("Internal error: cannot generate arbitrary FUNCTION node.");
        case NODE_PARAMETER:
            fatal("Internal error: cannot generate arbitrary PARAMETER node.");
        case NODE_INITIALIZER_LIST:
            fatal("Internal error: cannot generate arbitrary INITIALIZER_LIST node.");
        case NODE_TYPE:
            fatal("Internal error: cannot generate arbitrary TYPE node.");

        case NODE_VARIABLE:
            if (node->first_child) {
                generate_initializer(node, reg_out);
            }
            break;

        // statements
        case NODE_WHILE: generate_while(node, reg_out); break;
        case NODE_DO: generate_do(node, reg_out); break;
        case NODE_FOR: generate_for(node, reg_out); break;
        case NODE_SWITCH: generate_switch(node, reg_out); break;
        case NODE_BREAK: generate_break(node, reg_out); break;
        case NODE_CONTINUE: generate_continue(node, reg_out); break;
        case NODE_RETURN: generate_return(node, reg_out); break;
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
        case NODE_SEQUENCE: generate_sequence(node, reg_out); break;
        case NODE_CHARACTER: generate_character(node, reg_out); break;
        case NODE_STRING: generate_string(node, reg_out); break;
        case NODE_NUMBER: generate_number(node, reg_out); break;
        case NODE_ACCESS: generate_access(node, reg_out); break;
        case NODE_CALL: generate_call(node, reg_out); break;
        case NODE_BUILTIN: generate_builtin(node, reg_out); break;
    }
}

void generate_location(node_t* node, int reg_out) {
    switch (node->kind) {
        case NODE_ACCESS: generate_access_location(node->token, node->symbol, reg_out); break;
        case NODE_DEREFERENCE: generate_node(node->first_child, reg_out); break;
        case NODE_MEMBER_VAL: generate_location_member_val(node, reg_out); break;
        case NODE_MEMBER_PTR: generate_location_member_ptr(node, reg_out); break;
        case NODE_ARRAY_SUBSCRIPT: generate_location_array_subscript(node, reg_out); break;
        case NODE_BUILTIN: generate_builtin_location(node, reg_out); break;
        case NODE_STRING: generate_string(node, reg_out); break;
        default:
            fatal("Internal error, cannot generate location of non-value node: %s.", node_kind_to_string(node->kind));
            break;
    }
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
    node_append(root, variable);
    node_append(variable, initializer);

    // Push the current function (in case we're compiling a local static
    // variable)
    function_t* old_function = current_function;
    current_function = function;

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
}

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
        emit_newline();
        generate_static_initializer(symbol, initializer);
    }

    emit_global_divider();
}

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
