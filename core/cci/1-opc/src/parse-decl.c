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

#include "parse-decl.h"

#include <stdlib.h>
#include <string.h>

#include "compile.h"
#include "field.h"
#include "global.h"
#include "lexer.h"
#include "locals.h"
#include "parse-expr.h"
#include "parse-stmt.h"
#include "record.h"
#include "types.h"

// function generation
int function_frame_size;
bool inside_function;
global_t* current_function;

void parse_decl_init(void) {
}

void parse_decl_destroy(void) {
}



/*
 * Declaration Specifiers
 *
 * A declaration specifier list is a bit "fun" to parse because the specifiers
 * can come in any order. For example you can write:
 *
 *     long unsigned int typedef long x;
 *
 * That's a totally valid typedef that maps `x` to `unsigned long long`, and it
 * can appear almost anywhere, even nested deep within a function. Struct,
 * union, enum and typedef names can be mixed into the specifier list at any
 * position as well.
 *
 * (GCC under `-Wextra` claims that not having storage class specifiers first
 * is obsolescent. I can't actually find that in any of the specs, and even if
 * it is, compilers still have to support them in any order for backwards
 * compatibility.)
 *
 * Despite this, the compiler still has to ensure that no specifiers are
 * repeated (except for `long`, which can be repeated at most once), and that
 * the combination of specifiers is valid.
 *
 * I suppose we didn't really need to do all these checks in the opC compiler
 * but I did anyway, at least for those keywords we don't ignore. I was more
 * concerned with getting the implementation correct and I thought I might make
 * mistakes if I tried to cut corners.
 *
 * In opC, we don't allow typedef, struct, union, or enum declarations outside
 * of global scope.
 */

/*
 * Storage class specifiers
 *
 * At most one storage class specifier is allowed to appear in a declaration
 * specifier list.
 */

static bool try_parse_storage_class_specifier(storage_t* out_storage,
        const char* keyword, storage_t value)
{
    if (lexer_accept(keyword)) {
        if (*out_storage != STORAGE_DEFAULT) {
            fatal_2("Redundant storage class specifier: ", keyword);
        }
        *out_storage = value;
        return true;
    }
    return false;
}

static bool try_parse_storage_class_specifiers(storage_t* out_storage) {

    // we care about these
    if (try_parse_storage_class_specifier(out_storage, "typedef", STORAGE_TYPEDEF)) {return true;}
    if (try_parse_storage_class_specifier(out_storage, "extern", STORAGE_EXTERN)) {return true;}
    if (try_parse_storage_class_specifier(out_storage, "static", STORAGE_STATIC)) {return true;}

    // these are ignored
    if (lexer_accept("auto")) {return true;}
    if (lexer_accept("register")) {return true;}
    if (lexer_accept("_Thread_local")) {return true;}
    if (lexer_accept("constexpr")) {return true;}

    return false;
}

/*
 * Type Qualifiers
 *
 * These are all ignored. We don't bother to check for duplicates.
 */
static bool try_parse_type_qualifiers(void) {
    if (lexer_accept("const")) {return true;}
    if (lexer_accept("restrict")) {return true;}
    if (lexer_accept("volatile")) {return true;}
    return false;
}

/*
 * Function Specifiers
 *
 * These are all ignored.
 */
static bool try_parse_function_specifiers(void) {
    if (lexer_accept("inline")) {return true;}
    if (lexer_accept("_Noreturn")) {return true;}
    return false;
}

/*
 * Type Specifiers
 */

// type specifier keywords
#define TYPE_SPECIFIER_VOID         (1 << 0)
#define TYPE_SPECIFIER_CHAR         (1 << 1)
#define TYPE_SPECIFIER_SHORT        (1 << 2)
#define TYPE_SPECIFIER_INT          (1 << 3)
#define TYPE_SPECIFIER_LONG         (1 << 4)
//#define TYPE_SPECIFIER_LONG_LONG    (1 << 5)
#define TYPE_SPECIFIER_SIGNED       (1 << 6)
#define TYPE_SPECIFIER_UNSIGNED     (1 << 7)
//#define TYPE_SPECIFIER_RECORD       (1 << 8)
//#define TYPE_SPECIFIER_TYPEDEF      (1 << 9)

