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

function_t* current_function;
block_t* current_block;
int next_label;

// TODO probably change this to free a number of registers. compound assignment,
// initializer, post-increment take 3 registers
bool generate_register_push(int* register_num) {
    if (*register_num != 9)
        return false;
    block_add(current_block, PUSH, R8);
    block_add(current_block, MOV, R9, R8);
    *register_num = 8;
    return true;
}

void generate_register_pop(bool pushed) {
    if (pushed) {
        block_add(current_block, MOV, R8, R9);
        block_add(current_block, POP, R8);
    }
}

static void generate_sequence(node_t* node, int register_num) {
    assert(node->kind == NODE_SEQUENCE);
    if (node->first_child == NULL)
        return;

    // TODO we only need temporary space if the return type is indirect

    // we only need an extra register if we are passing indirectly and have
    // multiple children.
    // TODO pushing isn't necessary if only one of the children uses registers
    // (some of them might be labels for example.) we could try to defer this
    // until needed.
    bool pushed = false;
    if (node->first_child != node->last_child && type_is_passed_indirectly(node->type)) {
        pushed = generate_register_push(&register_num);
    }

    // generate all but last child
    for (node_t* child = node->first_child; child != node->last_child; child = child->right_sibling) {

        // make temporary storage (if passed indirectly)
        bool indirect = type_is_passed_indirectly(child->type);
        int reg = register_num + (pushed ? 1 : 0);
        int size;
        if (indirect) {
            size = (int)type_size(child->type);
            if (size > 0x7F) {
                block_add(current_block, IMW, ARGTYPE_NUMBER, R9, size);
                block_add(current_block, SUB, RSP, RSP, R9);
            } else {
                block_add(current_block, SUB, RSP, RSP, size);
            }
            block_add(current_block, MOV, reg, RSP, 0);
        }

        generate_node(child, reg);

        // discard the storage (if passed indirectly)
        if (indirect) {
            if (size > 0x7F) {
                block_add(current_block, IMW, ARGTYPE_NUMBER, R9, size);
                block_add(current_block, ADD, RSP, RSP, R9);
            } else {
                block_add(current_block, ADD, RSP, RSP, size);
            }
        }
    }

    // generate the last child into the destination
    generate_node(node->last_child, register_num);
    generate_register_pop(pushed);
}

static void generate_number(node_t* node, int register_num) {
    assert(node->kind == NODE_NUMBER);
    assert(node->first_child == NULL);
    // TODO for now assume it's int
    int value = node->int_value;
    if (value <= 127 && value >= -112) {
        block_add(current_block, MOV, register_num, value);
    } else {
        block_add(current_block, IMW, ARGTYPE_NUMBER, register_num, value);
    }
}

static void generate_character(node_t* node, int register_num) {
    assert(node->kind == NODE_CHARACTER);
    assert(node->first_child == NULL);
    block_add(current_block, MOV, register_num, node->int_value);
}

static void generate_string(node_t* node, int register_num) {
    assert(node->kind == NODE_STRING);
    assert(node->first_child == NULL);
    block_add(current_block, IMW, ARGTYPE_GENERATED, register_num, '^', STRING_LABEL_PREFIX, node->string_label);
    block_add(current_block, ADD, register_num, RPP, register_num);
}

static void generate_access(node_t* node, int register_num) {
    symbol_t* symbol = node->symbol;
    type_t* type = symbol->type;
    size_t size = type_size(type);

    // TODO if value is larger than a register, or is a record, need to copy it
    // to where register points. should have a generic copy function in
    // generate that copies of a known size using an extra register.
    opcode_t opcode;
    if (type_is_array(type)) {
        opcode = ADD; // decay array to pointer
        // TODO maybe don't do this, instead have the parser insert & wherever it is needed
    } else if (size == 1) {
        opcode = LDB;
    } else if (size == 2) {
        opcode = LDS;
    } else if (size == 4) {
        opcode = LDW;
    } else {
        fatal("TODO load larger than register");
    }

    if (symbol_is_global(symbol)) {
        block_add(current_block, IMW, ARGTYPE_NAME, register_num, '^', string_cstr(symbol->asm_name));
        block_add(current_block, LDW, register_num, RPP, register_num);
    } else {
        if (symbol->offset <= 127 && symbol->offset >= -112) {
            block_add(current_block, opcode, register_num, RFP, symbol->offset);
        } else {
            block_add(current_block, IMW, ARGTYPE_NUMBER, register_num, symbol->offset);
            block_add(current_block, opcode, register_num, RFP, register_num);
        }
    }
}

