#ifndef PARSE_DECL_H_INCLUDED
#define PARSE_DECL_H_INCLUDED

#include "common.h"

/**
 * A storage class.
 */
typedef int storage_t;

#define STORAGE_DEFAULT 0
#define STORAGE_TYPEDEF 1
#define STORAGE_EXTERN 2
#define STORAGE_STATIC 3

/**
 * Tries to parse a declaration specifier list.
 */
bool try_parse_declaration_specifiers(
        type_t** out_type, 
        storage_t* out_storage);

/**
 * Tries to parse a declarator using the given base type as the declaration
 * specifier list.
 *
 * Note that we don't include a function's argument list or a variable's
 * initialization list. They are parsed separately.
 *
 * If out_name is NULL, this parses an abstract declarator (a name is not
 * allowed.) If out_name is not NULL and a name is not found, the string
 * pointed to be out_name will be set to NULL.
 */
bool try_parse_declarator(const type_t* base_type, type_t** out_type,
        char** /*nullable*/ out_name);

/**
 * Parses a declarator using the given base type as the declaration specifier
 * list.
 *
 * Note that we don't include a function's argument list or a variable's
 * initialization list. They are parsed separately.
 */
/*
void parse_declarator(const type_t* base_type, type_t** out_type,
        char** out_name);
        */

#endif