static bool try_parse_type_specifier(int* type_specifiers,
        const char* keyword, int mask)
{
    if (lexer_accept(keyword)) {
        if (*type_specifiers & mask) {
            fatal_2("Redundant type specifier: ", keyword);
        }
        *type_specifiers = (*type_specifiers | mask);
        return true;
    }
    return false;
}

static void parse_enum(void) {

    // `enum` must be followed by a name, but we ignore it.
    if (lexer_type != lexer_type_alphanumeric) {
        fatal("`enum` must be followed by a name.");
    }
    lexer_consume();

    // check for `{`, if not we're done.
    if (!lexer_accept("{")) {
        return;
    }

    // we can only define enums globally
    if (inside_function) {
        fatal("Enums cannot be defined inside functions in opC.");
    }

    int value = 0;
    while (1) {
        // collect the name
        if (lexer_type != lexer_type_alphanumeric) {
            fatal("Expected an enum value");
        }
        char* name = lexer_take();

        // collect the optional value
        if (lexer_accept("=")) {
            type_t* type;
            if (!try_parse_constant_expression(&type, &value)) {
                fatal_2("Expected a constant expression after `=` for enum value: ", name);
            }
            type_delete(type);
        }

        // output it
        compile_enum_value(name, value);
        global_add(global_new_variable(type_new_base(BASE_SIGNED_INT), name));
        value = (value + 1);

        // see if we're done
        if (!lexer_accept(",")) {
            break;
        }
        if (lexer_is("}")) {
            break;
        }
    }

    lexer_expect("}", "Expected `,` or `}` after enum value");
}

static record_t* parse_record(bool is_struct) {
    record_t* record = NULL;

    // `struct` and `union` are allowed to be anonymous.
    if (lexer_type != lexer_type_alphanumeric) {
        record = record_new(strdup_checked(""));
        types_add_anonymous_record(record);
    }

    // A named struct might already exist.
    if (lexer_type == lexer_type_alphanumeric) {
        if (is_struct) {
            record = types_find_struct(lexer_token);
        }
        if (!is_struct) {
            record = types_find_union(lexer_token);
        }
        if (record != NULL) {
            lexer_consume();
        }

        // If not, create it
        if (record == NULL) {
            if (inside_function) {
                fatal("Structs and unions cannot be declared inside functions in opC.");
            }
            record = record_new(lexer_take());
            if (is_struct) {
                types_add_struct(record);
            }
            if (!is_struct) {
                types_add_union(record);
            }
        }
    }

    // Check for `{`. If not, we're done.
    if (!lexer_accept("{")) {
        if (0 == strcmp(record_name(record), "")) {
            fatal("Expected `{` or a name after `struct` or `union`.");
        }
        return record;
    }

    // We're defining the fields of this record. Check for errors
    if (inside_function) {
        fatal("Structs and unions cannot be defined inside functions in opC.");
    }
    if (record_fields(record)) {
        fatal("This struct or union is already defined.");
    }

    // Parse the fields
    field_t* fields = NULL;
    size_t offset = 0;
    while (!lexer_accept("}")) {

        // Parse the declaration specifiers
        type_t* base_type;
        if (!try_parse_declaration_specifiers(&base_type, NULL)) {
            fatal("Expected a struct or union field declaration.");
        }

        // Parse the declarator list
        type_t* type;
        char* name;
        if (!try_parse_declarator(base_type, &type, &name)) {
            fatal("Expected a declarator for this struct or union field declaration.");
        }

        // check for a bitfield declaration
        bool bitfield = lexer_accept(":");
        if (bitfield) {

            // parse and ignore the bitfield width
            type_t* width_type;
            int width;
            if (!try_parse_constant_expression(&width_type, &width)) {
                fatal("Expected bitfield width after `:`.");
            }
            type_delete(width_type);

            if (name == NULL) {
                // An unnamed bitfield is meant to declare padding. We ignore
                // it.
                type_delete(type);
                type_delete(base_type);
                lexer_expect(";", "Expected `;` after unnamed bitfield declaration.");
                continue;
            }
        }

        // We support C11 anonymous struct and union members. All other members
        // must be named.
        if (name == NULL) {
            if (type_is_record(type)) {
                name = strdup_checked("");
            }
        }
        if (name == NULL) {
            fatal("This struct or union member must have a name.");
        }

        // TODO loop on commas for multiple declarators. For now we don't bother.
        if (lexer_is(",")) {
            fatal("Multiple declarators with `,` are not yet implemented.");
        }
        type_delete(base_type);
        lexer_expect(";", "Expected `;` after struct or union field declaration.");

        // Align the field offset
        size_t align = type_alignment(type);
        if (align == 2) {
            offset = ((offset + 1) & ~1);
        }
        if (align == 4) {
            offset = ((offset + 3) & ~3);
        }

        // Add the field to the linked list
        fields = field_new(name, type, offset, fields);

        // In a struct, each field comes after the previous. In a union, all
        // fields have offset 0.
        if (is_struct) {
            // Flexible array members haven't been fixed up yet so we skip
            // them. Fields get checked in record_set_fields().
            if (type_array_length(type) != TYPE_ARRAY_INDETERMINATE) {
                offset = field_end(fields);
            }
        }
    }
    if (fields == NULL) {
        fatal("Structs and unions must have at least one field.");
    }

    record_set_fields(record, fields, is_struct);
    return record;
}