static void generate_access_location(symbol_t* symbol, int register_num) {
    if (symbol->offset == SYMBOL_OFFSET_GLOBAL) {
        block_add(current_block, IMW, ARGTYPE_NAME, register_num, '^', string_cstr(symbol->asm_name));
        block_add(current_block, ADD, register_num, RPP, register_num);
    } else {
        if (symbol->offset <= 127 && symbol->offset >= -112) {
            block_add(current_block, ADD, register_num, RFP, symbol->offset);
        } else {
            block_add(current_block, IMW, ARGTYPE_NUMBER, register_num, symbol->offset);
            block_add(current_block, ADD, register_num, RFP, register_num);
        }
    }
}

static void generate_return(node_t* node) {
    assert(node->kind == NODE_RETURN);
    if (node->first_child) {
        if (type_is_passed_indirectly(current_function->root->type)) {
            // The pointer to storage for the return value was pushed just
            // above the return address. We load it so we can generate the
            // return value directly into it.
            block_add(current_block, LDW, R0, RFP, 8);
        }
        generate_node(node->first_child, R0);
    } else {
        // No return value. If the function is main, we have to implicitly
        // return zero.
        if (string_equal_cstr(current_function->asm_name, "main")) {
            block_add(current_block, ZERO, R0);
        }
    }
    block_add(current_block, LEAVE);
    block_add(current_block, RET);
}

static void generate_break(node_t* node) {
    block_add(current_block, JMP, '&', JUMP_LABEL_PREFIX, node->container->end_label);
}

static void generate_continue(node_t* node) {
    block_add(current_block, JMP, '&', JUMP_LABEL_PREFIX, node->container->body_label);
}

static void generate_if(node_t* node, int register_num) {
    node_t* condition = node->first_child;
    node_t* true_node = condition->right_sibling;
    node_t* false_node = true_node->right_sibling;

    block_t* true_block = block_new(next_label++);
    block_t* false_block = false_node ? block_new(next_label++) : 0;
    block_t* end_block = block_new(next_label++);

    function_add_block(current_function, true_block);
    if (false_block)
        function_add_block(current_function, false_block);
    function_add_block(current_function, end_block);

    // TODO check range, spill if needed
    // TODO we could use the destination register if it's not indirect
    int pred_register = register_num + 1;
    generate_node(condition, pred_register);

    block_add(current_block, JNZ, pred_register, '&', JUMP_LABEL_PREFIX, true_block->label);
    block_add(current_block, JMP, '&', JUMP_LABEL_PREFIX, false_node ? false_block->label : end_block->label);

    current_block = true_block;
    generate_node(true_node, register_num);
    block_add(current_block, JMP, '&', JUMP_LABEL_PREFIX, end_block->label);

    if (false_node) {
        current_block = false_block;
        generate_node(false_node, register_num);
        block_add(current_block, JMP, '&', JUMP_LABEL_PREFIX, end_block->label);
    }

    current_block = end_block;
}

