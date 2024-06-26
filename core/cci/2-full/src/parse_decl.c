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

#include "parse_decl.h"

#include <stdbool.h>
#include <stdlib.h>

#include "common.h"
#include "strings.h"
#include "type.h"
#include "scope.h"
#include "node.h"
#include "parse_stmt.h"
#include "parse_expr.h"
#include "lexer.h"
#include "token.h"
#include "options.h"
#include "generate.h"
#include "function.h"
#include "emit.h"
#include "symbol.h"

extern struct function_t* current_function;

void parse_decl_init(void) {
}

void parse_decl_destroy(void) {
}



/*
 * Specifiers
 */

/*
 * The specifier sequence of a declaration (the part that does not include the
 * declarators.)
 */
typedef struct specifiers_t {
    int storage_specifiers;
    int type_specifiers;
    int type_qualifiers;
    int function_specifiers;
    //int alignment_specifier; // value, not flags; 0 if not provided

    // none of these have strong references
    type_t* type;
    //record_t* record; // TODO probably don't need these, type_t* can cover it
    //enum_t* enum_;

    // this should have a strong reference
    //bool is_struct;
    //token_t* record_name;
} specifiers_t;

static void specifiers_init(specifiers_t* specifiers) {
    memset(specifiers, 0, sizeof(*specifiers));
}

static void specifiers_destroy(specifiers_t* specifiers) {
}

#define STORAGE_SPECIFIER_TYPEDEF        (1 << 0)
#define STORAGE_SPECIFIER_EXTERN         (1 << 1)
#define STORAGE_SPECIFIER_STATIC         (1 << 2)
#define STORAGE_SPECIFIER_THREAD_LOCAL   (1 << 3)
#define STORAGE_SPECIFIER_AUTO           (1 << 4)
#define STORAGE_SPECIFIER_REGISTER       (1 << 5)

#define TYPE_SPECIFIER_VOID         (1 << 0)
#define TYPE_SPECIFIER_CHAR         (1 << 1)
#define TYPE_SPECIFIER_SHORT        (1 << 2)
#define TYPE_SPECIFIER_INT          (1 << 3)
#define TYPE_SPECIFIER_LONG         (1 << 4)
#define TYPE_SPECIFIER_LONG_LONG    (1 << 5)
#define TYPE_SPECIFIER_SIGNED       (1 << 6)
#define TYPE_SPECIFIER_UNSIGNED     (1 << 7)
#define TYPE_SPECIFIER_RECORD       (1 << 8)
#define TYPE_SPECIFIER_ENUM         (1 << 9)
#define TYPE_SPECIFIER_TYPEDEF      (1 << 10)
#define TYPE_SPECIFIER_BOOL         (1 << 11)

#define TYPE_QUALIFIER_CONST        (1 << 0)
#define TYPE_QUALIFIER_VOLATILE     (1 << 1)
#define TYPE_QUALIFIER_RESTRICT     (1 << 2)
//#define TYPE_QUALIFIER_ATOMIC       (1 << 3)

#define FUNCTION_SPECIFIER_INLINE     (1 << 0)
#define FUNCTION_SPECIFIER_NORETURN   (1 << 1)

static bool try_parse_declaration_specifiers(specifiers_t* specifiers);
static type_t* specifiers_make_type(specifiers_t* specifiers);
static bool try_parse_declarator(type_t** type, token_t** /*nullable*/ out_name);

static bool try_parse_specifier(int* flags, int flag, const string_t* keyword) {
    if (!lexer_accept(keyword)) {
        return false;
    }
    if (*flags & flag) {
        fatal("Redundant declaration specifier: %s", keyword->bytes);
    }
    *flags |= flag;
    return true;
}

static bool try_parse_type_qualifier(int* type_qualifiers) {
    if (try_parse_specifier(type_qualifiers, TYPE_QUALIFIER_CONST, STR_CONST)) return true;
    if (try_parse_specifier(type_qualifiers, TYPE_QUALIFIER_VOLATILE, STR_VOLATILE)) return true;
    if (try_parse_specifier(type_qualifiers, TYPE_QUALIFIER_RESTRICT, STR_RESTRICT)) return true;
    return false;
}

static bool try_parse_type_qualifiers(int* type_qualifiers) {
    if (!try_parse_type_qualifier(type_qualifiers))
        return false;
    while (try_parse_type_qualifiers(type_qualifiers)) {}
    return true;
}

