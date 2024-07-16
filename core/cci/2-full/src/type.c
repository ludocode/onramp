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

#include "type.h"

#include <stdlib.h>
#include <assert.h>

#include "token.h"

type_t* type_new_base(base_t base) {
    type_t* type = calloc(1, sizeof(type_t));
    if (type == NULL) {
        fatal("Out of memory.");
    }
    type->refcount = 1;
    type->is_declarator = false;
    type->base = base;
    return type;
}

static type_t* type_new_declarator(declarator_t declarator) {
    type_t* type = calloc(1, sizeof(type_t));
    if (type == NULL) {
        fatal("Out of memory.");
    }
    type->refcount = 1;
    type->is_declarator = true;
    type->declarator = declarator;
    return type;
}

type_t* type_new_record(record_t* record) {
    type_t* type = type_new_base(BASE_RECORD);
    type->record = record_ref(record);
    return type;
}

type_t* type_new_enum(enum_t* enum_) {
    type_t* type = type_new_base(BASE_ENUM);
    type->enum_ = enum_ref(enum_);
    return type;
}

type_t* type_new_pointer(type_t* pointed_to_type,
        bool is_const, bool is_volatile, bool is_restrict)
{
    type_t* type = type_new_declarator(DECLARATOR_POINTER);
    type->ref = type_ref(pointed_to_type);
    type->is_const = is_const;
    type->is_volatile = is_volatile;
    type->is_restrict = is_restrict;
    return type;
}

type_t* type_new_array(type_t* element_type, uint32_t element_count) {
    type_t* type = type_new_declarator(DECLARATOR_ARRAY);
    type->ref = type_ref(element_type);
    type->count = element_count;
    return type;
}

type_t* type_new_function(type_t* return_type, type_t** arg_types,
        token_t** arg_names, uint32_t args_count, bool is_variadic)
{
    type_t* type = type_new_declarator(DECLARATOR_FUNCTION);
    type->ref = return_type;//type_ref(return_type);
    type->count = args_count;
    type->is_variadic = is_variadic;

    // TODO memdup
    // TODO actually instead of copying we should take ownership and realloc() to the correct size

    type->args = malloc(sizeof(type_t*) * args_count);
    if (type->args == NULL) {
        fatal("Out of memory.");
    }
    memcpy(type->args, arg_types, sizeof(type_t*) * args_count);

    type->names = malloc(sizeof(token_t*) * args_count);
    if (type->names == NULL) {
        fatal("Out of memory.");
    }
    memcpy(type->names, arg_names, sizeof(token_t*) * args_count);

    return type;
}

void type_deref(type_t* type) {
    assert(type->refcount != 0);
    if (--type->refcount != 0) {
        return;
    }

    if (type->is_declarator) {
        switch (type->declarator) {
            case DECLARATOR_FUNCTION:
                for (uint32_t i = 0; i < type->count; ++i) {
                    type_deref(type->args[i]);
                    if (type->names[i]) {
                        token_deref(type->names[i]);
                    }
                }
                free(type->args);
                free(type->names);
                type_deref(type->ref);
                break;
            case DECLARATOR_POINTER:
            case DECLARATOR_ARRAY:
            case DECLARATOR_VLA:
            case DECLARATOR_FLEXIBLE:
                type_deref(type->ref);
                break;
        }
    } else {
        switch (type->base) {
            case BASE_RECORD:
                record_deref(type->record);
                break;
            case BASE_ENUM:
                enum_deref(type->enum_);
                break;
            default:
                break;
        }
    }

    free(type);
}

bool type_is_indirection(type_t* type) {
    if (!type->is_declarator) {
        return false;
    }
    switch (type->declarator) {
        case DECLARATOR_POINTER:
        case DECLARATOR_ARRAY:
        case DECLARATOR_VLA:
        case DECLARATOR_FLEXIBLE:
            return true;
        default:
            break;
    }
    return false;
}

