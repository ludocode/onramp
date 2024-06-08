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

#include "libo-error.h"
#include "common.h"
#include "token.h"

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
 * - Otherwise, if -Wpedantic or -Werror is specified, we treat extension usage
 *   as an error. (You should specify both of these options if you want to
 *   write conforming C code.)
 *
 * - Otherwise, we treat extension usage as a warning.
 *
 * As in GNU C, __extension__ disables all extension warnings until the end of
 * the expression or statement.
 *
 * (TODO most of the above is not implemented yet, but this is the goal.)
 */

/*
 * Defines how the options are specified.
 *
 * Warnings can be specified on the command-line in multiple ways. For a
 * warning "foo", you can specify:
 *
 * - -Wfoo            Treats "foo" as a warning, or an error if -Werror is specified
 * - -Wno-foo         Disables warnings about "foo"
 * - -Werror=foo      Treats "foo" as an error
 * - -Wno-error=foo   Treats "foo" as a warning regardless of -Werror
 *
 * We capture the specificity of a warning here. This works for groups as well.
 *
 * Once all command-line options are parsed, we then walk the groups and the
 * individual warnings to set the computed level of all warnings.
 */
typedef enum warning_spec_t {
    warning_spec_none,      // not specified
    warning_spec_on,        // specified with -W...
    warning_spec_off,       // specified with -Wno-...
    warning_spec_error,     // specified with -Werror=...
    warning_spec_no_error,  // specified with -Wno-error=...
} warning_spec_t;

typedef struct warning_info_t {
    const char* arg;
    warning_spec_t spec;
} warning_info_t;

static warning_info_t* warning_infos;

typedef enum warning_level_t {
    warning_level_off = 0,
    warning_level_warn,
    warning_level_error,
} warning_level_t;

/*
 * Warning levels are stored in an array which is itself stored in a stack.
 *
 * We implement `#pragma GCC diagnostic push` by copying the array and pushing
 * it onto the stack.
 */
static warning_level_t** warning_levels;
static size_t warning_levels_count;
static size_t warning_levels_capacity;

static void warning_init(warning_t warning, const char* arg, warning_level_t level) {
    warning_info_t* info = &warning_infos[warning];
    info->arg = arg;
    info->spec = warning_spec_none;
    warning_levels[0][warning] = level;
}

static void warnings_init(void) {
    warning_infos = calloc(warning_count, sizeof(warning_info_t));
    warning_levels = malloc(sizeof(warning_level_t*));
    warning_levels[0] = calloc(warning_count, sizeof(warning_level_t));
    warning_levels_count = 1;
    warning_levels_capacity = 1;

    // groups
    warning_init(warning_all, "all", warning_level_off);
    warning_init(warning_extra, "extra", warning_level_off);
    warning_init(warning_pedantic, "pedantic", warning_level_off);

    // deprecated
    warning_init(warning_implicit_int, "implicit-int", warning_level_warn);

    // extension usage
    warning_init(warning_statement_expressions, "statement-expressions", warning_level_off);
    warning_init(warning_extra_keywords, "asm", warning_level_off);
}



bool optimization;

int dump_ast;

void options_init(void) {
    warnings_init();

    //optimization = true;
}

void options_destroy(void) {
    free(warning_infos);
    for (size_t i = 0; i < warning_levels_count; ++i)
        free(warning_levels[i]);
    free(warning_levels);
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
