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

#include "common.h"
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

    // generate all but last child
    for (node_t* child = node->first_child; child != node->last_child; child = child->right_sibling) {

        // if both this node and the final node are passed directly, the return
        // register is free, so we can generate directly into it.
        if (!type_is_passed_indirectly(child->type) &&
               !type_is_passed_indirectly(node->last_child->type))
        {
            generate_node(child, reg_out);
            continue;
        }

        // make temporary storage
        int child_register = register_alloc(child->token);
        int size = (int)type_size(child->type);
        if (size > 0x7F) {
            block_append(current_block, node->token, IMW, ARGTYPE_NUMBER, R9, size);
            block_append(current_block, node->token, SUB, RSP, RSP, R9);
        } else {
            block_append(current_block, node->token, SUB, RSP, RSP, size);
        }
        block_append(current_block, node->token, MOV, child_register, RSP, 0);

        // generate
        generate_node(child, child_register);

        // discard the storage
        register_free(child->token, child_register);
        if (size > 0x7F) {
            block_append(current_block, node->token, IMW, ARGTYPE_NUMBER, R9, size);
            block_append(current_block, node->token, ADD, RSP, RSP, R9);
        } else {
            block_append(current_block, node->token, ADD, RSP, RSP, size);
        }
    }

    // generate the last child into the destination
    generate_node(node->last_child, reg_out);
}

static void generate_number(node_t* node, int reg_out) {
    assert(node->kind == NODE_NUMBER);
    assert(node->first_child == NULL);
    // TODO for now assume it's int
    int value = (int)node->u32;
    if (value <= 127 && value >= -112) {
        block_append(current_block, node->token, MOV, reg_out, value);
    } else {
        block_append(current_block, node->token, IMW, ARGTYPE_NUMBER, reg_out, value);
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
        block_append(current_block, node->token, IMW, ARGTYPE_NUMBER, reg_out, symbol->constant.u32);
        return;
    }

    if (type_is_array(type)) {
        // arrays decay to pointers.
        generate_access_location(node->token, node->symbol, reg_out);
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
        fatal("TODO load larger than register");
    }

    // TODO share this code with generate_access_location below
    if (symbol_is_global(symbol)) {
        block_append(current_block, node->token, IMW, ARGTYPE_NAME, reg_out, '^', string_cstr(symbol->asm_name));
        block_append(current_block, node->token, opcode, reg_out, RPP, reg_out);
    } else {
        if (symbol->offset <= 127 && symbol->offset >= -112) {
            block_append(current_block, node->token, opcode, reg_out, RFP, symbol->offset);
        } else {
            block_append(current_block, node->token, IMW, ARGTYPE_NUMBER, reg_out, symbol->offset);
            block_append(current_block, node->token, opcode, reg_out, RFP, reg_out);
        }
    }
}

static void generate_access_location(token_t* token, symbol_t* symbol, int reg_out) {
    if (symbol->offset == SYMBOL_OFFSET_GLOBAL) {
        block_append(current_block, token, IMW, ARGTYPE_NAME, reg_out, '^', string_cstr(symbol->asm_name));
        block_append(current_block, token, ADD, reg_out, RPP, reg_out);
    } else {
        if (symbol->offset <= 127 && symbol->offset >= -112) {
            block_append(current_block, token, ADD, reg_out, RFP, symbol->offset);
        } else {
            block_append(current_block, token, IMW, ARGTYPE_NUMBER, reg_out, symbol->offset);
            block_append(current_block, token, ADD, reg_out, RFP, reg_out);
        }
    }
}

/**
 * Generates offsets for function parameters, returning the necessary amount of
 * space in the stack frame.
 */
static int generate_parameter_offsets(function_t* function) {

    // The frame pointer points to the previous frame pointer. The arguments
    // are above it.
    int indirect_offset = 4;
    int frame_size = 0;

    // If the return value is indirect, the return value pointer is above it.
    if (type_is_passed_indirectly(function->root->type)) {
        indirect_offset += 4;
    }

    // All nodes except the last child of the function are parameters. The
    // first four register-size arguments are passed in registers; other
    // arguments are passed on the stack.
    for (node_t* param = function->root->first_child;
            param != function->root->last_child;
            param = param->right_sibling)
    {
        assert(param->kind == NODE_PARAMETER);
        type_t* type = param->type;
        int size = (int)type_size(type);

        if (type_is_passed_indirectly(type)) {
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
        }
        //printf("assigned offset %i to param %s of size %i\n", param->symbol->offset, param->symbol->name->bytes, (int)type_size(param->symbol->type));
    }

    return frame_size;
}

/**
 * Generates offsets for all local variables, calculating the total size of the
 * stack frame.
 */
