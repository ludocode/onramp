#include "parse-decl.h"

#include "lexer.h"

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
    if (try_parse_storage_class_specifier(out_storage, "typedef", STORAGE_TYPEDEF)) {return true;}
    if (try_parse_storage_class_specifier(out_storage, "extern", STORAGE_EXTERN)) {return true;}
    if (try_parse_storage_class_specifier(out_storage, "static", STORAGE_STATIC)) {return true;}
    return false;
}

static bool try_parse_type_qualifiers(void) {
    if (lexer_accept("const")) {return true;}
    if (lexer_accept("restrict")) {return true;}
    if (lexer_accept("volatile")) {return true;}
    return false;
}

static bool try_parse_function_specifiers(void) {
    if (lexer_accept("inline")) {return true;}
    if (lexer_accept("_Noreturn")) {return true;}
    return false;
}

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
    if (try_parse_type_specifier(type_specifiers, "void", TYPE_SPECIFIER_VOID)) {return true;}
    if (try_parse_type_specifier(type_specifiers, "char", TYPE_SPECIFIER_CHAR)) {return true;}
    if (try_parse_type_specifier(type_specifiers, "short", TYPE_SPECIFIER_SHORT)) {return true;}
    if (try_parse_type_specifier(type_specifiers, "int", TYPE_SPECIFIER_INT)) {return true;}
    if (try_parse_type_specifier(type_specifiers, "signed", TYPE_SPECIFIER_SIGNED)) {return true;}
    if (try_parse_type_specifier(type_specifiers, "unsigned", TYPE_SPECIFIER_UNSIGNED)) {return true;}

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

    const type_t* found = typedef_find(lexer_token);
    if (found) {
        if (out_typedef != NULL) {
            fatal_2("Redundant type specifier: ", lexer_token);
        }
        *out_typedef = found;
        return true;
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

/**
 * Tries to parse a declaration specifier list.
 */
bool try_parse_declaration_specifiers(
        type_t** out_type, 
        storage_t* out_storage)
{
    *out_storage = STORAGE_DEFAULT;

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

        if (try_parse_storage_class_specifiers(out_storage)) {
            optional = false;
            continue;
        }
        if (try_parse_type_specifiers(&type_specifiers, &record, &typedef_type)) {
            optional = false;
            continue;
        }
        break;
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

    // see if we got a typedef
    if (typedef_type != NULL) {
        if (type_specifiers != 0) {
            fatal("Redundant type name and type specifiers in declaration specifier list.");
        }
        *out_type = type_clone(typedef_type);
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
bool try_parse_declaration(type_t** out_type, char** out_name,
        storage_t* out_storage)
{
    base_t base;
    record_t* record;
    storage_t* storage;
    if (!try_parse_declaration_specifier(&base, &record, &storage)) {
        return false;
    }

    // A declaration starts with a declaration specifier list. Check if we have
    // any. If we have none, it's not a declaration. (We don't support default
    // int.)
    const record_t* record;
    const type_t* base_type;
    int specifiers = parse_declaration_specifiers(&record, &base_type);
    if (specifiers == 0) {
        return false;
    }


}
*/

/*
bool try_parse_declarator(const type_t* base_type,
        type_t** out_type, char** out_name)
{
}

void parse_declarator(const type_t* base_type,
        type_t** out_type, char** out_name)
{
    if (!try_parse_declarator(base_type, out_type, out_name)) {
        fatal("Expected a declarator (i.e. a name, `*`, `[`, etc.)");
    }
}
*/
