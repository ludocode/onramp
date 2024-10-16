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
#include "record.h"
#include "enum.h"
#include "scope.h"

static type_t* type_clone(type_t* type) {
    type_t* clone = malloc(sizeof(type_t));
    memcpy(clone, type, sizeof(*clone));
    clone->refcount = 1;

    // If it's a base, ref the record or enum
    if (!clone->is_declarator) {
        if (clone->base == BASE_ENUM)
            enum_ref(clone->enum_);
        return clone;
    }

    // Ref the pointed-to type or return type
    type_ref(clone->ref);

    // Copy and ref the arguments and scope
    if (clone->declarator == DECLARATOR_FUNCTION) {
        type_t** new_args = malloc(sizeof(type_t*) * clone->count);
        token_t** new_names = malloc(sizeof(token_t*) * clone->count);
        for (size_t i = 0; i < clone->count; ++i) {
            new_args[i] = type_ref(clone->args[i]);
            new_names[i] = token_ref(clone->names[i]);
        }
        clone->args = new_args;
        clone->names = new_names;
        scope_ref(clone->scope);
    }
    return clone;
}

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

type_t* type_new_declarator(declarator_t declarator) {
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
    type->record = record;
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
    type->ref = return_type;//type_ref(return_type); // TODO this needs to be re-enabled
    type->count = args_count;
    type->is_variadic = is_variadic;

    // TODO memdup
    // TODO actually instead of copying we should take ownership and realloc() to the correct size

    type->args = malloc(sizeof(type_t*) * args_count);
    if (type->args == NULL && args_count > 0) {
        fatal("Out of memory.");
    }
    memcpy(type->args, arg_types, sizeof(type_t*) * args_count);

    type->names = malloc(sizeof(token_t*) * args_count);
    if (type->names == NULL && args_count > 0) {
        fatal("Out of memory.");
    }
    memcpy(type->names, arg_names, sizeof(token_t*) * args_count);

    return type;
}

type_t* type_qualify(type_t* type, bool is_const, bool is_volatile) {
    if (!is_const && !is_volatile)
        return type;
    type_t* clone = type_clone(type);
    type_deref(type);
    clone->is_const |= is_const;
    clone->is_volatile |= is_volatile;
    return clone;
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
                if (type->scope) {
                    scope_deref(type->scope);
                }
                break;
            case DECLARATOR_POINTER:
            case DECLARATOR_ARRAY:
            case DECLARATOR_VLA:
            case DECLARATOR_INDETERMINATE:
                type_deref(type->ref);
                break;
        }
    } else {
        switch (type->base) {
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
        case DECLARATOR_INDETERMINATE:
            return true;
        default:
            break;
    }
    return false;
}

static void type_base_print(const type_t* type) {
    if (type->is_const) fputs("const ", stdout);
    if (type->is_volatile) fputs("volatile ", stdout);

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
            if (type->enum_->tag) {
                fputs(type->enum_->tag->value->bytes, stdout);
            } else {
                fputs("<anonymous>", stdout);
            }
            break;
        case BASE_VA_LIST: fputs("va_list", stdout); break;
    }
}

static void type_print_prefix(type_t* type) {
    if (type == NULL) {
        fputs("(null)", stdout);
        return;
    }

    if (!type->is_declarator) {
        type_base_print(type);
        return;
    }

    type_print_prefix(type->ref);

    if (type->declarator == DECLARATOR_POINTER) {
        type_t* ref = type->ref;
        if (ref->is_declarator && ref->declarator != DECLARATOR_POINTER)
            fputc('(', stdout);
        fputc('*', stdout);
        if (type->is_const) fputs(" const", stdout);
        if (type->is_volatile) fputs(" volatile", stdout);
        if (type->is_restrict) fputs(" restrict", stdout);
    }
}