static bool try_parse_type_specifiers(int* type_specifiers,
        const record_t** out_record,
        const type_t** out_typedef)
{
    // atomic is ignored.
    if (lexer_accept("_Atomic")) {return true;}

    // handle primitive specifiers
    if (try_parse_type_specifier(type_specifiers, "void", TYPE_SPECIFIER_VOID)) {return true;}
    if (try_parse_type_specifier(type_specifiers, "char", TYPE_SPECIFIER_CHAR)) {return true;}
    if (try_parse_type_specifier(type_specifiers, "short", TYPE_SPECIFIER_SHORT)) {return true;}
    if (try_parse_type_specifier(type_specifiers, "int", TYPE_SPECIFIER_INT)) {return true;}
    if (try_parse_type_specifier(type_specifiers, "signed", TYPE_SPECIFIER_SIGNED)) {return true;}
    if (try_parse_type_specifier(type_specifiers, "unsigned", TYPE_SPECIFIER_UNSIGNED)) {return true;}

    // handle long
    if (lexer_accept("long")) {
        if (*type_specifiers & TYPE_SPECIFIER_LONG) {
            fatal("`long long` is not supported in opC.");
        }
        /*
        if (*type_specifiers & TYPE_SPECIFIER_LONG_LONG) {
            fatal("`long long long` is invalid.");
        }
        if (*type_specifiers & TYPE_SPECIFIER_LONG) {
            *type_specifiers = (*type_specifiers & ~TYPE_SPECIFIER_LONG);
            *type_specifiers = (*type_specifiers | TYPE_SPECIFIER_LONG_LONG);
            return true;
        }
        */
        *type_specifiers = (*type_specifiers | TYPE_SPECIFIER_LONG);
        return true;
    }

    // handle enum
    if (lexer_accept("enum")) {
        parse_enum();
        // all enums are just aliases of int.
        if (*type_specifiers & TYPE_SPECIFIER_INT) {
            fatal("Redundant enum specifier");
        }
        *type_specifiers = (*type_specifiers | TYPE_SPECIFIER_INT);
        return true;
    }

    // handle struct and union
    bool is_struct = lexer_is("struct");
    if (is_struct | lexer_is("union")) {
        lexer_consume();
        if (*out_record != NULL) {
            fatal("Redundant struct/union specifier");
        }
        *out_record = parse_record(is_struct);
        return true;
    }

    // search for typedef (only if we haven't found any other type specifiers yet)
    if (*type_specifiers == 0) {
        if (*out_record == NULL) {
            if (*out_typedef == NULL) {
                *out_typedef = types_find_typedef(lexer_token);
                if (*out_typedef) {
                    lexer_consume();
                    return true;
                }
            }
        }
    }

    return false;
}