void type_print(type_t* type) {
    if (type->is_declarator) {
        type_print(type->ref);
        switch (type->declarator) {
            case DECLARATOR_POINTER:
                fputs("*", stdout);
                if (type->is_const) fputs(" const", stdout);
                if (type->is_volatile) fputs(" volatile", stdout);
                if (type->is_restrict) fputs(" restrict", stdout);
                return;
            case DECLARATOR_FUNCTION:
                fputs("(", stdout);
                for (uint32_t i = 0; i < type->count; ++i) {
                    if (i != 0) fputs(", ", stdout);
                    type_print(type->args[i]);
                }
                if (type->is_variadic) fputs(", ...", stdout);
                fputs(")", stdout);
                return;
            case DECLARATOR_ARRAY:
                printf("[%u]", type->count);
                return;
            case DECLARATOR_VLA:
                fputs("[<vla>]", stdout);
                return;
            case DECLARATOR_FLEXIBLE:
                fputs("[]", stdout);
                return;
        }
        // unreachable
        return;
    }

    switch (type->base) {
        case BASE_VOID: fputs("void", stdout); break;
        case BASE_BOOL: fputs("bool", stdout); break;
        case BASE_CHAR: fputs("char", stdout); break;
        case BASE_SIGNED_CHAR: fputs("signed char", stdout); break;
        case BASE_UNSIGNED_CHAR: fputs("unsigned char", stdout); break;
        case BASE_SIGNED_SHORT: fputs("short", stdout); break;
        case BASE_UNSIGNED_SHORT: fputs("unsigned short", stdout); break;
        case BASE_SIGNED_INT: fputs("int", stdout); break;
        case BASE_UNSIGNED_INT: fputs("unsigned int", stdout); break;
        case BASE_SIGNED_LONG: fputs("long", stdout); break;
        case BASE_UNSIGNED_LONG: fputs("unsigned long", stdout); break;
        case BASE_SIGNED_LONG_LONG: fputs("long long", stdout); break;
        case BASE_UNSIGNED_LONG_LONG: fputs("unsigned long long", stdout); break;
        case BASE_FLOAT: fputs("float", stdout); break;
        case BASE_DOUBLE: fputs("double", stdout); break;
        case BASE_LONG_DOUBLE: fputs("long double", stdout); break;
        case BASE_RECORD:
            fputs(type->record->is_struct ? "struct " : "union ", stdout);
            fputs(type->record->tag ? type->record->tag->value->bytes : "<anon>", stdout);
            break;
        case BASE_ENUM:
            fputs("enum ", stdout);
            fputs("(TODO type_print enum name)", stdout);
            //fputs(type->enum_->name->bytes, stdout);
            break;
    }
}

bool type_matches_base(type_t* type, base_t base) {
    return !type->is_declarator && type->base == base;
}

bool type_is_arithmetic(type_t* type) {
    if (!type_is_base(type))
        return false;
    switch (type->base) {
        case BASE_BOOL:
        case BASE_CHAR:
        case BASE_SIGNED_CHAR:
        case BASE_UNSIGNED_CHAR:
        case BASE_SIGNED_SHORT:
        case BASE_UNSIGNED_SHORT:
        case BASE_SIGNED_INT:
        case BASE_UNSIGNED_INT:
        case BASE_SIGNED_LONG:
        case BASE_UNSIGNED_LONG:
        case BASE_FLOAT:
        //case BASE_ENUM: // TODO is enum considered arithmetic? or does it get implicitly cast to int first?
        case BASE_SIGNED_LONG_LONG:
        case BASE_UNSIGNED_LONG_LONG:
        case BASE_DOUBLE:
        case BASE_LONG_DOUBLE:
            return true;
        default: break;
    }
    return false;
}

bool type_is_integer(type_t* type) {
    if (!type_is_base(type))
        return false;
    switch (type->base) {
        case BASE_BOOL:
        case BASE_CHAR:
        case BASE_SIGNED_CHAR:
        case BASE_UNSIGNED_CHAR:
        case BASE_SIGNED_SHORT:
        case BASE_UNSIGNED_SHORT:
        case BASE_SIGNED_INT:
        case BASE_UNSIGNED_INT:
        case BASE_SIGNED_LONG:
        case BASE_UNSIGNED_LONG:
        //case BASE_ENUM: // TODO as above, is enum considered integer? if yes, then it should also be considered arithmetic
        case BASE_SIGNED_LONG_LONG:
        case BASE_UNSIGNED_LONG_LONG:
            return true;
        default: break;
    }
    return false;
}