static bool try_parse_declaration_specifier_keywords(specifiers_t* specifiers) {

    // storage specifiers
    if (try_parse_specifier(&specifiers->storage_specifiers, STORAGE_SPECIFIER_TYPEDEF, STR_TYPEDEF)) return true;
    if (try_parse_specifier(&specifiers->storage_specifiers, STORAGE_SPECIFIER_EXTERN, STR_EXTERN)) return true;
    if (try_parse_specifier(&specifiers->storage_specifiers, STORAGE_SPECIFIER_STATIC, STR_STATIC)) return true;
    if (try_parse_specifier(&specifiers->storage_specifiers, STORAGE_SPECIFIER_THREAD_LOCAL, STR_THREAD_LOCAL)) return true;
    if (try_parse_specifier(&specifiers->storage_specifiers, STORAGE_SPECIFIER_AUTO, STR_AUTO)) return true;
    if (try_parse_specifier(&specifiers->storage_specifiers, STORAGE_SPECIFIER_REGISTER, STR_REGISTER)) return true;

    // type specifiers (except `long` and user types)
    if (try_parse_specifier(&specifiers->type_specifiers, TYPE_SPECIFIER_VOID, STR_VOID)) return true;
    if (try_parse_specifier(&specifiers->type_specifiers, TYPE_SPECIFIER_CHAR, STR_CHAR)) return true;
    if (try_parse_specifier(&specifiers->type_specifiers, TYPE_SPECIFIER_SHORT, STR_SHORT)) return true;
    if (try_parse_specifier(&specifiers->type_specifiers, TYPE_SPECIFIER_INT, STR_INT)) return true;
    if (try_parse_specifier(&specifiers->type_specifiers, TYPE_SPECIFIER_SIGNED, STR_SIGNED)) return true;
    if (try_parse_specifier(&specifiers->type_specifiers, TYPE_SPECIFIER_UNSIGNED, STR_UNSIGNED)) return true;
//TODO _Bool disabled to compile as cci/0
    //if (try_parse_specifier(&specifiers->type_specifiers, TYPE_SPECIFIER_BOOL, STR_BOOL_X)) return true;

    // type qualifiers
    if (try_parse_type_qualifier(&specifiers->type_qualifiers)) return true;

    // function specifiers
    if (try_parse_specifier(&specifiers->function_specifiers, FUNCTION_SPECIFIER_INLINE, STR_INLINE)) return true;
    if (try_parse_specifier(&specifiers->function_specifiers, FUNCTION_SPECIFIER_NORETURN, STR_NORETURN_X)) return true;

    // `long` can appear at most twice.
//TODO long disabled to compile as cci/0
    if (0&&lexer_accept(STR_LONG)) {
        if (specifiers->type_specifiers & TYPE_SPECIFIER_LONG_LONG) {
            fatal("`long long long` is invalid.");
        }
        if (specifiers->type_specifiers & TYPE_SPECIFIER_LONG) {
            specifiers->type_specifiers &= ~TYPE_SPECIFIER_LONG;
            specifiers->type_specifiers |= TYPE_SPECIFIER_LONG_LONG;
            return true;
        }
        specifiers->type_specifiers |= TYPE_SPECIFIER_LONG;
        return true;
    }

    // We don't support _Atomic yet because we have to parse _Atomic(type-specifiers)
    // and I haven't bothered yet. Same with _Alignas. We might never bother to
    // implement these since there's no point.
    if (lexer_accept(STR_ATOMIC)) {
        fatal("_Atomic is not supported.");
    }
    if (lexer_accept(STR_ALIGNAS) || lexer_accept(STR_ALIGNAS_X)) {
        fatal("_Alignas is not supported.");
    }

    return false;
}