base_t convert_type_specifier(int type_specifiers) {

    // If we have another integer width, "int" is redundant.
    if (type_specifiers & TYPE_SPECIFIER_INT) {
        if ((type_specifiers & TYPE_SPECIFIER_SHORT) |
            ((type_specifiers & TYPE_SPECIFIER_LONG) /*|
            (type_specifiers & TYPE_SPECIFIER_LONG_LONG)*/))
        {
            type_specifiers = (type_specifiers & ~TYPE_SPECIFIER_INT);
        }
    }

    // If we have "long", convert it to "int".
    if (type_specifiers & TYPE_SPECIFIER_LONG) {
        type_specifiers = (type_specifiers & ~TYPE_SPECIFIER_LONG);
        type_specifiers = (type_specifiers | TYPE_SPECIFIER_INT);
    }

    // We check each supported type combination against our type specifier
    // bitmap (minus our simplifications above.) See C11 6.7.2.2

    if (type_specifiers == TYPE_SPECIFIER_VOID) {return BASE_VOID;}

    if (type_specifiers == (TYPE_SPECIFIER_UNSIGNED | TYPE_SPECIFIER_CHAR))
        {return BASE_UNSIGNED_CHAR;}
    if (type_specifiers == (TYPE_SPECIFIER_UNSIGNED | TYPE_SPECIFIER_SHORT))
        {return BASE_UNSIGNED_SHORT;}
    if ((type_specifiers == (TYPE_SPECIFIER_UNSIGNED | TYPE_SPECIFIER_INT)) |
            (type_specifiers == TYPE_SPECIFIER_UNSIGNED))
        {return BASE_UNSIGNED_INT;}
    //if (type_specifiers == (TYPE_SPECIFIER_UNSIGNED | TYPE_SPECIFIER_LONG_LONG))
    //    {return BASE_UNSIGNED_LONG_LONG;}

    if ((type_specifiers == TYPE_SPECIFIER_CHAR) |
            (type_specifiers == (TYPE_SPECIFIER_SIGNED | TYPE_SPECIFIER_CHAR)))
        {return BASE_SIGNED_CHAR;}
    if ((type_specifiers == TYPE_SPECIFIER_SHORT) |
            (type_specifiers == (TYPE_SPECIFIER_SIGNED | TYPE_SPECIFIER_SHORT)))
        {return BASE_SIGNED_SHORT;}
    if (((type_specifiers == TYPE_SPECIFIER_INT) |
            (type_specifiers == TYPE_SPECIFIER_SIGNED)) |
            (type_specifiers == (TYPE_SPECIFIER_SIGNED | TYPE_SPECIFIER_INT)))
        {return BASE_SIGNED_INT;}
    //if ((type_specifiers == TYPE_SPECIFIER_LONG_LONG) |
    //        (type_specifiers == (TYPE_SPECIFIER_SIGNED | TYPE_SPECIFIER_LONG_LONG)))
    //    {return BASE_SIGNED_LONG_LONG;}

    fatal("Unsupported combination of type specifiers.");
}

/*
 * Declaration Specifier List
 */

