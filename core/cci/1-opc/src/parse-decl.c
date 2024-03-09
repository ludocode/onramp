#include "parse-decl.h"

#include "lexer.h"



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
 * In opC, we don't allow typedef or struct/union declarations outside of
 * global scope. We take the `global` flag to some of the below functions to
 * check this.
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
#define TYPE_SPECIFIER_LONG_LONG    (1 << 5)
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
        if (*type_specifiers & TYPE_SPECIFIER_LONG_LONG) {
            fatal("`long long long` is invalid.");
        }
        if (*type_specifiers & TYPE_SPECIFIER_LONG) {
            *type_specifiers = (*type_specifiers & ~TYPE_SPECIFIER_LONG);
            *type_specifiers = (*type_specifiers | TYPE_SPECIFIER_LONG_LONG);
            return true;
        }
        *type_specifiers = (*type_specifiers | TYPE_SPECIFIER_LONG);
        return true;
    }

    // handle enum
    if (lexer_accept("enum")) {
        if (lexer_type != lexer_type_alphanumeric) {
            fatal("`enum` must be followed by a name.");
        }
        // ignore the name, pretend it's int
        lexer_consume();
        if (*type_specifiers & TYPE_SPECIFIER_INT) {
            fatal("Redundant type specifier: int (or enum)");
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
        // TODO lexer_token is struct/union name, do lookup in typedef table
        fatal("TODO try_parse_type_specifiers() record");
        return true;
    }

    // search for typedef (only if we haven't found any other type specifiers yet)
    if (*type_specifiers == 0) {
        if (*out_record == NULL) {
            if (*out_typedef == NULL) {
                *out_typedef = typedef_find(lexer_token);
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
        if (((type_specifiers & TYPE_SPECIFIER_CHAR) |
            (type_specifiers & TYPE_SPECIFIER_SHORT)) |
            ((type_specifiers & TYPE_SPECIFIER_LONG) |
            (type_specifiers & TYPE_SPECIFIER_LONG_LONG)))
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
    if (type_specifiers == (TYPE_SPECIFIER_UNSIGNED | TYPE_SPECIFIER_INT))
        {return BASE_UNSIGNED_INT;}
    if (type_specifiers == (TYPE_SPECIFIER_UNSIGNED | TYPE_SPECIFIER_LONG_LONG))
        {return BASE_UNSIGNED_LONG_LONG;}

    if ((type_specifiers == TYPE_SPECIFIER_CHAR) |
            (type_specifiers == (TYPE_SPECIFIER_SIGNED | TYPE_SPECIFIER_CHAR)))
        {return BASE_SIGNED_CHAR;}
    if ((type_specifiers == TYPE_SPECIFIER_SHORT) |
            (type_specifiers == (TYPE_SPECIFIER_SIGNED | TYPE_SPECIFIER_SHORT)))
        {return BASE_SIGNED_SHORT;}
    if ((type_specifiers == TYPE_SPECIFIER_INT) |
            (type_specifiers == (TYPE_SPECIFIER_SIGNED | TYPE_SPECIFIER_INT)))
        {return BASE_SIGNED_INT;}
    if ((type_specifiers == TYPE_SPECIFIER_LONG_LONG) |
            (type_specifiers == (TYPE_SPECIFIER_SIGNED | TYPE_SPECIFIER_LONG_LONG)))
        {return BASE_SIGNED_LONG_LONG;}

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
        if (type_specifiers != 0) {
            fatal("Redundant type name and type specifiers in declaration specifier list.");
        }
        *out_type = type_clone(typedef_type);
        return true;
    }

    // see if we got a record
    if (record != NULL) {
        if (typedef_type != NULL) {
            fatal("Redundant type name and struct/union name in declaration specifier list.");
        }
        if (type_specifiers != 0) {
            fatal("Redundant struct/union name and type specifiers in declaration specifier list.");
        }
        //*out_type = type_new_record(record);
        // TODO
        fatal("try_parse_declaration_specifiers() record not yet implemented");
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
 * In this case the list is `*` with name `x`, `[4]` with name `y`, and
 * `*(*(void*))[8]` with name `z`.
 *
 * We don't parse function arguments as part of the declarator list in the opC
 * compiler because we don't support function pointers. Function arguments are
 * parsed separately.
 */

static bool try_parse_declarator_impl(type_t* type, char** /*nullable*/ out_name);

static bool try_parse_pointer(type_t* type) {
    if (!lexer_accept("*")) {
        return false;
    }
    if (type_is_array(type)) {
        // We don't support this in opC.
        fatal("Pointers to arrays are not supported.");
    }
    type_increment_pointers(type);
    // We ignore type qualifiers.
    while (try_parse_type_qualifiers()) {}
    return true;
}

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
            fatal("Multi-dimensional arrays are not supported.");
        }

        if (lexer_accept("]")) {
            type_set_array_length(type, TYPE_ARRAY_INDETERMINATE);
        }

        // We ignore type qualifiers.
        while (try_parse_type_qualifiers()) {}

        fatal("array size not yet implemented");
        int length = 0;// TODO parse a constant expression
        if (length < 0) {
            fatal("Array size cannot be negative.");
        }
        type_set_array_length(type, length);
        found = true;
    }

    return found;
}

static bool try_parse_declarator_impl(type_t* type, char** /*nullable*/ out_name) {
    bool pointer_found = false;

    while (try_parse_pointer(type)) {
        pointer_found = true;
    }

    if (!try_parse_direct_declarator(type, out_name)) {
        if (out_name == NULL) {
            // just pointers are allowed in an abstract declarator
            return pointer_found;
        }
        if (pointer_found) {
            fatal("Pointer declarator must be followed by a direct declarator.");
        }
        return false;
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
