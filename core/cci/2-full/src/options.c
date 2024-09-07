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

#define _GNU_SOURCE

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
 * TODO move to libo
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
 * Warnings can be specified on the command-line in multiple ways. For a
 * warning "foo", you can specify:
 *
 * - -Wfoo            Treats "foo" as a warning, or an error if -Werror is specified
 * - -Wno-foo         Disables warnings about "foo"
 * - -Werror=foo      Treats "foo" as an error
 * - -Wno-error=foo   Treats "foo" as a warning regardless of -Werror
 *
 * We capture the specification of warnings, i.e. what the user specified, in
 * warning_spec_t. This works for groups as well.
 *
 * Once all command-line options are parsed, we then walk the groups and the
 * individual warnings to set the computed level of all warnings.
 */

/*
 * This is complicated all the more by `#pragma GCC diagnostic` lines. For
 * example:
 *
 *     #pragma GCC diagnostic warning "-Wall"
 *     int main(void) { int x; }
 *
 * This is a warning, not an error, even if the above is compiled with
 * `-Werror=implicit-int` on the command-line, in both GCC and Clang. So
 * clearly `-Wall` from a pragma is overriding individual warnings on the
 * command-line.
 *
 * But it gets even more complicated in pragmas, and in fact GCC and Clang
 * don't even agree with each other. For example in the following test case:
 *
 *     #pragma GCC diagnostic warning "-Wunused-variable"
 *     #pragma GCC diagnostic error "-Wall"
 *     int main(void) { int x; }
 *
 * In GCC this is a warning while in Clang it's an error, and in both cases it
 * doesn't matter in what order the #pragma lines appear. If you swap `warning`
 * and `error`, it's a error in both GCC and Clang, and again it doesn't matter
 * what order the pragmas are in.
 *
 * It also doesn't have anything to do with diagnostic stack, because for
 * example:
 *
 *     #pragma GCC diagnostic error "-Wall"
 *     #pragma GCC diagnostic warning "-Wunused-variable"
 *     #pragma GCC diagnostic push
 *     #pragma GCC diagnostic warning "-Wunused-variable"
 *     int main(void) { int x; }
 *
 * In Clang this is still an error. The second and fourth pragmas don't do
 * anything. If however you change either of them from "warning" to "ignored",
 * the error is gone. You can apparently silence the errors but not turn them
 * back to warnings.
 *
 * I give up trying to figure out what algorithm these are using, and if they
 * don't even agree with each other then it's probably not that important to
 * try to match them.
 *
 * Instead we do something much simpler: we just follow the order of the
 * pragmas. Whenever a `#pragma GCC diagnostic` line occurs, we apply the level
 * to the given warning, or to all warnings in the given group even if a more
 * specific option was previously given. This at least seems to match GCC and
 * Clang wherever they agree with each other, even though it matches neither
 * when they don't. It's good enough.
 */



/*
 * The specification of a warning, i.e. what the user specified on the
 * command-line.
 */
typedef enum warning_spec_t {
    warning_spec_none,      // not specified
    warning_spec_on,        // specified with -W...
    warning_spec_off,       // specified with -Wno-...
    warning_spec_error,     // specified with -Werror=...
    warning_spec_no_error,  // specified with -Wno-error=...
} warning_spec_t;

/*
 * The resolved level of a warning: off, warn, or error.
 */
typedef enum warning_level_t {
    warning_level_off = 0,
    warning_level_warn,
    warning_level_error,
} warning_level_t;

/*
 * The specification of all warnings. This is indexed by `warning_t` (not
 * including `warning_count`.)
 *
 * This is only used during argument parsing. It is freed afterwards.
 */
static warning_spec_t* warning_specs;

/*
 * The level of all warnings. This is indexed by `warning_t` (not including
 * `warning_count`.)
 *
 * This can change due to `#pragma GCC diagnostic` lines. We maintain a stack
 * of copies of this array below to implement push/pop. This always points to
 * the top of the stack.
 */
static warning_level_t* warning_levels;

/*
 * The argument string of all warnings.
 */
static const char** warning_args;