bool try_parse_declaration_specifiers(
        type_t** out_type,
        storage_t* /*nullable*/ out_storage)
{
    if (out_storage) {
        *out_storage = STORAGE_DEFAULT;
    }

    // We ignore type qualifiers and function specifiers, except that we still
    // need to track whether we found one during parsing because if we did, a
    // declaration is not optional. (We don't bother to check if any of these
    // specifiers are redundant.)
    bool optional = true;
    int type_specifiers = 0;
    const record_t* record = NULL;
    const type_t* typedef_type = NULL;

    // All declaration specifiers are alphanumeric so we can short-circuit with
    // an alphanumeric test.
    while (lexer_type == lexer_type_alphanumeric) {

        // We ignore all type qualifiers and function specifiers.
        if (try_parse_type_qualifiers() | try_parse_function_specifiers()) {
            optional = false;
            continue;
        }

        if (out_storage) {
            if (try_parse_storage_class_specifiers(out_storage)) {
                optional = false;
                continue;
            }
        }

        if (try_parse_type_specifiers(&type_specifiers, &record, &typedef_type)) {
            optional = false;
            continue;
        }
        break;
    }

    // see if we got a typedef
    if (typedef_type != NULL) {
        if (record != NULL) {
            fatal("Redundant type name and struct/union name in declaration specifier list.");
        }
        if (type_specifiers != 0) {
            fatal("Redundant type name and type specifiers in declaration specifier list.");
        }
        *out_type = type_clone(typedef_type);
        return true;
    }

    // see if we got a record
    if (record != NULL) {
        if (type_specifiers != 0) {
            fatal("Redundant struct/union and type specifiers in declaration specifier list.");
        }
        *out_type = type_new_record(record);
        return true;
    }

    // otherwise we may have gotten a primitive type
    if (type_specifiers != 0) {
        base_t base = convert_type_specifier(type_specifiers);
        if (base != BASE_INVALID) {
            *out_type = type_new_base(base);
            return true;
        }
    }

    if (!optional) {
        fatal("Expected a declaration specifier list.");
    }
    return false;

}



/*
 * Declarator List
 *
 * A declarator list is the part that comes after the declaration specifiers.
 * For example:
 *
 *     int long typedef *x, y[4], *(*z(void*))[8];
 *
 * In this case there are three named declarator lists: `*` with name `x`,
 * `[4]` with name `y`, and `*(*(void*))[8]` with name `z`.
 *
 * We don't parse function arguments as part of the declarator list in the opC
 * compiler because we don't support function pointers. Function arguments are
 * parsed separately in parse_function_declaration().
 */

static bool try_parse_declarator_impl(type_t* type, char** /*nullable*/ out_name);

static bool try_parse_direct_declarator(type_t* type, char** /*nullable*/ out_name) {
    bool found = false;

    // Parens before another direct declarator are a parenthesized declarator.
    if (lexer_accept("(")) {
        if (!try_parse_declarator_impl(type, out_name)) {
            fatal("Expected declarator after `(`");
        }
        lexer_expect(")", "Expected `)` after parenthesized declarator.");
        found = true;
    }

    // If out_name is NULL, this declarator is abstract. In that case a name
    // cannot be provided and the direct-declarator is optional.
    if (out_name != NULL) {

        // Check for a name
        if ((!found & (lexer_type == lexer_type_alphanumeric)) & (out_name != NULL)) {
            if (*out_name != NULL) {
                fatal_2("Redundant identifier in declarator: ", lexer_token);
            }
            // We should check that this isn't a keyword. For now we don't
            // bother. We should also check that this doesn't re-declare a name in
            // the same scope. We don't properly support typedef scope (and we don't
            // even really track variable scopes outside of the block parse
            // functions) so we don't bother with this either.
            *out_name = lexer_take();
            found = true;
        }

        if (!found) {
            return false;
        }
    }

    // Parens after another direct declarator are function arguments, but we
    // don't check that here because we don't support function pointers or a
    // real function type. The global declaration parser will expect to see '('
    // to parse the arguments itself.

    while (lexer_accept("[")) {
        if (type_is_array(type)) {
            fatal("Multi-dimensional arrays are not supported in opC.");
        }
        if (type_pointers(type) > 0) {
            fatal("Pointers to arrays are not supported in opC.");
        }

        // Support arrays of indeterminate size
        bool indeterminate = false;
        if (lexer_accept("]")) {
            type_set_array_length(type, TYPE_ARRAY_INDETERMINATE);
            indeterminate = true;
        }

        if (!indeterminate) {

            // We ignore type qualifiers.
            //while (try_parse_type_qualifiers()) {}

            // parse a constant expression for the array length
            type_t* length_type;
            int length;
            if (!try_parse_constant_expression(&length_type, &length)) {
                fatal("Expected array size.");
            }
            if (!lexer_accept("]")) {
                fatal("Expected `]` after array size.");
            }
            if (length < 0) {
                fatal("Array size cannot be negative.");
            }

            type_delete(length_type);
            type_set_array_length(type, length);
        }

        found = true;
    }

    return found;
}