static void type_print_suffix(type_t* type) {
    if (!type->is_declarator) {
        return;
    }

    switch (type->declarator) {
        case DECLARATOR_POINTER: {
            type_t* ref = type->ref;
            if (ref->is_declarator && ref->declarator != DECLARATOR_POINTER)
                fputc(')', stdout);
            break;
        }

        case DECLARATOR_FUNCTION:
            fputs("(", stdout);
            for (uint32_t i = 0; i < type->count; ++i) {
                if (i != 0) fputs(", ", stdout);
                type_print(type->args[i]);
            }
            if (type->is_variadic)
                fputs(", ...", stdout);
            fputs(")", stdout);
            break;

        case DECLARATOR_ARRAY:
            printf("[%u]", type->count);
            break;

        case DECLARATOR_VLA:
            fputs("[<vla>]", stdout);
            break;

        case DECLARATOR_INDETERMINATE:
            fputs("[]", stdout);
            break;
    }

    type_print_suffix(type->ref);
}

void type_print(type_t* type) {
    type_print_prefix(type);
    type_print_suffix(type);
}

void type_print_words(const type_t* type) {
    if (!type->is_declarator) {
        type_base_print(type);
        return;
    }

    switch (type->declarator) {
        case DECLARATOR_POINTER:
            if (type->is_const) fputs("const ", stdout);
            if (type->is_volatile) fputs("volatile ", stdout);
            if (type->is_restrict) fputs("restrict ", stdout);
            fputs("pointer to ", stdout);
            break;

        case DECLARATOR_FUNCTION:
            fputs("function taking ", stdout);
            if (type->count == 0) {
                fputs("no arguments ", stdout);
            } else {
                fputs("(", stdout);
                for (uint32_t i = 0; i < type->count; ++i) {
                    if (i != 0) fputs(", ", stdout);
                    type_print_words(type->args[i]);
                }
                if (type->is_variadic)
                    fputs(", and variadic arguments", stdout);
                fputs(") ", stdout);
            }
            fputs("and returning ", stdout);
            break;

        case DECLARATOR_ARRAY:
            printf("array of %u elements of ", type->count);
            break;

        case DECLARATOR_VLA:
            fputs("variable-length array of ", stdout);
            break;

        case DECLARATOR_INDETERMINATE:
            fputs("indeterminate array of ", stdout);
            break;
    }
    type_print_words(type->ref);
}

bool type_matches_base(type_t* type, base_t base) {
    return !type->is_declarator && type->base == base;
}

bool type_matches_declarator(type_t* type, declarator_t declarator) {
    return type->is_declarator && type->declarator == declarator;
}

bool type_is_arithmetic(type_t* type) {
    if (!type_is_base(type))
        return false;
    switch (type->base) {
        // C17 6.2.5.18: integer types and floating types are collectively
        // called arithmetic types.
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
        case BASE_ENUM:
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
        case BASE_BOOL: // C17 6.2.5.6: _Bool is an unsigned integer type.
        case BASE_CHAR:
        case BASE_SIGNED_CHAR:
        case BASE_UNSIGNED_CHAR:
        case BASE_SIGNED_SHORT:
        case BASE_UNSIGNED_SHORT:
        case BASE_SIGNED_INT:
        case BASE_UNSIGNED_INT:
        case BASE_SIGNED_LONG:
        case BASE_UNSIGNED_LONG:
        case BASE_ENUM: // C17 6.2.5.17: enumerated types are integer types.
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
            return 5;
        case BASE_SIGNED_LONG_LONG:
        case BASE_UNSIGNED_LONG_LONG:
            return 6;
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
        case BASE_VA_LIST:
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
        case DECLARATOR_INDETERMINATE:
            fatal("Internal error: cannot take the size of an array of indeterminate size.");
            break;
    }

    fatal("Internal error: end of type_size() unreachable.");
}