/*
 * We implement `#pragma GCC diagnostic push` by copying the levels array and
 * pushing it onto this stack.
 *
 * TODO copy lazily. push/pop don't need to match up, pop without push restores
 * command-line options. So when we see a diagnostic change and the stack is
 * empty, implicitly push.
 */
//static warning_level_t** warning_levels_stack;
//static size_t warning_levels_stack_count;
//static size_t warning_levels_stack_capacity;

bool option_debug_info;
bool optimization;
int dump_ast;
static bool werror;

/*
 * The warning table is a mapping of command-line warning strings (not
 * including `-W` and other prefixes) to the warning enum name.
 *
 * We used a fixed-size open hashtable with linear probing. We also don't
 * intern these strings. We care more about startup time than parse time
 * because we have way more warnings than are likely to be specified on any
 * command-line so building this table needs to be fast.
 *
 * In a "real" compiler it would make sense to make this a pre-computed perfect
 * hashtable. Unfortunately that isn't practical for Onramp. We really only
 * have cci/1 and cpp/1 to work with.
 */
#define warning_table_capacity 256 // must be power of two
#define warning_table_mask (warning_table_capacity - 1)
static const char* warning_table_args[warning_table_capacity]; // NULL means empty bucket
static warning_t warning_table_enums[warning_table_capacity];
static size_t warning_table_count;

/*
 * Adds a warning to the warning table.
 */
static void warning_add(const char* arg, warning_t warning, warning_level_t default_level) {
    if (warning_table_count++ == (warning_table_capacity >> 1)) {
        fatal("Internal error: warning_table_capacity needs to be increased.");
    }

    uint32_t bucket = fnv1a_cstr(arg) & warning_table_mask;
    while (warning_table_args[bucket]) {
        bucket = (bucket + 1) & warning_table_mask;
    }

    warning_table_args[bucket] = arg;
    warning_table_enums[bucket] = warning;

    warning_args[warning] = arg;
    warning_levels[warning] = default_level;
}

/*
 * Finds the warning for the given arg.
 *
 * Returns warning_invalid if not found.
 */
static warning_t warning_find(const char* arg) {
    uint32_t bucket = fnv1a_cstr(arg) & warning_table_mask;
    for (;;) {
        if (!warning_table_args[bucket])
            return warning_invalid;
        if (0 == strcmp(arg, warning_table_args[bucket])) {
            return warning_table_enums[bucket];
        }
        bucket = (bucket + 1) & warning_table_mask;
    }
    // unreachable
    return warning_invalid;
}

static void warnings_init(void) {
    warning_args = calloc(warning_count, sizeof(const char*));
    warning_specs = calloc(warning_count, sizeof(warning_spec_t));
    warning_levels = calloc(warning_count, sizeof(warning_level_t));

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

    // incorrect code
    warning_add("implicit-int", warning_implicit_int, warning_level_warn);
    warning_add("zero-length-array", warning_zero_length_array, warning_level_warn);
    warning_add("discarded-qualifiers", warning_discarded_qualifiers, warning_level_warn);

    // extension usage
    warning_add("statement-expressions", warning_statement_expressions, warning_level_error);
    warning_add("asm", warning_extra_keywords, warning_level_error); // TODO also need to support -fasm
    warning_add("anonymous-tags", warning_anonymous_tags, warning_level_error);
    warning_add("pointer-arith", warning_pointer_arith, warning_level_error);
}

static void warnings_destroy(void) {
    free(warning_specs);
    // TODO free warning_levels_stack and contents
    free(warning_levels);
}

static bool warning_parse(const char* arg) {
    if (!starts_with(arg, "-W"))
        return false;

    // figure out what level this is specifying
    warning_spec_t spec;
    if (starts_with(arg, "-Wno-error=")) {
        spec = warning_spec_no_error;
        arg += strlen("-Wno-error=");
    } else if (starts_with(arg, "-Werror=")) {
        spec = warning_spec_error;
        arg += strlen("-Werror=");
    } else if (starts_with(arg, "-Wno-")) {
        spec = warning_spec_off;
        arg += strlen("-Wno-");
    } else {
        spec = warning_spec_on;
        arg += strlen("-W");
    }

    // lookup the option
    warning_t warning = warning_find(arg);
    if (warning == warning_invalid)
        return false; // unrecognized option
    warning_specs[warning] = spec;
    return true;
}