static bool try_parse_pointer(type_t* type) {
    if (!lexer_accept("*")) {
        return false;
    }
    // We ignore type qualifiers.
    while (try_parse_type_qualifiers()) {}
    return true;
}

static bool try_parse_declarator_impl(type_t* type, char** /*nullable*/ out_name) {
    int pointers = 0;

    // Collect pointers. We apply them *after* parsing the direct declarator
    // because we parse types backwards and we want to detect unsupported
    // features such as pointers to arrays.
    while (try_parse_pointer(type)) {
        pointers = (pointers + 1);
    }

    if (!try_parse_direct_declarator(type, out_name)) {
        if (!pointers) {
            // pointers only are allowed in an abstract declarator, not in a
            // normal declarator (even if the name is optional)
            // TODO I'm sure this is wrong, function arguments are not abstract
            // and should be allowed to be pointers only
            /*
            if (out_name != NULL)) {
                fatal("Pointer declarator must be followed by a direct declarator.");
            }
            */
        }
    }

    while (pointers > 0) {
        pointers = (pointers - 1);
        type_increment_pointers(type);
    }

    return true;
}

bool try_parse_declarator(const type_t* base_type, type_t** out_type,
        char** /*nullable*/ out_name)
{
    *out_type = type_clone(base_type);
    if (out_name) {
        *out_name = NULL;
    }
    if (!try_parse_declarator_impl(*out_type, out_name)) {
        type_delete(*out_type);
        return false;
    }
    return true;
}

/*
void parse_declarator(const type_t* base_type,
        type_t** out_type, char** out_name)
{
    if (!try_parse_declarator(base_type, out_type, out_name)) {
        fatal("Expected a declarator (i.e. a name, `*`, `[`, etc.)");
    }
}
*/

bool try_parse_declaration(
        storage_t* /*nullable*/ out_storage,
        type_t** out_type,
        char** /*nullable*/ out_name)
{
    type_t* base_type;
    if (!try_parse_declaration_specifiers(&base_type, out_storage)) {
        return false;
    }

    if (!try_parse_declarator(base_type, out_type, out_name)) {
        *out_type = base_type;
        return true;
    }
    type_delete(base_type);
    return true;
}



/*
 * Globals
 */

// Takes ownership of the given type
static void parse_local_declaration(type_t* type, char* /*nullable*/ name) {
    if (name == NULL) {
        // We ignore useless variable declarations.
        type_delete(type);
    }

    //printf("defining new variable %s\n",name);
    locals_add(name, type);

    if (lexer_accept(";")) {
        // No initializer.
        return;
    }
    type = type_clone(type);

    if (!lexer_accept("=")) {
        fatal("Expected `;` or `=` after local variable declaration.");
    }

    // The variable has an initializer. Parse it, then generate an assignment
    // operation.

    // compile and push the variable
    type_t* var_type = compile_load_variable(name);
    compile_push(0);

    // TODO for now ignoring var_type
    type_delete(var_type);

    // get the assignment expression
    // TODO need to rewrite initializer code to support arrays, structs
    type_t* expr_type = parse_assignment_expression();

    // pop the destination
    compile_pop(1);

    // compile an assignment
    type_set_lvalue(type, true);
    type_delete(compile_assign(type, expr_type));
}

bool try_parse_local_declaration(void) {
    // TODO we need to do this separately to handle commas, like parse_global()
    type_t* type;
    char* name;
    if (try_parse_declaration(NULL, &type, &name)) {
        parse_local_declaration(type, name);
        return true;
    }
    return false;
}