static void parse_record_member(record_t* record) {

    // Parse specifier sequence
    specifiers_t specifiers;
    specifiers_init(&specifiers);
    if (!try_parse_declaration_specifiers(&specifiers)) {
        fatal_token(lexer_token, "Expected a declaration.");
    }
    type_t* base_type = specifiers_make_type(&specifiers);

    // No storage or function specifiers are allowed
    if (specifiers.storage_specifiers) {
        fatal("Storage specifiers are not allowed in a `struct` or `union` definition.");
    }
    if (specifiers.function_specifiers) {
        fatal("Function specifiers are not allowed in a `struct` or `union` definition.");
    }

    // Parse comma-separated list of declarators.
    for (;;) {
        type_t* type = type_ref(base_type);
        token_t* name = NULL;
        if (!try_parse_declarator(&type, &name)) {
            fatal_token(lexer_token, "Expected a declarator for this `struct` or `union` member declaration.");
        }

        if (name == NULL) {
            // There are a few cases where a struct member can be anonymous:
            // - an anonymous struct or union
            // - a zero-width bitfield
            // TODO handle these cases
            fatal("TODO record member with blank name");
        }

        int offset = record->is_struct ? record_size(record) : 0;
        record_add(record, name, type, offset);
        token_deref(name);
        type_deref(type);

        if (lexer_is(STR_ASSIGN)) {
            fatal_token(lexer_token, "An initializer is not allowed in a `struct` or `union` member declaration.");
        }
        if (lexer_accept(STR_COMMA)) {
            continue;
        }
        lexer_expect(STR_SEMICOLON, "Expected `;` or `,` at end of declaration.");
        break;
    }

    type_deref(base_type);
    specifiers_destroy(&specifiers);
}

static void parse_record(specifiers_t* specifiers) {

    // apply the struct/union keyword
    if (specifiers->type_specifiers & TYPE_SPECIFIER_RECORD) {
        fatal_token(lexer_token, "Redundant struct/union specifier");
    }
    specifiers->type_specifiers |= TYPE_SPECIFIER_RECORD;
    bool is_struct = lexer_is(STR_STRUCT);
    lexer_consume();

    // collect the name
    if (lexer_token->type != token_type_alphanumeric && !lexer_is(STR_BRACE_OPEN)) {
        fatal_token(lexer_token, "Expected name or `{` after `%s`", is_struct ? "struct" : "union");
    }
    token_t* name = lexer_take();

    // Decide if we should search in parent scopes or only in the current scope
    // for this record definition. If this is a record definition, or if this
    // is a forward declaration, we only check the current scope (as both
    // declare a record in the current scope if one doesn't already exist.)
    //
    // A forward declaration of a struct occurs when the named struct is the
    // only thing in the declaration. Anything else, even a type qualifier
    // (e.g. const), makes it not a forward declaration.
    //
    // GCC has a warning about an incorrect forward declaration (e.g. `const
    // struct foo;`) that fails to shadow a declaration in an outer scope. We
    // could potentially implement the same. We have a test for this: see
    bool is_definition = lexer_is(STR_BRACE_OPEN);
    bool is_forward_declaration = lexer_is(STR_SEMICOLON)
            && specifiers->type_qualifiers == 0 && specifiers->storage_specifiers == 0;
    bool find_recursive = !is_definition && !is_forward_declaration;

    // find or create the struct
    type_t* type = scope_find_type(scope_current,
            is_struct ? TAG_STRUCT : TAG_UNION,
            name->value,
            find_recursive);
    if (type == NULL) {
        record_t* record = record_new(name->value, is_struct);
        type = type_new_record(record);
        scope_add_type(scope_current,
                is_struct ? TAG_STRUCT : TAG_UNION,
                name,
                type);
        type_deref(type);
        record_deref(record);
    }
    assert(type_matches_base(type, BASE_RECORD));
    token_deref(name);
    specifiers->type = type;

    // parse a definition if given
    if (lexer_is(STR_BRACE_OPEN)) {

        // make sure we don't already have a definition
        record_t* record = type->record;
        if (record->is_defined) {
            fatal_token(lexer_token, "Duplicate definition of struct/union");
        }
        lexer_consume();

        // parse members
        while (!lexer_accept(STR_BRACE_CLOSE)) {
            parse_record_member(record);
        }

        if (record->member_list == NULL) {
            // TODO an empty struct is a GNU extension
            fatal("TODO empty struct not yet supported, GNU extension");
        }

        record->is_defined = true;
    }
}