static void generate_while(node_t* node) {
    node_t* condition = node->first_child;
    node_t* body = condition->right_sibling;

    node->body_label = next_label++;
    node->end_label = next_label++;

    block_t* body_block = block_new(node->body_label);
    block_t* end_block = block_new(node->end_label);
    function_add_block(current_function, body_block);
    function_add_block(current_function, end_block);

    block_add(current_block, JMP, '&', JUMP_LABEL_PREFIX, body_block->label);

    current_block = body_block;
    generate_node(condition, R0);
    block_add(current_block, JZ, R0, '&', JUMP_LABEL_PREFIX, end_block->label);
    generate_node(body, R0);
    block_add(current_block, JMP, '&', JUMP_LABEL_PREFIX, body_block->label);

    current_block = end_block;
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
            //printf("assigning offset %i to var %s\n", offset, child->symbol->name->bytes);
            child->symbol->offset = offset;
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
    block_add(current_block, ENTER);
    block_sub_rsp_r9(current_block, frame_size);

    // move register arguments into local variables
    int param_reg = R0;
    for (node_t* param = root->first_child; param != root->last_child; param = param->right_sibling) {
        assert(param->kind == NODE_PARAMETER);
        if (param->symbol && param->symbol->offset < 0) {
            int offset = -(param_reg - R0 + 1) * 4;
            block_add(current_block, STW, param_reg, RFP, offset);
            ++param_reg;
        }
    }

    // generate the function contents
    generate_node(root->last_child, R0);

    // If the last block doesn't end in 'ret', we add a return. If the function
    // is main, we have to return 0.
    size_t count = block_count(current_block);
    if (count == 0 || block_at(current_block, count - 1)->opcode != RET) {
        if (string_equal_cstr(function->asm_name, "main")) {
            block_add(current_block, ZERO, R0);
        }
        block_add(current_block, LEAVE);
        block_add(current_block, RET);
    }
}