// Parses a function declaration (and definition, if provided.)
static void parse_function_declaration(type_t* return_type, char* name, storage_t storage) {
    inside_function = true;
    bool is_variadic = false;
    int arg_count = 0;
    int arg_capacity = 64;
    type_t** arg_types = malloc(arg_capacity * sizeof(type_t*));

    while (!lexer_accept(")")) {
        if (arg_count > 0) {
            lexer_expect(",", "Expected `,` or `)` after argument");
        }
        if (arg_count == arg_capacity) {
            // We could make this growable but there's no point.
            fatal("This function has too many parameters.");
        }

        // check for variadic arguments
        if (lexer_accept("...")) {
            if (arg_count == 0) {
                fatal("At least one non-variadic argument is required before `...`.");
            }
            lexer_expect(")", "Expected `)` after `...`");
            is_variadic = true;
            break;
        }

        // parse parameter
        type_t* arg_type;
        char* arg_name;
        if (!try_parse_declaration(NULL, &arg_type, &arg_name)) {
            fatal("Expected a function parameter declaration");
        }

        // check for (void) parameter list
        if (((arg_name == NULL) & (arg_count == 0)) & type_is_base(arg_type, BASE_VOID)) {
            if (lexer_accept(")")) {
                type_delete(arg_type);
                break;
            }
        }

        // array parameters are treated as pointers.
        arg_type = type_decay_array(arg_type);

        // If we don't have a name, for now we give it an anonymous name. We'll
        // still reserve stack space for it and move the argument into it. This
        // is simpler than trying to optimize away stack storage for unnamed
        // parameters.
        if (arg_name == NULL) {
            arg_name = strdup("");
        }

        // add variable
        locals_add(arg_name, type_clone(arg_type));
        *(arg_types + arg_count) = arg_type;
        arg_count = (arg_count + 1);
    }

    global_t* global = global_new_function(return_type, name, arg_count, arg_types);
    free(arg_types);
    if (is_variadic) {
        global_set_variadic(global, true);
    }
    global = global_add(global);
    current_function = global;

    if (!lexer_accept(";")) {

        // compile the function
        function_frame_size = 0;
        compile_function_open(global);
        parse_block();
        compile_function_close(global, function_frame_size, storage);

        // output any strings that were used in the function
        output_string_literals();
        compile_global_divider();
    }

    current_function = NULL;
    inside_function = false;
    locals_pop(0);
}

void parse_global(void) {
    storage_t storage;
    type_t* base_type;
    if (!try_parse_declaration_specifiers(&base_type, &storage)) {
        fatal("Expected a global declaration.");
    }

    while (1) {
        type_t* type;
        char* name;
        if (!try_parse_declarator(base_type, &type, &name)) {
            fatal("Expected a declarator for this global declaration.");
        }

        // A no-name declaration is not necessarily an error, for example it
        // could be an enum.
        if (name == NULL) {
            type_delete(type);
        }
        if (name != NULL) {

            // Check for a typedef
            if (storage == STORAGE_TYPEDEF) {
                types_add_typedef(name, type);
                lexer_expect(";", "Expected `;` at end of typedef");
                break;
            }

            // Check for a function
            if (lexer_accept("(")) {
                parse_function_declaration(type, name, storage);
                break;
            }

            // Otherwise it's a global variable declaration
            if (lexer_accept("=")) {
                fatal("Global variable initializer is not yet implemented");
            }
            if (storage != STORAGE_EXTERN) {
                compile_global_variable(type, name, storage);
            }
            global_add(global_new_variable(type, name));

        }

        // TODO check for a comma for multiple declarators, only allowed if none are functions
        if (lexer_is(",")) {
            fatal("Multiple declarators with `,` are not yet implemented.");
        }
        lexer_expect(";", "Expected `;` at end of global variable declaration.");
        break;
    }

    type_delete(base_type);
}