static bool try_parse_declaration_specifiers(specifiers_t* specifiers) {
    bool found;

    // All declaration specifiers are alphanumeric so we can short-circuit with
    // an alphanumeric test.
    while (lexer_token->type == token_type_alphanumeric) {
        if (try_parse_declaration_specifier_keywords(specifiers)) {
            found = true;
            continue;
        }

        // record (struct and enum)
        if (lexer_is(STR_STRUCT) || lexer_is(STR_UNION)) {
            found = true;
            parse_record(specifiers);
            continue;
        }

        // enum
        if (lexer_accept(STR_ENUM)) {
            found = true;
            fatal("enum not yet implemented");
            continue;
        }

        // typedef (only if we don't already have a type specifier)
        if (specifiers->type_specifiers == 0) {
            type_t* type = scope_find_type(scope_current, TAG_TYPEDEF, lexer_token->value, true);
            if (type) {
                found = true;
                if (specifiers->type) {
                    fatal_token(lexer_token, "Redundant type name specifier");
                }
                lexer_consume();
                specifiers->type_specifiers |= TYPE_SPECIFIER_TYPEDEF;
                specifiers->type = type;
                continue;
            }
        }

        break;
    }

    return found;
}

static base_t specifiers_convert(specifiers_t* specifiers) {
    int ts = specifiers->type_specifiers;

    // Check for implicit int
    if (ts == 0) {
        warn(warning_implicit_int, lexer_token,
                "No type specifiers for this declaration. (Implicit int was removed in C99.)");
        return BASE_SIGNED_INT;
    }

    // If we have another integer width, "int" is redundant.
    if (ts & TYPE_SPECIFIER_INT) {
        if ((ts & TYPE_SPECIFIER_SHORT) ||
            (ts & TYPE_SPECIFIER_LONG) ||
            (ts & TYPE_SPECIFIER_LONG_LONG))
        {
            ts &= ~TYPE_SPECIFIER_INT;
        }
    }

    // If we have "long", convert it to "int".
    if (ts & TYPE_SPECIFIER_LONG) {
        ts &= ~TYPE_SPECIFIER_LONG;
        ts |= TYPE_SPECIFIER_INT;
    }

    // We check each supported type combination against our type specifier
    // bitmap (minus our simplifications above.) See C11 6.7.2.2

    if (ts == TYPE_SPECIFIER_VOID) {return BASE_VOID;}

    if (ts == (TYPE_SPECIFIER_UNSIGNED | TYPE_SPECIFIER_CHAR))
        {return BASE_UNSIGNED_CHAR;}
    if (ts == (TYPE_SPECIFIER_UNSIGNED | TYPE_SPECIFIER_SHORT))
        {return BASE_UNSIGNED_SHORT;}
    if ((ts == (TYPE_SPECIFIER_UNSIGNED | TYPE_SPECIFIER_INT)) |
            (ts == TYPE_SPECIFIER_UNSIGNED))
        {return BASE_UNSIGNED_INT;}
    if (ts == (TYPE_SPECIFIER_UNSIGNED | TYPE_SPECIFIER_LONG_LONG))
        {return BASE_UNSIGNED_LONG_LONG;}

    if ((ts == TYPE_SPECIFIER_CHAR) |
            (ts == (TYPE_SPECIFIER_SIGNED | TYPE_SPECIFIER_CHAR)))
        {return BASE_SIGNED_CHAR;}
    if ((ts == TYPE_SPECIFIER_SHORT) |
            (ts == (TYPE_SPECIFIER_SIGNED | TYPE_SPECIFIER_SHORT)))
        {return BASE_SIGNED_SHORT;}
    if (((ts == TYPE_SPECIFIER_INT) |
            (ts == TYPE_SPECIFIER_SIGNED)) |
            (ts == (TYPE_SPECIFIER_SIGNED | TYPE_SPECIFIER_INT)))
        {return BASE_SIGNED_INT;}
    if ((ts == TYPE_SPECIFIER_LONG_LONG) |
            (ts == (TYPE_SPECIFIER_SIGNED | TYPE_SPECIFIER_LONG_LONG)))
        {return BASE_SIGNED_LONG_LONG;}

    // TODO floats, etc

    fatal("Unsupported combination of type specifiers.");
}

static type_t* specifiers_make_type(specifiers_t* specifiers) {
    if (specifiers->type) {
        switch (specifiers->type_specifiers) {
            case TYPE_SPECIFIER_TYPEDEF:
            case TYPE_SPECIFIER_ENUM:
            case TYPE_SPECIFIER_RECORD:
                return type_ref(specifiers->type);
            default:
                break;
        }
        fatal("Unsupported combination of type specifiers.");
    }
    base_t base = specifiers_convert(specifiers);
    return type_new_base(base);
}



/*
 * Declarators
 *
 * The declarator parse functions take a strong reference to a type and return
 * a strong reference to a potentially different type that includes any parsed
 * declarators.
 */