static void generate_call(node_t* call, int register_num) {

    // push all registers (not including the return register)
    for (int i = R0; i != register_num; ++i) {
        block_add(current_block, PUSH, i);
    }

    // if the return value is passed indirectly, stash its pointer in r0 for
    // now. (it goes at the top of the stack after pushing args.)
    bool return_indirect = type_is_passed_indirectly(call->type);
    int first_register = return_indirect ? R1 : R0;
    if (return_indirect) {
        block_add(current_block, MOV, R0, register_num);
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
    for (node_t* arg = call->last_child; arg != call->first_child; arg = arg->left_sibling) {
        if (arg == last_register_arg)
            last_register_arg_found = true;

        if (type_is_passed_indirectly(arg->type)) {
            // argument does not fit in a register. make stack space for it and
            // generate it.
            size_t size = type_size(arg->type);
            block_sub_rsp_r9(current_block, size);
            block_add(current_block, MOV, first_register, RSP);
            generate_node(arg, first_register);
            stack_space += size;

        } else if (!last_register_arg_found) {
            // argument fits in a register but we have too many register-size
            // arguments. generate and push it.
            generate_node(arg, first_register);
            block_add(current_block, PUSH, first_register);
            stack_space += 4;
        }
    }

    // push the return pointer (if indirect)
    if (return_indirect) {
        block_add(current_block, PUSH, R0);
        stack_space += 4;
    }

    // compute register args into registers r0-r3
    int reg = R0;
    for (node_t* arg = call->first_child->right_sibling; arg; arg = arg->right_sibling) {
        if (type_is_passed_indirectly(arg->type))
            continue;
        generate_node(arg, reg++);
        if (arg == last_register_arg)
            break;
    }

    // call the function
    node_t* function = call->first_child;
    if (function->kind == NODE_ACCESS && type_is_function(function->type)) {
        block_add(current_block, CALL, '^', string_cstr(function->symbol->asm_name));
    } else {
        // It's a function pointer expression.
        // TODO compile the expression into r4 and call it
        fatal("TODO function pointer calling not implemented yet");
    }

    // move the return value where it goes (this is necessary for both direct
    // and indirect)
    block_add(current_block, MOV, register_num, R0);

    // pop all argument stack usage
    block_add_rsp_r9(current_block, stack_space);

    // pop all registers
    for (int i = register_num; i-- != R0;) {
        block_add(current_block, POP, i);
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
        case BASE_CHAR:
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

void generate_cast(node_t* node, int register_num) {
    base_t target_base = cast_base(node->type);
    base_t source_base = cast_base(node->first_child->type);
    if (target_base == source_base) {
        // TODO we also need BASE_DOUBLE and BASE_LONG_DOUBLE to be equivalent
        generate_node(node->first_child, register_num);
        return;
    }

    size_t target_size = base_size(target_base);
    size_t source_size = base_size(source_base);

    if (source_size > 4) {
        if (target_size <= 4) {

            // generate the source into stack space. we can re-use the same
            // register.
            assert(source_size < 0x80);
            block_add(current_block, SUB, RSP, RSP, (uint8_t)source_size);
            block_add(current_block, MOV, register_num, RSP);
            generate_node(node->first_child, register_num);

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
                    block_add(current_block, LDW, register_num, register_num, 0);
                }
            }

            block_add(current_block, ADD, RSP, RSP, (uint8_t)source_size);

        } else {

            // Casting from llong or double to llong or double.
            // The register already contains a pointer to 64-bit space. We can
            // use it to generate our source, then convert to target in-place.
            generate_node(node->first_child, register_num);

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
            generate_node(node->first_child, register_num);

            // For simplicity, if the source size is signed and less than register
            // size, we just sign extend right away. This can lead to some redundant
            // combinations but we can optimize those out afterwards.
            switch (source_base) {
                case BASE_CHAR:
                case BASE_SIGNED_CHAR:
                    block_add(current_block, SXB, register_num, register_num);
                    break;
                case BASE_SIGNED_SHORT:
                    block_add(current_block, SXS, register_num, register_num);
                    break;
                default:
                    break;
            }

            // TODO aren't these target casts redundant? we should only do
            // these to source casts. no point in doing sxb if we're just going
            // to follow it up with stb for example. pretty sure these casts
            // are all wrong
            switch (target_base) {
                case BASE_CHAR:
                case BASE_SIGNED_CHAR:
                    block_add(current_block, SXB, register_num, register_num);
                    break;
                case BASE_UNSIGNED_CHAR:
                    block_add(current_block, TRB, register_num, register_num);
                    break;
                case BASE_SIGNED_SHORT:
                    block_add(current_block, SXS, register_num, register_num);
                    break;
                case BASE_UNSIGNED_SHORT:
                    block_add(current_block, TRS, register_num, register_num);
                    break;
                default:
                    break;
            }
        }
    }
}

void generate_unary_minus(node_t* node, int register_num) {
    generate_node(node->first_child, register_num);
    if (type_size(node->type) > 4) {
        fatal("TODO codegen negate long long, float, etc.");
    }
    block_add(current_block, SUB, register_num, 0, register_num);
}

void generate_initializer(node_t* variable, int register_num) {

    // We need two free registers to do the store.
    // TODO the first register push here is probably not necessary, or at least
    // there is a way of optimizing it in most cases (e.g. when the declaration
    // is in a void sequence.) We do it all the time anyway for simplicity.
    bool pushed_1 = generate_register_push(&register_num);
    ++register_num;
    bool pushed_2 = generate_register_push(&register_num);

    node_t* initializer = variable->first_child;
    if (initializer->kind == NODE_INITIALIZER_LIST) {
        fatal("TODO generate initializer list not yet implemented");
    }

    generate_node(initializer, register_num);
    generate_access_location(variable->symbol, register_num + 1);
    // TODO figure out cast or whatever, probably the parser should do it
    generate_store(variable->symbol->type, register_num + 1, register_num);

    generate_register_pop(pushed_2);
    generate_register_pop(pushed_1);
}

void generate_dereference(node_t* node, int register_num) {
    if (type_is_passed_indirectly(node->type)) {
        fatal_token(node->token, "TODO dereference indirectly, need function to memcpy struct");
    } else {
        generate_node(node->first_child, register_num);
        size_t size = type_size(node->type);
        opcode_t opcode;
        if (size == 1) {
            opcode = LDB;
        } else if (size == 2) {
            opcode = LDS;
        } else if (size == 4) {
            opcode = LDW;
        }
        block_add(current_block, opcode, register_num, 0, register_num);
    }
}

static void generate_member_impl(type_t* type, int register_num, int member_offset) {
    block_add(current_block, ADD, register_num + 1, register_num + 1, member_offset);

    // TODO for now this is copied from generate_dereference() above, need a
    // shared way to generate code to dereference a ptr
    size_t size = type_size(type);
    opcode_t opcode;
    if (size == 1) {
        opcode = LDB;
    } else if (size == 2) {
        opcode = LDS;
    } else if (size == 4) {
        opcode = LDW;
    }
    block_add(current_block, opcode, register_num, 0, register_num + 1);
}

static void generate_member_val(node_t* node, int register_num) {
    bool pushed = generate_register_push(&register_num);
    generate_location(node->first_child, register_num + 1);
    generate_member_impl(node->type, register_num, node->member_offset);
    generate_register_pop(pushed);
}

static void generate_member_ptr(node_t* node, int register_num) {
    bool pushed = generate_register_push(&register_num);
    generate_node(node->first_child, register_num + 1);
    generate_register_pop(pushed);
}

static void generate_location_member_val(node_t* node, int register_num) {
    generate_location(node->first_child, register_num);
    block_add(current_block, ADD, register_num, register_num, node->member_offset);
}

static void generate_location_member_ptr(node_t* node, int register_num) {
    generate_node(node->first_child, register_num);
    block_add(current_block, ADD, register_num, register_num, node->member_offset);
}

void generate_node(node_t* node, int register_num) {
    switch (node->kind) {
        case NODE_INVALID:
            fatal("Internal error: cannot generate unrecognized node.");
            break;

        // these nodes are generated separately, not through generate_node()
        case NODE_FUNCTION:
            fatal("Internal error: cannot generate arbitrary FUNCTION node.");
        case NODE_PARAMETER:
            fatal("Internal error: cannot generate arbitrary PARAMETER node.");
        case NODE_INITIALIZER_LIST:
            fatal("Internal error: cannot generate arbitrary INITIALIZER_LIST node.");

        case NODE_VARIABLE:
            if (node->first_child) {
                generate_initializer(node, register_num);
            }
            break;

        // statements
        case NODE_WHILE: generate_while(node); break;
        case NODE_DO: fatal_token(node->token, "TODO generate DO");
        case NODE_FOR: fatal_token(node->token, "TODO generate FOR");
        case NODE_SWITCH: fatal_token(node->token, "TODO generate SWITCH");
        case NODE_CASE: fatal_token(node->token, "TODO generate CASE");
        case NODE_DEFAULT: fatal_token(node->token, "TODO generate DEFAULT");
        case NODE_BREAK: generate_break(node); break;
        case NODE_CONTINUE: generate_continue(node); break;
        case NODE_RETURN: generate_return(node); break;
        case NODE_GOTO: fatal_token(node->token, "TODO generate GOTO");

        // assignment expressions
        case NODE_ASSIGN: generate_assign(node, register_num); break;
        case NODE_ADD_ASSIGN: fatal_token(node->token, "TODO generate ADD_ASSIGN");
        case NODE_SUB_ASSIGN: fatal_token(node->token, "TODO generate SUB_ASSIGN");
        case NODE_MUL_ASSIGN: fatal_token(node->token, "TODO generate MUL_ASSIGN");
        case NODE_DIV_ASSIGN: fatal_token(node->token, "TODO generate DIV_ASSIGN");
        case NODE_MOD_ASSIGN: fatal_token(node->token, "TODO generate MOD_ASSIGN");
        case NODE_AND_ASSIGN: fatal_token(node->token, "TODO generate AND_ASSIGN");
        case NODE_OR_ASSIGN: fatal_token(node->token, "TODO generate OR_ASSIGN");
        case NODE_XOR_ASSIGN: fatal_token(node->token, "TODO generate XOR_ASSIGN");
        case NODE_SHL_ASSIGN: fatal_token(node->token, "TODO generate SHL_ASSIGN");
        case NODE_SHR_ASSIGN: fatal_token(node->token, "TODO generate SHR_ASSIGN");

        // other binary expressions
        case NODE_LOGICAL_OR: fatal_token(node->token, "TODO generate LOGICAL_OR");
        case NODE_LOGICAL_AND: fatal_token(node->token, "TODO generate LOGICAL_AND");
        case NODE_BIT_OR: generate_bit_or(node, register_num); break;
        case NODE_BIT_XOR: fatal_token(node->token, "TODO generate BIT_XOR");
        case NODE_BIT_AND: fatal_token(node->token, "TODO generate BIT_AND");
        case NODE_EQUAL: generate_equal(node, register_num); break;
        case NODE_NOT_EQUAL: generate_not_equal(node, register_num); break;
        case NODE_LESS: generate_less(node, register_num); break;
        case NODE_GREATER: generate_greater(node, register_num); break;
        case NODE_LESS_OR_EQUAL: generate_less_or_equal(node, register_num); break;
        case NODE_GREATER_OR_EQUAL: generate_greater_or_equal(node, register_num); break;
        case NODE_SHL: generate_shl(node, register_num); break;
        case NODE_SHR: generate_shr(node, register_num); break;
        case NODE_ADD: generate_add(node, register_num); break;
        case NODE_SUB: generate_sub(node, register_num); break;
        case NODE_MUL: generate_mul(node, register_num); break;
        case NODE_DIV: generate_div(node, register_num); break;
        case NODE_MOD: generate_mod(node, register_num); break;

        // unary expressions
        case NODE_CAST: generate_cast(node, register_num); break;
        case NODE_SIZEOF: fatal_token(node->token, "TODO generate SIZEOF");
        case NODE_TYPEOF: fatal_token(node->token, "TODO generate TYPEOF");
        case NODE_TYPEOF_UNQUAL: fatal_token(node->token, "TODO generate TYPEOF_UNQUAL");
        case NODE_UNARY_PLUS: fatal_token(node->token, "TODO generate UNARY_PLUS");
        case NODE_UNARY_MINUS: generate_unary_minus(node, register_num); break;
        case NODE_BIT_NOT: generate_bit_not(node, register_num); break;
        case NODE_LOGICAL_NOT: generate_log_not(node, register_num); break;
        case NODE_DEREFERENCE: generate_dereference(node, register_num); break;
        case NODE_ADDRESS_OF: generate_location(node, register_num); break;
        case NODE_PRE_INC: fatal_token(node->token, "TODO generate PRE_INC");
        case NODE_PRE_DEC: fatal_token(node->token, "TODO generate PRE_DEC");

        // postfix operators
        case NODE_POST_INC: fatal_token(node->token, "TODO generate POST_INC");
        case NODE_POST_DEC: fatal_token(node->token, "TODO generate POST_DEC");
        case NODE_ARRAY_INDEX: fatal_token(node->token, "TODO generate ARRAY_INDEX");
        case NODE_MEMBER_VAL: generate_member_val(node, register_num); break;
        case NODE_MEMBER_PTR: generate_member_ptr(node, register_num); break;

        // other expressions
        case NODE_IF: generate_if(node, register_num); break;
        case NODE_SEQUENCE: generate_sequence(node, register_num); break;
        case NODE_CHARACTER: generate_character(node, register_num); break;
        case NODE_STRING: generate_string(node, register_num); break;
        case NODE_NUMBER: generate_number(node, register_num); break;
        case NODE_ACCESS: generate_access(node, register_num); break;
        case NODE_CALL: generate_call(node, register_num); break;
    }
}

void generate_location(node_t* node, int register_num) {
    switch (node->kind) {
        case NODE_ACCESS:
            generate_access_location(node->symbol, register_num);
            break;
        case NODE_DEREFERENCE:
            generate_node(node->first_child, register_num);
            break;
        case NODE_MEMBER_VAL:
            generate_location_member_val(node, register_num);
            break;
        case NODE_MEMBER_PTR:
            generate_location_member_ptr(node, register_num);
            break;
        default:
            fatal("Internal error, cannot generate location of non-value.");
            break;
    }
}

void generate_global_variable(struct symbol_t* symbol, struct node_t* /*nullable*/ initializer) {

    // TODO if this is a tentative definition and -fcommon is specified, we should emit weak.

    emit_char(symbol->is_static ? '@' : '=');
    emit_string(symbol->asm_name);

    if (initializer) {
        fatal("TODO emit global variable initializer");
    } else {
        // TODO emit a zero symbol. Linker and libc don't support them yet. For
        // now we just emit a bunch of zeroes.
        for (size_t count = type_size(symbol->type) >> 2; count-- > 0;) {
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