static int generate_variable_offsets(node_t* node, int offset, int frame_size) {
    for (node_t* child = node->first_child; child; child = child->right_sibling) {
        if (child->kind == NODE_VARIABLE && child->symbol->offset == 0) {
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

    // TODO walk the tree.
    // - assign a stack frame offset to each variable and arg
    //     - args 5+ can have a positive offset! they stay where they are
    //     - DONE for vars, not for args
    // - add up "weight" of each node
    //     - weight should be estimate of how many temp registers it needs
    //     - used so binary expressions can compute heavier branches first
    //       (extra registers)
    //     - TODO don't bother with weight for now (or maybe ever)

    // TODO calculate stack frame size, emit preamble
    // TODO args

    // generate the preamble
    current_function = function;
    current_block = block_new(-1);
    function_add_block(function, current_block);
    block_append(current_block, root->token, ENTER);
    block_sub_rsp(current_block, root->token, frame_size);

    // move register arguments into local variables
    int param_reg = R0;
    for (node_t* param = root->first_child; param != root->last_child; param = param->right_sibling) {
        assert(param->kind == NODE_PARAMETER);
        if (param->symbol && param->symbol->offset < 0) {
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

static void generate_call(node_t* call, int reg_out) {

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
    for (node_t* arg = call->first_child->right_sibling; arg; arg = arg->right_sibling) {
        if (!type_is_passed_indirectly(arg->type)) {
            last_register_arg = arg;
            if (++register_args == 4)
                break;
        }
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

    // call the function
    node_t* function = call->first_child;
    if (function->kind == NODE_ACCESS && type_is_function(function->type)) {
        block_append(current_block, call->token, CALL, '^', string_cstr(function->symbol->asm_name));
    } else {
        // It's a function pointer expression.
        // TODO compile the expression into r4 and call it
        fatal("TODO function pointer calling not implemented yet");
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

// Returns the "cast base" for the given type, i.e. the underlying type we'll
// cast to. Pointers decay to unsigned int, enums decay to signed int, etc.
static base_t cast_base(type_t* type) {
    if (type_is_declarator(type)) {
        // Function types aren't allowed in a cast operator. All other
        // declarator types decay to pointers.
        return BASE_UNSIGNED_INT;
    }
    switch (type->base) {
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
    base_t target_base = cast_base(node->type);
    base_t source_base = cast_base(node->first_child->type);
    if (target_base == source_base) {
        generate_node(node->first_child, reg_out);
        return;
    }

    size_t target_size = base_size(target_base);
    size_t source_size = base_size(source_base);

    if (source_size > 4) {
        if (target_size <= 4) {

            // generate the source into stack space. we can re-use the same
            // register.
            assert(source_size < 0x80);
            block_append(current_block, node->token, SUB, RSP, RSP, (uint8_t)source_size);
            block_append(current_block, node->token, MOV, reg_out, RSP);
            generate_node(node->first_child, reg_out);

            // convert source to target
            if (source_base == BASE_DOUBLE || source_base == BASE_LONG_DOUBLE) {
                // It's a double. we need a function call to cast it.
                if (target_base == BASE_FLOAT) {
                    fatal("TODO cast from double to float, emit function call");
                } else {
                    fatal("TODO cast from double to int, emit function call");
                }
            } else {
                // It's llong.
                if (target_base == BASE_FLOAT) {
                    fatal("TODO cast from long to float, emit function call");
                } else {
                    // We're casting from llong to a register-size or smaller
                    // integer. We can just load the low word.
                    block_append(current_block, node->token, LDW, reg_out, reg_out, 0);
                }
            }

            block_append(current_block, node->token, ADD, RSP, RSP, (uint8_t)source_size);

        } else {

            // Casting from llong or double to llong or double.
            // The register already contains a pointer to 64-bit space. We can
            // use it to generate our source, then convert to target in-place.
            generate_node(node->first_child, reg_out);

            if (source_base == BASE_DOUBLE || source_base == BASE_LONG_DOUBLE) {
                fatal("TODO cast from double to llong, emit function call");
            } else {
                fatal("TODO cast from llong to double, emit function call");
            }
        }
    } else {
        if (target_size > 4) {

            // The register contains a pointer to 64-bit space. We need an
            // auxiliary register to generate the word-size source.
            // TODO make a register, generate into it, do manual stores for ints or function call for float/double
            fatal("TODO casting of large values not implemented yet");

        } else {

            // The to and from types both fit in registers. We can use the same
            // register for both and truncate or sign-extend in place.
            generate_node(node->first_child, reg_out);

            // For simplicity, if the source size is signed and less than register
            // size, we just sign extend right away. This can lead to some redundant
            // combinations but we can optimize those out afterwards.
            switch (source_base) {
                case BASE_BOOL:
                    block_append(current_block, node->token, BOOL, reg_out, reg_out);
                    break;
                case BASE_CHAR:
                case BASE_SIGNED_CHAR:
                    block_append(current_block, node->token, SXB, reg_out, reg_out);
                    break;
                case BASE_SIGNED_SHORT:
                    block_append(current_block, node->token, SXS, reg_out, reg_out);
                    break;
                default:
                    break;
            }

            // TODO aren't these target casts redundant? we should only do
            // these to source casts. no point in doing sxb if we're just going
            // to follow it up with stb for example. pretty sure these casts
            // are all wrong
            switch (target_base) {
                case BASE_BOOL:
                    block_append(current_block, node->token, BOOL, reg_out, reg_out);
                    break;
                case BASE_CHAR:
                case BASE_SIGNED_CHAR:
                    block_append(current_block, node->token, SXB, reg_out, reg_out);
                    break;
                case BASE_UNSIGNED_CHAR:
                    block_append(current_block, node->token, TRB, reg_out, reg_out);
                    break;
                case BASE_SIGNED_SHORT:
                    block_append(current_block, node->token, SXS, reg_out, reg_out);
                    break;
                case BASE_UNSIGNED_SHORT:
                    block_append(current_block, node->token, TRS, reg_out, reg_out);
                    break;
                default:
                    break;
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

void generate_initializer(node_t* variable, int reg_out) {

    // TODO the first register alloc here is probably not necessary, or at least
    // there is a way of optimizing it in most cases (e.g. when the declaration
    // is in a void sequence.) We do it all the time anyway for simplicity.

    // TODO this needs to be fixed up, we should be generating in place for
    // indirect values

    node_t* initializer = variable->first_child;
    if (initializer->kind == NODE_INITIALIZER_LIST) {
        fatal("TODO generate initializer list not yet implemented");
    }
    int reg_value = register_alloc(variable->token);
    generate_node(initializer, reg_value);

    int reg_location = register_alloc(variable->token);
    generate_access_location(variable->token, variable->symbol, reg_location);

    // TODO figure out cast or whatever, probably the parser should do it
    generate_store(variable->token, variable->symbol->type, reg_location, reg_value);

    register_free(variable->token, reg_location);
    register_free(variable->token, reg_value);
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
    block_append_op_imm(current_block, node->token, ADD, reg_ptr, offset);

    if (type_is_passed_indirectly(node->type)) {
        fatal_token(node->token, "TODO dereference indirectly, need function to memcpy struct");
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
    if (!type_is_passed_indirectly(node->type)) {
        generate_node(node->first_child, reg_out);
        generate_dereference_impl(node, reg_out, reg_out, 0);
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
    // TODO we don't always need to alloc a register, see generate_dereference above
    int reg_loc = register_alloc(node->token);
    generate_location(node->first_child, reg_loc);
    generate_dereference_impl(node, reg_out, reg_loc, node->member_offset);
    register_free(node->token, reg_loc);
}

static void generate_member_ptr(node_t* node, int reg_out) {
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
        case NODE_SWITCH: fatal_token(node->token, "TODO generate SWITCH");
        case NODE_CASE: fatal_token(node->token, "TODO generate CASE");
        case NODE_DEFAULT: fatal_token(node->token, "TODO generate DEFAULT");
        case NODE_BREAK: generate_break(node, reg_out); break;
        case NODE_CONTINUE: generate_continue(node, reg_out); break;
        case NODE_RETURN: generate_return(node, reg_out); break;
        case NODE_GOTO: fatal_token(node->token, "TODO generate GOTO");

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
        case NODE_LOGICAL_OR: fatal_token(node->token, "TODO generate LOGICAL_OR");
        case NODE_LOGICAL_AND: fatal_token(node->token, "TODO generate LOGICAL_AND");
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
        case NODE_LOGICAL_NOT: generate_log_not(node, reg_out); break;
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
    }
}

void generate_location(node_t* node, int reg_out) {
    switch (node->kind) {
        case NODE_ACCESS: generate_access_location(node->token, node->symbol, reg_out); break;
        case NODE_DEREFERENCE: generate_node(node->first_child, reg_out); break;
        case NODE_MEMBER_VAL: generate_location_member_val(node, reg_out); break;
        case NODE_MEMBER_PTR: generate_location_member_ptr(node, reg_out); break;
        case NODE_ARRAY_SUBSCRIPT: generate_location_array_subscript(node, reg_out); break;
        default:
            fatal("Internal error, cannot generate location of non-value.");
            break;
    }
}

void generate_global_variable(struct symbol_t* symbol, struct node_t* /*nullable*/ initializer) {

    // TODO if this is a tentative definition and -fcommon is specified, we should emit weak.

    emit_source_location(symbol->token);
    emit_char(symbol->is_static ? '@' : '=');
    emit_string(symbol->asm_name);

    if (initializer) {
        fatal("TODO emit global variable initializer");
    } else {
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
    }

    emit_newline();
    emit_global_divider();
}