static void parse_function_arguments(type_t** type) {
    bool is_variadic = false;
    int arg_count = 0;
    int arg_capacity = 64;
    type_t** arg_types = malloc(arg_capacity * sizeof(type_t*));
    token_t** arg_names = malloc(arg_capacity * sizeof(token_t*));

    // TODO we should check for ")" here for an old-style function that is not
    // a prototype

    while (!lexer_accept(STR_PAREN_CLOSE)) {
        if (arg_count > 0) {
            lexer_expect(STR_COMMA, "Expected `,` or `)` after function argument.");
        }
        if (arg_count == arg_capacity) {
            // TODO make this growable.
            fatal("This function has too many parameters.");
        }

        // check for variadic arguments
        if (lexer_accept(STR_ELLIPSIS)) {
            if (arg_count == 0) {
                fatal_token(lexer_token, "At least one non-variadic argument is required before `...`.");
            }
            lexer_expect(STR_PAREN_CLOSE, "Expected `)` after `...`");
            is_variadic = true;
            break;
        }

        // Parse specifier sequence
        specifiers_t specifiers;
        specifiers_init(&specifiers);
        if (!try_parse_declaration_specifiers(&specifiers)) {
            // TODO implicit int, K&R-style function prototypes
            fatal_token(lexer_token, "Expected a declaration specifier (a type) for this function parameter.");
        }

        // Check for specifier errors
        // TODO error should be on correct token. try_parse_declaration_specifiers()
        // should take an option to forbid invalid specifiers
        // TODO we should also forbid restrict here
        if (specifiers.storage_specifiers != 0 || specifiers.function_specifiers != 0) {
            fatal("Storage and function specifiers are not allowed on function parameters.");
        }

        // Parse declarator (rest of type and name)
        type_t** type = arg_types + arg_count;
        token_t** name = arg_names + arg_count;
        *type = specifiers_make_type(&specifiers);
        specifiers_destroy(&specifiers);
        *name = NULL;
        if (!try_parse_declarator(type, name)) {
            // Check for (void)
            if (arg_count == 0 && type_matches_base(*type, BASE_VOID) && *name == NULL && lexer_accept(STR_PAREN_CLOSE)) {
                type_deref(*type);
                break;
            }
        }

        ++arg_count;
    }

    *type = type_new_function(*type, arg_types, arg_names, arg_count, is_variadic);
    free(arg_types);
    free(arg_names);
}

static bool try_parse_direct_declarator(type_t** type, token_t** /*nullable*/ out_name) {
    bool found = false;

    // Parens before another direct declarator are a parenthesized declarator.
    if (lexer_accept(STR_PAREN_OPEN)) {
        if (!try_parse_declarator(type, out_name)) {
            fatal("Expected declarator after `(`");
        }
        lexer_expect(STR_PAREN_CLOSE, "Expected `)` after parenthesized declarator.");
        found = true;
    }

    // If out_name is NULL, this declarator is abstract. In that case a name
    // cannot be provided and the direct-declarator is optional.
    if (out_name != NULL) {

        // Check for a name
        if (!found && lexer_token->type == token_type_alphanumeric && out_name != NULL) {
            if (*out_name != NULL) {
                fatal("Redundant identifier in declarator: %s", string_cstr(lexer_token->value));
            }
            // TODO We should check that this isn't a keyword. For now we don't bother.
            *out_name = lexer_take();
            found = true;
        }

        if (!found) {
            return false;
        }
    }

    // Square brackets are arrays
    while (lexer_accept(STR_SQUARE_OPEN)) {
        fatal("TODO array declarators");
    }

    // Parens after another direct declarator are function arguments.
    while (lexer_accept(STR_PAREN_OPEN)) {
        //printf("found a `(` for %s ", (*out_name)->value->bytes);
        //type_print(*type);
        //printf("\n");
        parse_function_arguments(type);
    }

    return found;
}