int type_integer_rank(type_t* type) {
    if (!type_is_base(type))
        fatal("Internal error: non-base type does not have an integer rank.");
    switch (type->base) {
        case BASE_BOOL:
            return 1;
        case BASE_CHAR:
        case BASE_SIGNED_CHAR:
        case BASE_UNSIGNED_CHAR:
            return 2;
        case BASE_SIGNED_SHORT:
        case BASE_UNSIGNED_SHORT:
            return 3;
        case BASE_SIGNED_INT:
        case BASE_UNSIGNED_INT:
        //case BASE_ENUM: // TODO as above, is enum considered integer? if yes, then it should also be considered arithmetic
            return 4;
        case BASE_SIGNED_LONG:
        case BASE_UNSIGNED_LONG:
        case BASE_SIGNED_LONG_LONG:
            return 5;
        default: break;
    }
    fatal("Internal error: non-integer type does not have an integer rank.");
}

size_t base_size(base_t base) {
    switch (base) {
        case BASE_VOID: // fallthrough, GNU extension; -Wpointer-arith checked during parse
        case BASE_BOOL:
        case BASE_CHAR:
        case BASE_SIGNED_CHAR:
        case BASE_UNSIGNED_CHAR:
            return 1;
        case BASE_SIGNED_SHORT:
        case BASE_UNSIGNED_SHORT:
            return 2;
        case BASE_SIGNED_INT:
        case BASE_UNSIGNED_INT:
        case BASE_SIGNED_LONG:
        case BASE_UNSIGNED_LONG:
        case BASE_FLOAT:
        case BASE_ENUM:
            return 4;
        case BASE_SIGNED_LONG_LONG:
        case BASE_UNSIGNED_LONG_LONG:
        case BASE_DOUBLE:
        case BASE_LONG_DOUBLE:
            return 8;
        case BASE_RECORD:
            // should be handled in type_size()
            fatal("Internal error: cannot take the base size of a record.");
            break;
    }
    fatal("Internal error: end of base_size() unreachable.");
}

base_t base_unsigned_of_signed(base_t base) {
    switch (base) {
        case BASE_CHAR:
        case BASE_SIGNED_CHAR:
            return BASE_UNSIGNED_CHAR;
        case BASE_SIGNED_SHORT:
            return BASE_UNSIGNED_SHORT;
        case BASE_SIGNED_INT:
            return BASE_UNSIGNED_INT;
        case BASE_SIGNED_LONG:
            return BASE_UNSIGNED_LONG;
        case BASE_SIGNED_LONG_LONG:
            return BASE_UNSIGNED_LONG_LONG;
        default:
            break;
    }
    fatal("Internal error: Expected a signed integer base to convert to unsigned.");
}

size_t type_size(type_t* type) {
    if (type_is_base(type)) {
        if (type->base == BASE_RECORD)
            return record_size(type->record);
        return base_size(type->base);
    }

    switch (type->declarator) {
        case DECLARATOR_POINTER:
            return 4;
        case DECLARATOR_FUNCTION:
            fatal("Internal error: cannot take the size of a function.");
            break;
        case DECLARATOR_ARRAY:
            return type->count * type_size(type->ref);
            break;
        case DECLARATOR_VLA:
            // sizeof() on a VLA is runtime code inserted in parse_sizeof().
            fatal("Internal error: cannot take the compile-time size of a variable-length array.");
            break;
        case DECLARATOR_FLEXIBLE:
            fatal("Internal error: cannot take the size of an array of indeterminate size.");
            break;
    }

    fatal("Internal error: end of type_size() unreachable.");
}

size_t type_alignment(type_t* type) {
    if (type_is_array(type)) {
        return type_alignment(type_pointed_to(type));
    }
    size_t size = type_size(type);

    // TODO we'll need to be a bit smarter about this since users can specify
    // greater alignment. Probably the simplest thing is if we have a manual
    // alignment, use that; otherwise if we're a record, use its alignment;
    // otherwise use the alignment of the integer/float (where this basically
    // works.) Not bothering with any of this now.
    if (size > 4)
        return 4;

    assert(size != 3); // internal error, nothing has a size of 3 (except for an array, checked above)
    return size;
}

