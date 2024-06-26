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

#include "options.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>

#include "libo-error.h"
#include "libo-table.h"
#include "libo-util.h"
#include "common.h"
#include "token.h"

/**
 * Returns true if string starts with prefix.
 */
static bool starts_with(const char* string, const char* prefix) {
    size_t len = strlen(prefix);
    if (len > strlen(string)) {
        return false;
    }
    return 0 == memcmp(string, prefix, len);
}



/****************************************
 * Warnings
 ****************************************/

/*
 * We support many extensions to C, but unlike most C compilers, we warn about
 * using them by default.
 *
 * The logic is something like this:
 *
 * - If an extension warning option is explicitly specified, this overrides any
 *   other flags for this extension. If specified multiple times, the last one
 *   is used.
 *
 * - Otherwise, if a compiler-specific extension flag is used, we do not warn
 *   about extensions from that compiler. This includes `-fgnu-extensions`,
 *   `-fms-extensions`, `-fplan9-extensions` and the `-std=gnu*` options.
 *
 * - Otherwise, if -Werror is specified, we treat extension usage as an error.
 *
 * - Otherwise, we treat extension usage as a warning.
 *
 * As in GNU C, __extension__ disables all extension warnings until the end of
 * the expression or statement.
 *
 * (TODO most of the above is not implemented yet, but this is the goal.)
 */

/*
 * Defines how warnings are specified.
 *
 * Warnings can be specified on the command-line in multiple ways. For a
 * warning "foo", you can specify:
 *
 * - -Wfoo            Treats "foo" as a warning, or an error if -Werror is specified
 * - -Wno-foo         Disables warnings about "foo"
 * - -Werror=foo      Treats "foo" as an error
 * - -Wno-error=foo   Treats "foo" as a warning regardless of -Werror
 *
 * We capture the specificity of warnings here. This works for groups as well.
 *
 * Once all command-line options are parsed, we then walk the groups and the
 * individual warnings to set the computed level of all warnings.
 */
typedef enum warning_spec_t {
    warning_spec_none,      // not specified
    warning_spec_on,        // specified with -W... or -f...
    warning_spec_off,       // specified with -Wno-... or -fno-...
    warning_spec_error,     // specified with -Werror=...
    warning_spec_no_error,  // specified with -Wno-error=...
} warning_spec_t;

static warning_spec_t* warning_specs;

typedef enum warning_level_t {
    warning_level_off = 0,
    warning_level_warn,
    warning_level_error,
} warning_level_t;

/*
 * Warnings are stored in an array which is itself stored in a stack.
 *
 * We implement `#pragma GCC diagnostic push` by copying the array and pushing
 * it onto the stack.
 */
static warning_level_t** warning_levels;
static size_t warning_levels_count;
static size_t warning_levels_capacity;

bool optimization;
int dump_ast;

/*
 * The warning table is a mapping of command-line warning strings (not
 * including `-W` and other prefixes) to the warning enum name.
 *
 * We used a fixed-size open hashtable with linear probing. We also don't
 * intern these strings. We care more about startup time than parse time
 * because we have way more warnings than are likely to be specified on any
 * command-line so building this table needs to be fast.
 */
#define warning_table_capacity 256 // must be power of two
#define warning_table_mask (warning_table_capacity - 1)
static const char* warning_table_args[warning_table_capacity]; // NULL means empty bucket
static warning_t warning_table_values[warning_table_capacity];
static size_t warning_table_count;

static void warning_add(const char* arg, warning_t warning, warning_level_t default_level) {
    if (warning_table_count++ == (warning_table_capacity >> 1)) {
        fatal("Internal error: warning_table_capacity needs to be increased.");
    }

    uint32_t bucket = fnv1a_cstr(arg) & warning_table_mask;
    while (warning_table_args[bucket]) {
        bucket = (bucket + 1) & warning_table_mask;
    }

    warning_table_args[bucket] = arg;
    warning_table_values[bucket] = warning;

    warning_levels[0][warning] = default_level;
}

static void warnings_init(void) {
    warning_specs = calloc(warning_count, sizeof(warning_spec_t));
    warning_levels = malloc(sizeof(warning_level_t*));
    warning_levels[0] = calloc(warning_count, sizeof(warning_level_t));
    warning_levels_count = 1;
    warning_levels_capacity = 1;

    // groups
    warning_add("all", warning_all, warning_level_off);
    warning_add("extra", warning_extra, warning_level_off);
    warning_add("pedantic", warning_pedantic, warning_level_off);

    // group aliases
    // TODO these don't start with -W so they won't go here, but in generic options at the bottom
    //warning_table_add(NULL, "-pedantic", warning_pedantic, warning_spec_on);
    // TODO the GCC documentation claims that -pedantic-errors is not the same
    // as -Werror=pedantic. For now we treat them as the same.
    //warning_table_add(NULL, "-pedantic-errors", warning_pedantic, warning_spec_error);

    // deprecated
    warning_add("implicit-int", warning_implicit_int, warning_level_warn);

    // extension usage
    warning_add("statement-expressions", warning_statement_expressions, warning_level_off);
    warning_add("asm", warning_extra_keywords, warning_level_off);
}

static void warnings_destroy(void) {
    free(warning_specs);
    for (size_t i = 0; i < warning_levels_count; ++i)
        free(warning_levels[i]);
    free(warning_levels);
}

static bool warning_parse(const char* arg) {
    if (!starts_with(arg, "-W"))
        return false;

    // TODO

    return true;
}



/****************************************
 * Flags
 ****************************************/

static void flags_init(void) {
    // TODO
}

static void flags_destroy(void) {
}

static bool flag_parse(const char* arg) {
    // TODO
    return false;
}



/****************************************
 * Other Options
 ****************************************/

void options_init(void) {
    warnings_init();
    flags_init();

    //optimization = true;
}

void options_destroy(void) {
    flags_destroy();
    warnings_destroy();
}

void warn(warning_t warning, struct token_t* token, const char* message, ...) {
    warning_level_t level = warning_levels[0][warning];
    if (level == warning_level_off)
        return;

    // TODO include arg. probably should just call asprintf twice, this doesn't
    // need to be fast
    va_list args;
    va_start(args, message);
    if (level == warning_level_error)
        vfatal_token(token, message, args);
    current_line = token->line;
    current_filename = (char*)string_cstr(token->filename);
    vwarning(message, args);
    current_filename = NULL;
    va_end(args);
}

static bool options_parse_dump_ast(const char* arg) {
    if (0 == strcmp("-dump-ast=off", arg)) {
        dump_ast = DUMP_AST_OFF;
        return true;
    }
    if (0 == strcmp("-dump-ast", arg) || 0 == strcmp("-dump-ast=unicode", arg)) {
        dump_ast = DUMP_AST_UNICODE;
        return true;
    }
    if (0 == strcmp("-dump-ast=ascii", arg)) {
        dump_ast = DUMP_AST_ASCII;
        return true;
    }
    return false;
}

bool options_parse(const char* arg) {
    if (warning_parse(arg)) return true;
    if (flag_parse(arg)) return true;
    if (options_parse_dump_ast(arg)) return true;
    return false;
}