static bool try_parse_declarator(type_t** type, token_t** /*nullable*/ out_name) {

    /*
    // Check for a pointer. If so, recurse and apply the pointer afterwards.
    // TODO no, this is all wrong
    if (lexer_accept(STR_ASTERISK)) {
        int type_qualifiers = 0;
        try_parse_type_qualifiers(&type_qualifiers);
        if (!try_parse_declarator(type, out_name)) {
            fatal("Expected a declarator.");
        }
        type_t* new_type = type_new_pointer(*type,
                !!(type_qualifiers & TYPE_QUALIFIER_CONST),
                !!(type_qualifiers & TYPE_QUALIFIER_VOLATILE),
                !!(type_qualifiers & TYPE_QUALIFIER_RESTRICT));
        type_deref(*type);
        *type = new_type;
        return true;
    }
    */

    // Collect pointers
    while (lexer_accept(STR_ASTERISK)) {
        int type_qualifiers = 0;
        try_parse_type_qualifiers(&type_qualifiers);
        type_t* new_type = type_new_pointer(*type,
                !!(type_qualifiers & TYPE_QUALIFIER_CONST),
                !!(type_qualifiers & TYPE_QUALIFIER_VOLATILE),
                !!(type_qualifiers & TYPE_QUALIFIER_RESTRICT));
        type_deref(*type);
        *type = new_type;
    }

    return try_parse_direct_declarator(type, out_name);
}

static void parse_function_definition(type_t* type, token_t* name, string_t* asm_name) {

    // create the function
    assert(type_is_function(type));
    node_t* root = node_new_token(NODE_FUNCTION, name);
    root->type = type_ref(type->ref); // return value
    function_t* function = function_new(type, name->value, asm_name, root);
    current_function = function;

    // attach parameters
    for (uint32_t i = 0; i < type->count; ++i) {
        token_t* param_name = type->names[i];
        node_t* param = node_new_token(NODE_PARAMETER, param_name);
        param->type = type_ref(type->args[i]);
        node_append(root, param);

        // TODO warn about unnamed parameters before whatever version of the standard allowed them
        if (param_name != NULL) {
            // TODO check for duplicate parameter names (here or in declaration below)
            symbol_t* symbol = symbol_new(symbol_kind_variable, param->type, param_name, param_name->value);
            param->symbol = symbol;
            scope_add_symbol(scope_current, symbol);
        }
    }

    // parse
    scope_push();
    // TODO handle args
    node_append(root, parse_compound_statement());
    scope_pop();

    // codegen
    generate_function(function);
    emit_function(function);

    // done
    current_function = NULL;
    function_delete(function);
}

static void parse_function_declaration(specifiers_t* specifiers, type_t* type, token_t* name, string_t* asm_name) {

    // create the symbol
    symbol_t* symbol = symbol_new(symbol_kind_function, type, name, asm_name);
    // TODO handle duplicate/redundant declarations
    // TODO refcount properly
    scope_add_symbol(scope_current, symbol);
    symbol_deref(symbol);

    // check for a function definition
    if (!lexer_is(STR_BRACE_OPEN)) {
        lexer_expect(STR_SEMICOLON, "Expected `;` or `{` after function declaration");
    } else {
        if (scope_current != scope_global) {
            fatal_token(lexer_token, "Function definitions can only appear at file scope.");
        }
        parse_function_definition(type, name, asm_name);
    }

    string_deref(asm_name);
    token_deref(name);
    type_deref(type);
}

/**
 * Parses an initializer for the given type.
 *
 * The initializer can be an expression, a string, or an initializer list.
 */
static node_t* parse_initializer(type_t* type) {
    if (lexer_accept(STR_BRACE_OPEN)) {
        fatal("TODO parse initializer list");
    }

    if (lexer_token->type == token_type_string) {
        fatal("TODO parse string literal initializer");
    }

    if (lexer_token->type == token_type_character) {
        fatal("TODO parse char literal initializer");
    }

    node_t* node = parse_expression();
    if (!type_equal(type, node->type)) {
        // TODO check that types compatible, etc. We need a general purpose
        // "insert cast" function somewhere that handles all these cases (both
        // implicit and explicit) and does the appropriate checks
        node_t* cast = node_new(NODE_CAST);
        cast->type = type_ref(type);
        node_append(cast, node);
        node = cast;
    }
    return node;
}