size_t type_alignment(type_t* type) {
    if (type_is_declarator(type)) {
        switch (type->declarator) {
            case DECLARATOR_POINTER:
                return 4;
            case DECLARATOR_ARRAY:
            case DECLARATOR_VLA:
            case DECLARATOR_INDETERMINATE:
                return type_alignment(type_pointed_to(type));
            case DECLARATOR_FUNCTION:
                fatal("Internal error: cannot compute alignment of function type.");
        }
    }

    if (type->base == BASE_RECORD) {
        return type->record->alignment;
    }

    // TODO we should check for manual alignment here. _Alignas is not
    // supported yet.

    size_t size = type_size(type);
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

bool type_quals_match(type_t* left, type_t* right) {
    if (left->is_const != right->is_const)
        return false;
    if (left->is_volatile != right->is_volatile)
        return false;
    if (left->is_declarator && right->is_declarator &&
            left->is_restrict != right->is_restrict)
        return false;
    return true;
}

bool type_equal(type_t* left, type_t* right) {
    if (left == right)
        return true;
    return type_quals_match(left, right) && type_equal_unqual(left, right);
}

bool type_compatible(type_t* left, type_t* right) {
    if (left == right)
        return true;
    return type_quals_match(left, right) && type_compatible_unqual(left, right);
}

// checks if the declarators match (not counting qualifiers.)
static bool type_declarator_equal(type_t* left, type_t* right) {
    assert(left->is_declarator);
    assert(right->is_declarator);
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

    return true;
}

bool type_equal_unqual(type_t* left, type_t* right) {

    // declarators
    if (left->is_declarator != right->is_declarator)
        return false;
    if (left->is_declarator) {
        return type_declarator_equal(left, right) &&
            type_equal(left->ref, right->ref);
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

bool type_compatible_unqual(type_t* left, type_t* right) {

    // declarators
    if (left->is_declarator != right->is_declarator)
        return false;
    if (left->is_declarator) {
        if (!type_declarator_equal(left, right))
            return false;

        // void* is compatible with any other pointer type.
        if (type_matches_base(left->ref, BASE_VOID))
            return true;
        if (type_matches_base(right->ref, BASE_VOID))
            return true;

        return type_compatible(left->ref, right->ref);
    }

    // bases

    // enums are compatible with an implementation-defined integer type; in our
    // case, this is int.
    if (left->base == BASE_ENUM) {
        if (right->base == BASE_ENUM)
            return left->enum_ == right->enum_; // enums must match
        return type_matches_base(right, BASE_SIGNED_INT);
    }
    if (right->base == BASE_ENUM) {
        return type_matches_base(left, BASE_SIGNED_INT);
    }

    // otherwise, the bases must match
    if (left->base != right->base)
        return false;

    // and if it's a record, the records must match.
    if (left->base == BASE_RECORD && left->record != right->record)
        return false;
    return true;
}

bool type_is_array(type_t* type) {
    if (!type_is_declarator(type))
        return false;
    switch (type->declarator) {
        case DECLARATOR_ARRAY:
        case DECLARATOR_VLA:
        case DECLARATOR_INDETERMINATE:
            return true;
        default:
            break;
    }
    return false;
}

bool type_is_function(type_t* type) {
    return type_is_declarator(type) && type->declarator == DECLARATOR_FUNCTION;
}

bool type_is_pointer(type_t* type) {
    return type_is_declarator(type) && type->declarator == DECLARATOR_POINTER;
}

bool type_is_passed_indirectly(type_t* type) {
    if (type_is_declarator(type)) {
        if (type->declarator == DECLARATOR_FUNCTION) {
            // TODO see if this is still needed. We could generate functions as
            // pointers the same as arrays.
            fatal("Internal error: functions cannot be passed by value.");
        }
        // Note: Arrays are not passed indirectly because when arrays are
        // passed, we are actually passing a pointer to the first element.
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

type_t* type_pointed_to(type_t* type) {
    if (!type_is_indirection(type))
        fatal("Internal error: cannot call type_pointed_to() on a non-pointer");
    return type->ref;
}

bool type_is_complete(type_t* type) {
    if (type_matches_base(type, BASE_RECORD)) {
        return type->record->is_defined;
    }
    if (type_is_declarator(type) && type->declarator == DECLARATOR_INDETERMINATE) {
        return false;
    }
    return true;
}

bool type_is_flexible_array(type_t* type) {
    if (type_matches_declarator(type, DECLARATOR_INDETERMINATE))
        return true;
    if (type_matches_declarator(type, DECLARATOR_ARRAY) && type->count == 0)
        return true;
    return false;
}

static void type_add_builtin(const char* cname, base_t base) {
    type_t* type = type_new_base(base);
    token_t* token = token_new_builtin(cname);
    scope_add_type(scope_global, NAMESPACE_TYPEDEF, token, type);
    token_deref(token);
    type_deref(type);
}

void type_create_builtins(void) {
    type_add_builtin("__builtin_va_list", BASE_VA_LIST);
}