bool type_is_int(type_t* type) {
    if (type->is_declarator)
        return false;
    return type->base == BASE_SIGNED_INT ||
           type->base == BASE_UNSIGNED_INT;
}

bool type_is_long_long(type_t* type) {
    if (type->is_declarator)
        return false;
    return type->base == BASE_SIGNED_LONG_LONG ||
           type->base == BASE_UNSIGNED_LONG_LONG;
}

bool type_is_signed_integer(type_t* type) {
    if (type->is_declarator)
        return false;
    switch (type->base) {
        case BASE_CHAR:
        case BASE_SIGNED_CHAR:
        case BASE_SIGNED_SHORT:
        case BASE_SIGNED_INT:
        case BASE_SIGNED_LONG:
        case BASE_SIGNED_LONG_LONG:
            return true;
        default:
            break;
    }
    return false;
}

bool type_equal(type_t* left, type_t* right) {
    if (left->is_declarator != right->is_declarator ||
            left->is_const != right->is_const ||
            left->is_volatile != right->is_volatile)
        return false;

    // declarators
    if (left->is_declarator) {
        if (left->declarator != right->declarator)
            return false;

        switch (left->declarator) {
            case DECLARATOR_POINTER:
                if (left->is_restrict != right->is_restrict)
                    return false;
                break;
            case DECLARATOR_ARRAY:
                if (left->count != right->count)
                    return false;
                break;
            case DECLARATOR_FUNCTION:
                if (left->is_variadic != right->is_variadic ||
                        left->count != right->count)
                    return false;
                for (uint32_t i = 0; i < left->count; ++i)
                    if (!type_equal(left->args[i], right->args[i]))
                        return false;
                break;
            default:
                break;
        }

        return type_equal(left->ref, right->ref);
    }

    // bases
    if (left->base != right->base) {
        return false;
    }
    switch (left->base) {
        case BASE_RECORD:
            if (left->record != right->record)
                return false;
            break;
        case BASE_ENUM:
            if (left->enum_ != right->enum_)
                return false;
            break;
        default:
            break;
    }
    return true;
}

bool type_is_callable(type_t* type) {
    if (!type_is_declarator(type))
        return false;
    if (type->declarator == DECLARATOR_FUNCTION)
        return true;
    // TODO are arrays callable? for now we assume they decay to pointers and
    // are callable
    return type_is_function(type->ref);
}

bool type_is_array(type_t* type) {
    if (!type_is_declarator(type))
        return false;
    switch (type->declarator) {
        case DECLARATOR_ARRAY:
        case DECLARATOR_VLA:
        case DECLARATOR_FLEXIBLE:
            return true;
        default:
            break;
    }
    return false;
}

bool type_is_function(type_t* type) {
    return type_is_declarator(type) && type->declarator == DECLARATOR_FUNCTION;
}

bool type_is_passed_indirectly(type_t* type) {
    if (type_is_declarator(type)) {
        if (type->declarator == DECLARATOR_FUNCTION) {
            fatal("Internal error: functions cannot be passed by value.");
        }
        // Note: Arrays are not passed indirectly because they are not passed
        // at all; they decay to pointers before being passed.
        return false;
    }
    switch (type->base) {
        case BASE_DOUBLE:
        case BASE_LONG_DOUBLE:
        case BASE_SIGNED_LONG_LONG:
        case BASE_UNSIGNED_LONG_LONG:
            return true;
        case BASE_RECORD:
            return record_size(type->record) > 4;
        default:
            break;
    }
    return false;
}

type_t* type_decay(type_t* type) {
    if (!type_is_declarator(type))
        return type_ref(type);
    switch (type->declarator) {
        case DECLARATOR_POINTER:
        case DECLARATOR_FUNCTION:
            return type_ref(type);
        case DECLARATOR_ARRAY:
        case DECLARATOR_VLA:
        case DECLARATOR_FLEXIBLE:
            break;
    }
    return type_new_pointer(type->ref, type->is_const, type->is_volatile,
            type->is_restrict);
}

type_t* type_pointed_to(type_t* type) {
    if (!type_is_indirection(type))
        fatal("Internal error: cannot call type_pointed_to() on a non-pointer");
    return type->ref;
}