static void parse_variable_declaration(node_t* /*nullable*/ parent,
        specifiers_t* specifiers, type_t* type, token_t* name, string_t* asm_name)
{
    if (lexer_is(STR_BRACE_OPEN)) {
        fatal("Cannot initialize a variable with `{`.");
    }

    // We only support extern, static and neither.
    bool is_extern = specifiers->storage_specifiers == STORAGE_SPECIFIER_EXTERN;
    bool is_static = specifiers->storage_specifiers == STORAGE_SPECIFIER_STATIC;
    if (!is_extern && !is_static && specifiers->storage_specifiers != 0) {
        fatal_token(name, "Invalid or unsupported storage specifiers for declaration.");
    }
    if (parent && is_extern) {
        fatal_token(name, "Cannot declare a local variable with `extern` storage.");
    }

    // Collect the initializer
    node_t* initializer = NULL;
    if (lexer_is(STR_ASSIGN)) {
        if (is_extern) {
            fatal_token(lexer_token, "Cannot initialize a variable with `extern` storage.");
        }
        lexer_consume();
        initializer = parse_initializer(type);
    }

    bool is_tentative =
        (parent == NULL) &&  // global declaration
        (initializer == NULL) &&  // no initializer
        specifiers->storage_specifiers == 0;  // no storage specifiers

    // Check to see if there's already a symbol with this name in this scope.
    symbol_t* previous = scope_find_symbol(scope_current, name->value, false);
    if (previous) {

        // The types must match.
        if (!type_equal(previous->type, type)) {
            fatal_token(name, "Variable re-declared with a different type.");
        }

        // We replace the declaration if:
        // - The previous declaration is extern and this one is not; or
        // - The previous declaration is tentative and this one is neither
        //   tentative nor extern
        if ((previous->is_extern && !is_extern) ||
                (previous->is_tentative && !is_tentative && !is_extern))
        {
            scope_remove_symbol(scope_current, previous);
            symbol_deref(previous);
        } else {
            // If we're not replacing the declaration, ignore it.
            goto ignore_declaration;
        }
    }

    // TODO static const variables with constant expression values should be
    // considered constants. should be a flag in symbol

    symbol_t* symbol = symbol_new(symbol_kind_variable, type, name, asm_name);
    symbol->is_tentative = is_tentative;
    symbol->is_extern = is_extern;
    symbol->is_static = is_static;
    scope_add_symbol(scope_current, symbol);

    if (parent) {
        if (specifiers->storage_specifiers) {
            // TODO support static
            if (specifiers->storage_specifiers == STORAGE_SPECIFIER_STATIC) {
                fatal_token(name, "`static` local variables are not implemented yet.");
            }
            fatal_token(name, "Storage specifiers are not allowed on this local variable.");
        }

        // It's a local variable. Its offset will be set at code generation time.
        symbol->offset = 0;

        // Create a node for it
        node_t* node = node_new_token(NODE_VARIABLE, name);
        node->type = type_new_base(BASE_VOID);
        node->symbol = symbol_ref(symbol);
        node_append(parent, node);
        if (initializer) {
            node_append(node, initializer);
        }

    } else {
        // Global variable. If it's neither extern nor tentative, we can emit a
        // definition for it now.
        if (!is_extern && !is_tentative) {
            generate_global_variable(symbol, initializer);
        }
    }

    symbol_deref(symbol);
ignore_declaration:
    string_deref(asm_name);
    token_deref(name);
    type_deref(type);
}



/*
 * Declaration
 */

type_t* try_parse_type(void) {

    // Parse specifier sequence
    specifiers_t specifiers;
    specifiers_init(&specifiers);
    if (!try_parse_declaration_specifiers(&specifiers)) {
        specifiers_destroy(&specifiers);
        return NULL;
    }

    // Storage and function specifiers are not allowed.
    if (specifiers.storage_specifiers != 0) {
        fatal("Storage specifiers are not allowed on this type declaration.");
    }
    if (specifiers.function_specifiers != 0) {
        fatal("Function specifiers are not allowed on this type declaration.");
    }

    // Parse declarator
    token_t* name = NULL;
    type_t* type = specifiers_make_type(&specifiers);
    try_parse_declarator(&type, &name);

    if (name != NULL) {
        fatal_token(name, "Expected an unnamed type declarator.");
    }

    specifiers_destroy(&specifiers);
    return type;
}