static void warning_apply_spec(warning_t warning, warning_spec_t spec) {
    switch (spec) {
        case warning_spec_none:
            return;
        case warning_spec_on:
            warning_levels[warning] = werror ? warning_level_error : warning_level_warn;
            return;
        case warning_spec_off:
            warning_levels[warning] = warning_level_off;
            return;
        case warning_spec_error:
            warning_levels[warning] = warning_level_error;
            return;
        case warning_spec_no_error:
            // -Wno-error turns errors into warnings, but it doesn't turn on
            // the warning if it wasn't already on.
            if (warning_levels[warning] == warning_level_error)
                warning_levels[warning] = warning_level_warn;
            return;
        default:
            break;
    }
    fatal("Internal error: Invalid warning spec");
}

void warnings_resolve(void) {

    // apply -Wall
    // TODO these are not really in -Wall, just testing for now
    warning_apply_spec(warning_implicit_int, warning_specs[warning_all]);
    warning_apply_spec(warning_statement_expressions, warning_specs[warning_all]);
    warning_apply_spec(warning_extra_keywords, warning_specs[warning_all]);

    // apply -Wextra
    // TODO same thing
    warning_apply_spec(warning_implicit_int, warning_specs[warning_extra]);
    warning_apply_spec(warning_statement_expressions, warning_specs[warning_extra]);
    warning_apply_spec(warning_extra_keywords, warning_specs[warning_extra]);

    // apply -Wpedantic
    // TODO same thing
    warning_apply_spec(warning_implicit_int, warning_specs[warning_extra]);
    warning_apply_spec(warning_statement_expressions, warning_specs[warning_extra]);
    warning_apply_spec(warning_extra_keywords, warning_specs[warning_extra]);

    // apply other options
    for (int warning = 0; warning < warning_count; ++warning) {
        warning_apply_spec(warning, warning_specs[warning]);
    }

    // TODO add a push here; pragma pop with no push restores command-line options
}



/****************************************
 * Flags
 ****************************************/

typedef enum flag_spec_t {
    flag_spec_none,      // not specified
    flag_spec_on,        // specified with -f...
    flag_spec_off,       // specified with -fno-...
} flag_spec_t;

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
}

void options_destroy(void) {
    flags_destroy();
    warnings_destroy();
}

void warn(warning_t warning, struct token_t* token, const char* message, ...) {
    warning_level_t level = warning_levels[warning];
    if (level == warning_level_off)
        return;

    va_list args;
    va_start(args, message);
    if (level == warning_level_error)
        vfatal_token(token, message, args);

    // the libo warning/error code uses these current_line/current_filename
    // globals. for now we just push and set them. later we should stop using
    // the libo error handling entirely and do this ourselves (especially when
    // we get to the point of print nice error messages with context from the
    // original source.)
    int old_line = current_line;
    char* old_filename = current_filename;
    current_line = token->line;
    current_filename = (char*)string_cstr(token->filename);

    char* formatted;
    if (-1 == vasprintf(&formatted, message, args))
        fatal("Out of memory.");
    va_end(args);

    const char* arg = warning_args[warning];
    if (!arg)
        fatal("Internal error: missing warning string for warning %i", (int)warning);
    print_warning("%s (-W%s)", formatted, arg);

    free(formatted);
    current_line = old_line;
    current_filename = old_filename;
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

static bool options_parse_misc(const char* arg) {
    if (0 == strcmp(arg, "-Werror")) {
        werror = true;
        return true;
    }
    if (0 == strcmp(arg, "-Wno-error")) {
        werror = false;
        return true;
    }

    // cc doesn't pass along suffixes for -g or -O
    if (0 == strcmp(arg, "-g")) {
        option_debug_info = true;
        return true;
    }
    if (0 == strcmp(arg, "-O")) {
        optimization = true;
        return true;
    }

    return false;
}

bool options_parse(const char* arg) {
    if (warning_parse(arg))
        return true;
    if (flag_parse(arg))
        return true;
    if (options_parse_dump_ast(arg))
        return true;
    if (options_parse_misc(arg))
        return true;
    return false;
}

void options_resolve(void) {
    warnings_resolve();
}