static string_t* parse_asm_name(node_t* /*nullable*/ parent, specifiers_t* specifiers, token_t* name) {

    // check for keyword
    bool is_asm = lexer_is(STR_ASM);
    if (!is_asm && !lexer_is(STR_ASM_X)) {
        return string_ref(name->value);
    }

    // do some error checks
    if (is_asm)
        warn(warning_extra_keywords, lexer_token, "`asm` is a GNU extension. (Use `__asm__` or pass `-fasm` or `-fgnu-extensions` or `-std=gnu*`.)");
    if (parent && !(specifiers->storage_specifiers & STORAGE_SPECIFIER_EXTERN)) {
        fatal_token(lexer_token, "Cannot provide an asm name for a local symbol.");
    }

    // parse it
    lexer_consume();
    lexer_expect(STR_PAREN_OPEN, "Expected `(` for an asm name declaration.");
    if (lexer_token->type != token_type_string)
        fatal_token(lexer_token, "Expected a string in this asm name declaration.");
    string_t* asm_name = string_ref(lexer_token->value);
    lexer_consume();

    // check for string concatenations
    while (lexer_token->type == token_type_string) {
        // (This is really inefficient because we're doing two copies per
        // additional string fragment plus all the unnecessary string
        // interning. It really doesn't matter though because concatenated asm
        // names are rare.)
        size_t total = string_length(asm_name) + string_length(lexer_token->value);
        char* c = malloc(total);
        memcpy(c, string_cstr(asm_name), string_length(asm_name));
        memcpy(c + string_length(asm_name), string_cstr(lexer_token->value), string_length(lexer_token->value));
        string_deref(asm_name);
        asm_name = string_intern_bytes(c, total);
        free(c);
        lexer_consume();
    }

    lexer_expect(STR_PAREN_CLOSE, "Expected `)` after asm name declaration.");
    return asm_name;
}

bool try_parse_declaration(node_t* /*nullable*/ parent) {
    //printf("\n-------------------\ntrying to parse new decl - %s\n", lexer_token->value->bytes);

    // Parse specifier sequence. At file scope we allow a empty specifier list
    // if implicit int is enabled. TODO make that an option
    specifiers_t specifiers;
    specifiers_init(&specifiers);
    if (!try_parse_declaration_specifiers(&specifiers) && parent != NULL) {
        specifiers_destroy(&specifiers);
        return false;
    }
    type_t* base_type = specifiers_make_type(&specifiers);

    // Parse comma-separated list of declarators.
    for (;;) {
        type_t* type = type_ref(base_type);
        token_t* name = NULL;
        if (!try_parse_declarator(&type, &name)) {
            if (specifiers.type_specifiers == TYPE_SPECIFIER_RECORD) {
                // A forward declaration of a struct is allowed. TODO make sure
                // it's in the correct scope, if not re-declare it.
            } else if (specifiers.type_specifiers == TYPE_SPECIFIER_ENUM) {
                fatal("TODO enum forward declaration");
            } else {
                fatal("Expected a declarator for this global declaration.");
            }
        }

        // A no-name declaration is not necessarily an error, for example it
        // could be a struct or enum.
        // TODO check if it's actually a struct or enum
        // TODO make sure it's not a typedef, extern, etc.
        if (name == NULL) {
            type_deref(type);
            goto declarator_done;
        }
        /*
        printf("parsed name: %s  type: ", string_cstr(name->value));
        type_print(type);
        puts("");
        */

        // Check for a typedef
        if (specifiers.storage_specifiers & STORAGE_SPECIFIER_TYPEDEF) {
            if (specifiers.storage_specifiers != STORAGE_SPECIFIER_TYPEDEF) {
                fatal_token(name, "`typedef` cannot be combined with other storage specifiers.");
            }
            scope_add_type(scope_current, TAG_TYPEDEF, name, type);
            if (lexer_is(STR_ASSIGN) || lexer_is(STR_BRACE_OPEN)) {
                fatal_token(name, "A definition cannot be provided for a `typedef` declaration.");
            }
            type_deref(type);
            token_deref(name);
            goto declarator_done;
        }

        // Parse an asm name (or default to the declared name)
        string_t* asm_name = parse_asm_name(parent, &specifiers, name);

        // Check for a function
        //printf("asm name %s\n", asm_name->bytes);
        //type_print(type);
        if (type_is_declarator(type) && type->declarator == DECLARATOR_FUNCTION) {
            //printf("    is func\n");
            parse_function_declaration(&specifiers, type, name, asm_name);
            break;
        } else {
            //printf("    is var\n");
            parse_variable_declaration(parent, &specifiers, type, name, asm_name);
        }

    declarator_done:
        if (lexer_accept(STR_COMMA)) {
            continue;
        }
        lexer_expect(STR_SEMICOLON, "Expected `;` or `,` at end of declaration.");
        break;
    }

    type_deref(base_type);
    specifiers_destroy(&specifiers);
    return true;
}

void parse_global(void) {
    if (!try_parse_declaration(NULL)) {
        fatal("Expected a declaration at file scope.");
    }
}
