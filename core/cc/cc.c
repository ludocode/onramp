/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2025 Fraser Heavy Software
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



/*
 * This is the Onramp compiler driver. It is written in omC.
 *
 * It runs the preprocessor, compiler, code generator, assembler and linker
 * all-in-one. Its command-line argument syntax is similar to POSIX-style C
 * compilers such as GCC.
 *
 * This is implemented all in one file because it was originally intended to be
 * bootstrapped before ld/1, the linker that adds file scope. This is no longer
 * the case as it now depends on libc/1 so it could expand to multiple files if
 * necessary. For now I haven't bothered.
 */



/*
 * Headers
 */

#include <ctype.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#ifndef __onramp__
    #include <unistd.h>
    #include <sys/wait.h>
#endif

#include "libo-error.h"
#include "libo-util.h"

#ifdef __onramp_libc_omc__
// TODO we don't have strcasecmp() during bootstrapping but we probably need it
// in case the platform doesn't support lowercase filenames. We don't
// officially support such platforms (we require most of ASCII and long
// filenames) but it would be nice to be able to support them anyway.
#define strcasecmp strcmp
#endif

extern char** environ;

// TODO dedupe these with libc/2
void* __memdup(const void* p, size_t count);
extern int* __process_info_table;
#define PIT_VERSION 0
#define PIT_BREAK 1
#define PIT_EXIT 2
#define PIT_INPUT 3
#define PIT_OUTPUT 4
#define PIT_ERROR 5
#define PIT_ARGS 6
#define PIT_ENVIRON 7
#define PIT_WORKDIR 8

static void parse_options(char** argv);



/*
 * Globals
 */

#define MODE_LINK 1
#define MODE_ASSEMBLE 2
#define MODE_CODEGEN 3
#define MODE_COMPILE 4
#define MODE_PREPROCESS 5

#define TYPE_OO 1  // .oo and .oa
#define TYPE_OS 2
#define TYPE_OIR 3
#define TYPE_I 4
#define TYPE_C 5

#define FILEARGS_BUFFER_SIZE 256

// arguments files
static char* fileargs_buffer;
char** fileargs;
size_t fileargs_count;
size_t fileargs_capacity;

// options
static const char* output_filename;
static int mode;
static bool verbose;
static bool disable_run;
static bool nostdinc;
static bool nostdlib;
static bool nostddef;
static bool debug_info;
static bool optimize;
static bool emit_ir;
static bool dump_macros;
static char* wrap_header;

// tools and libc files to use
static char* tool_cpp;
static char* tool_cci;
static char* tool_cg;
static char* tool_as;
static char* tool_ld;
static char* libc_archive;
static char* libc_include;

// array of input filenames
static char** inputs;
static size_t inputs_count;
static size_t inputs_capacity;

// array of cpp options (e.g. -I, -D, -U, -include)
static char** cpp_opts;
static size_t cpp_opts_count;
static size_t cpp_opts_capacity;

// array of cci options
static char** cci_opts;
static size_t cci_opts_count;
static size_t cci_opts_capacity;

// array of temporary files (owning allocated strings)
static char** temp_files;
static size_t temp_files_count;
static size_t temp_files_capacity;

// array of linker inputs
static char** objects;
static size_t objects_count;
static size_t objects_capacity;



/*
 * Error handling
 */

static void delete_temp_files(void);

// We define our own fatal() function that cleans up temporary files before
// exiting.
// TODO it would be nice if fatal() in libo had some kind of cleanup callback
// built-in so we could just call the real fatal(). This is a bit challenging
// without function pointer support.
_Noreturn
void fatal_cleanup(const char* message) {
    delete_temp_files();
    fatal(message);
}

// We don't want to accidentally use fatal().
#define fatal ERROR_FATAL_DISABLED



/*
 * Misc string functions
 */

// TODO maybe put this in libo
static char* strdup_checked(char* str) {
    char* ret = strdup(str);
    if (ret == NULL) {
        fatal_cleanup("Out of memory.");
    }
    return ret;
}

// This is similar to libo's vector_append(). We don't have libo/1 here, and we
// don't have structs either so we can't conveniently group the array fields
// together. It's better to just keep them separate.
static void string_array_append(char*** array, size_t* count,
        size_t* capacity, char* string)
{
    // grow if necessary
    if (*count == *capacity) {

        // calculate new capacity
        size_t new_capacity;
        if (*capacity == 0) {
            new_capacity = 8;
        }
        if (*capacity != 0) {
            new_capacity = (*capacity * 2);
            if (new_capacity <= *capacity) {
                fatal_cleanup("Out of address space.");
            }
        }

        // realloc
        *array = realloc(*array, new_capacity * sizeof(char*));
        if (*array == NULL) {
            fatal_cleanup("Out of memory.");
        }
        *capacity = new_capacity;

    }

    // append the string
    *(*array + *count) = string;
    *count = (*count + 1);
}

/**
 * Frees a string array in which the strings were allocated.
 */
static void string_array_free(char** array, size_t count) {
    while (count > 0) {
        count = (count - 1);
        free(*(array + count));
    }
    free(array);
}

/**
 * Returns true if string starts with prefix.
 * TODO put this in libo, cci/2 also uses it. can also optimize it there, this implementation scans twice
 */
static bool starts_with(const char* string, const char* prefix) {
    size_t len = strlen(prefix);
    if (len > strlen(string)) {
        return false;
    }
    return 0 == memcmp(string, prefix, len);
}



/*
 * Command-line argument parsing
 */

static bool try_parse_output(char*** argv) {
    if (!starts_with(**argv, "-o")) {
        return false;
    }
    if (output_filename != 0) {
        fatal_cleanup("-o can only be specified once.");
    }

    // The output file can be provided as the next option or appended directly
    // to -o.
    bool appended = (*(**argv + 2) != 0);
    if (appended) {
        output_filename = (**argv + 2);
    }
    if (!appended) {
        *argv = (*argv + 1);
        if (**argv == 0) {
            fatal_cleanup("-o must be followed by a filename.");
        }
        output_filename = **argv;
    }
    *argv = (*argv + 1);
    return true;
}

static bool try_parse_mode(char*** argv) {
    if (0 == strcmp(**argv, "-E")) {
        if (mode != MODE_LINK) {
            fatal_cleanup("Only one of -c, -E and -S can be provided.");
        }
        mode = MODE_PREPROCESS;
        *argv = (*argv + 1);
        return true;
    }
    if (0 == strcmp(**argv, "-S")) {
        if (mode != MODE_LINK) {
            fatal_cleanup("Only one of -c, -E and -S can be provided.");
        }
        mode = MODE_CODEGEN;
        *argv = (*argv + 1);
        return true;
    }
    if (0 == strcmp(**argv, "-c")) {
        if (mode != MODE_LINK) {
            fatal_cleanup("Only one of -c, -E and -S can be provided.");
        }
        mode = MODE_ASSEMBLE;
        *argv = (*argv + 1);
        return true;
    }
    return false;
}

static bool try_parse_warnings(char*** argv) {
    // TODO some warnings are for cpp, others are for cci, and maybe some are
    // for both. We should also probably warn against unrecognized warnings.
    // We may also want to scan the argument list for -Werror first.
    // For now we don't support any warning options at all.
    return false;
}

static bool try_parse_cpp_opts(char*** argv) {
    if (!starts_with(**argv, "-I")) {
        if (!starts_with(**argv, "-D")) {
            if (!starts_with(**argv, "-U")) {
                return false;
            }
        }
    }

    string_array_append(&cpp_opts, &cpp_opts_count, &cpp_opts_capacity, **argv);

    // -I, -D and -U can have the option appended directly or provided
    // separately. If provided separately, we have to append that option as
    // well.
    if (*(**argv + 2) == 0) {
        // option is separate
        const char* opt = **argv;
        *argv = (*argv + 1);
        if (**argv == NULL) {
            // error, option missing
            if (0 == strcmp(opt, "-I")) {
                fatal_cleanup("-I must be followed by a path.");
            }
            fatal_cleanup("-D or -U must be followed by a macro name.");
        }
        string_array_append(&cpp_opts, &cpp_opts_count, &cpp_opts_capacity, **argv);
    }

    *argv = (*argv + 1);
    return true;
}

static bool try_parse_include(char*** argv) {

    // add -include
    if (0 != strcmp("-include", **argv)) {
        return false;
    }
    string_array_append(&cpp_opts, &cpp_opts_count, &cpp_opts_capacity, **argv);
    *argv = (*argv + 1);

    // add the filename
    if (**argv == NULL) {
        fatal_cleanup("`-include` must be followed by an include file.");
    }
    string_array_append(&cpp_opts, &cpp_opts_count, &cpp_opts_capacity, **argv);
    *argv = (*argv + 1);

    return true;
}

static bool is_cci_option(char* arg) {
    // Check if it's an option cci cares about. We accept everything starting
    // with `-f` or `-W`; we let cci sort out the details.
    // Also note that the option to `-std` cannot be separate; `=` is required.
// TODO there are some warnings that only cpp needs and some that both cpp
// and cci need. We need to check those first.
    if (starts_with(arg, "-std=")) {return true;}
    if (starts_with(arg, "-f")) {return true;}
    if (starts_with(arg, "-W")) {return true;}
    if (starts_with(arg, "-pedantic")) {return true;}
    if (0 == strcmp(arg, "-ansi")) {return true;}
    return false;
}

static bool try_parse_cci_opts(char*** argv) {
    if (!is_cci_option(**argv)) {
        return false;
    }
    string_array_append(&cci_opts, &cci_opts_count, &cci_opts_capacity, **argv);
    *argv = (*argv + 1);
    return true;
}

// Parses an option that takes an extra string
static bool try_parse_option_string(char*** argv, char* prefix, char** str) {
    if (!starts_with(**argv, prefix)) {
        return false;
    }
    size_t len = strlen(prefix);

    // The string can be given with an = sign (e.g. `-with-cpp=/path/to/cpp.oe`)
    // or separately (e.g. `-with-cpp /path/to/cpp.oe`).

    // check for =
    if (*(**argv + len) == '=') {
        free(*str);
        *str = strdup_checked(**argv + (len + 1));
        *argv = (*argv + 1);
        return true;
    }

    // check for separate argument
    if (*(**argv + len) == 0) {
        *argv = (*argv + 1);
        if (**argv == NULL) {
            fatal_cleanup("`-with` option must be followed by a path");
        }
        free(*str);
        *str = strdup_checked(**argv);
        *argv = (*argv + 1);
    }

    return true;
}

static bool try_parse_with(char*** argv) {
    if (!starts_with(**argv, "-with")) {
        return false;
    }

    if (try_parse_option_string(argv, "-with-cpp", &tool_cpp)) {return true;}
    if (try_parse_option_string(argv, "-with-cci", &tool_cci)) {return true;}
    if (try_parse_option_string(argv, "-with-cg", &tool_cg)) {return true;}
    if (try_parse_option_string(argv, "-with-as", &tool_as)) {return true;}
    if (try_parse_option_string(argv, "-with-ld", &tool_ld)) {return true;}
    return false;
}

static bool try_parse_misc_option(char*** argv, const char* string, bool* /*nullable*/ option) {
    if (0 == strcmp(string, **argv)) {
        if (option) {
            *option = true;
        }
        *argv = (*argv + 1);
        return true;
    }
    return false;
}

static bool try_parse_misc(char*** argv) {
    if (try_parse_misc_option(argv, "-v", &verbose)) {return true;}
    if (try_parse_misc_option(argv, "-g", &debug_info)) {return true;}
    if (try_parse_misc_option(argv, "-nostdinc", &nostdinc)) {return true;}
    if (try_parse_misc_option(argv, "-nostdlib", &nostdlib)) {return true;}
    if (try_parse_misc_option(argv, "-nostddef", &nostddef)) {return true;}
    if (try_parse_misc_option(argv, "-dM", &dump_macros)) {return true;}
    if (try_parse_misc_option(argv, "-emit-ir", &emit_ir)) {return true;}

    if (try_parse_misc_option(argv, "-###", &disable_run)) {
        // -### implies -v
        verbose = true;
        return true;
    }

    if (try_parse_misc_option(argv, "-O", &optimize)) {return true;}
    if (try_parse_misc_option(argv, "-Og", &optimize)) {return true;}
    if (try_parse_misc_option(argv, "-O1", &optimize)) {return true;}
    if (try_parse_misc_option(argv, "-O2", &optimize)) {return true;}
    if (try_parse_misc_option(argv, "-O3", &optimize)) {return true;}
    if (try_parse_misc_option(argv, "-Ofast", &optimize)) {return true;}
    if (try_parse_misc_option(argv, "-Os", &optimize)) {return true;}
    if (try_parse_misc_option(argv, "-Oz", &optimize)) {return true;}

    if (try_parse_misc_option(argv, "-O0", NULL)) {
        optimize = false;
        *argv = (*argv + 1);
        return true;
    }

    if (try_parse_option_string(argv, "-wrap-header", &wrap_header)) {return true;}

    if (try_parse_misc_option(argv, "--version", NULL)) {
        puts("Onramp (cc) version 0");
        exit(0);
    }

    return false;
}

static char* create_tool_path(const char* path, size_t path_len, const char* filename) {
    size_t filename_len = strlen(filename);
    size_t full_len = (path_len + filename_len);
    char* ret = malloc(full_len + 1);
    if (ret == NULL) {
        fatal_cleanup("Out of memory.");
    }
    memcpy(ret, path, path_len);
    memcpy(ret + path_len, filename, filename_len);
    *(ret + full_len) = 0;
    return ret;
}

static void set_default_options(const char* cc_filename) {

    // the default mode is link if no other options are specified
    mode = MODE_LINK;

    // the default toolchain is relative to the path of cc
    const char* path_end = strrchr(cc_filename, '/');
    if (path_end == NULL) {
        // TODO should we use the working directory?
        fatal_cleanup("Failed to determine path to cc binary.");
    }
    path_end = (path_end + 1);
    size_t path_len = (path_end - cc_filename);

    #ifdef __onramp__
    tool_cpp = create_tool_path(cc_filename, path_len, "cpp.oe");
    tool_cci = create_tool_path(cc_filename, path_len, "cci.oe");
    // TODO for backwards compatibility we don't run a cg tool by default. cg/1
    // doesn't exist yet and we need to update all the build scripts first.
    //tool_cg = create_tool_path(cc_filename, path_len, "cg.oe");
    tool_as = create_tool_path(cc_filename, path_len, "as.oe");
    tool_ld = create_tool_path(cc_filename, path_len, "ld.oe");
    #endif
    #ifndef __onramp__
    tool_cpp = create_tool_path(cc_filename, path_len, "cpp");
    tool_cci = create_tool_path(cc_filename, path_len, "cci");
    //tool_cg = create_tool_path(cc_filename, path_len, "cg");
    tool_as = create_tool_path(cc_filename, path_len, "as");
    tool_ld = create_tool_path(cc_filename, path_len, "ld");
    #endif

    libc_archive = create_tool_path(cc_filename, path_len, "../lib/libc.oa");
    libc_include = create_tool_path(cc_filename, path_len, "../include");
}

static void parse_options_file(const char* filename) {
    if (fileargs_buffer == NULL) {
        fileargs_buffer = malloc(FILEARGS_BUFFER_SIZE);
        if (fileargs_buffer == NULL) {
            fatal_cleanup("Out of memory.");
        }
    }

    FILE* file = fopen(filename, "rb");
    if (!file) {
        fatal_cleanup("Failed to open options file.");
    }

    // We support multiple @file arguments and we need all their strings to
    // stick around until the end. To keep this simple we add them all to one
    // big string array and only pass the just-parsed subset of it to
    // parse_options().
    size_t fileargs_start = fileargs_count;

    size_t index = 0;
    *fileargs_buffer = 0;
    while (true) {
        int c = fgetc(file);

        // see if it's the end of an option
        if ((c == EOF) | isspace(c)) {
            if (index > 0) {
                *(fileargs_buffer + index) = 0;
                string_array_append(&fileargs, &fileargs_count,
                        &fileargs_capacity, strdup_checked(fileargs_buffer));
                index = 0;
            }
            if (c == EOF) {
                break;
            }
            continue;
        }

        // otherwise add it to the buffer
        *(fileargs_buffer + index) = c;
        index = (index + 1);
        if (index == FILEARGS_BUFFER_SIZE) {
            fatal_cleanup("Argument in arguments file is too long.");
        }
    }
    fclose(file);

    // We temporarily append a null-terminator just for options parsing.
    string_array_append(&fileargs, &fileargs_count, &fileargs_capacity, NULL);
    parse_options(fileargs + fileargs_start);
    fileargs_count = (fileargs_count - 1);
}

static void parse_options(char** argv) {

    while (*argv != 0) {

        // check for an options file
        if (starts_with(*argv, "@")) {
            parse_options_file(*argv + 1);
            argv = (argv + 1);
            continue;
        }

        // check for an option
        if (starts_with(*argv, "-")) {

            // see if it's supported
            if (try_parse_output(&argv)) {
                continue;
            }
            if (try_parse_mode(&argv)) {
                continue;
            }
            if (try_parse_warnings(&argv)) {
                continue;
            }
            if (try_parse_cpp_opts(&argv)) {
                continue;
            }
            if (try_parse_include(&argv)) {
                continue;
            }
            if (try_parse_cci_opts(&argv)) {
                continue;
            }
            if (try_parse_with(&argv)) {
                continue;
            }
            if (try_parse_misc(&argv)) {
                continue;
            }

            // if we don't recognize it, fatal error
            if (**argv == '-') {
                fputs("Option: ", stderr);
                fputs(*argv, stderr);
                fputc('\n', stderr);
                fatal_cleanup("Unsupported command-line option.");
            }
        }

        // otherwise it's an input file
        string_array_append(&inputs, &inputs_count,
                &inputs_capacity, *argv);
        argv = (argv + 1);
    }
}

static void check_options(void) {
    if (inputs_count == 0) {
        fatal_cleanup("No input files.");
    }

    if (mode != MODE_LINK) {
        if (inputs_count != 1) {
            //printf("%zi %s %s\n",inputs_count,*inputs,*(inputs+1));
            fatal_cleanup("Exactly one input file must be specified if using -c, -E or -S.");
        }
    }

    // If -emit-ir was specified, -S must have been specified as well, and we
    // change the mode to codegen.
    if (emit_ir) {
        if (mode != MODE_CODEGEN) {
            fatal_cleanup("-emit-ir requires -S.");
        }
        mode = MODE_COMPILE;
    }

    if (output_filename == NULL) {
        // TODO we should default to a.out if we're wrapped for posix.
        // otherwise we should default to the input filename basename plus .exe
        // or .oe.
        /*
        if (mode == MODE_LINK) {
            output_filename = "a.out";
        }
        if (mode != MODE_LINK)*/ {
            fatal_cleanup("No output file.");
        }
    }

    // Default include paths.
    if (!nostdinc) {
        string_array_append(&cpp_opts, &cpp_opts_count, &cpp_opts_capacity, "-I");
        string_array_append(&cpp_opts, &cpp_opts_count, &cpp_opts_capacity, libc_include);
    }

    // Default libc force-included header. This is disabled if any of
    // -nostdinc, -nostdlib or -nostddef are passed.
    if ((!nostdinc & !nostdlib) & !nostddef) {
        string_array_append(&cpp_opts, &cpp_opts_count, &cpp_opts_capacity, "-include");
        string_array_append(&cpp_opts, &cpp_opts_count, &cpp_opts_capacity, "__onramp/__predef.h");
    }

    // Default macros. We don't define __onramp_cpp__ because cpp defines it
    // itself. Instead we forward -nostddef to it to inhibit them.
    if (nostddef) {
        string_array_append(&cpp_opts, &cpp_opts_count, &cpp_opts_capacity, "-nostddef");
    }
    if (!nostddef) {
        string_array_append(&cpp_opts, &cpp_opts_count, &cpp_opts_capacity, "-D__onramp__=1");
        string_array_append(&cpp_opts, &cpp_opts_count, &cpp_opts_capacity, "-D__onramp_cci__=1");
    }

    if (dump_macros & (mode != MODE_PREPROCESS)) {
        fatal_cleanup("-dM requires -E.");
    }
}

static void free_options(void) {
    free(inputs);
    free(cpp_opts);
    free(cci_opts);
    free(tool_cpp);
    free(tool_cci);
    free(tool_as);
    free(tool_ld);
    free(fileargs_buffer);
    string_array_free(fileargs, fileargs_count);
}



/*
 * Running
 */

static int file_type(const char* name) {
    char* extension = strrchr(name, '.');
    if (extension != NULL) {
        extension = (extension + 1);

        // We assume .C is a C file on a platform that uses uppercase filenames
        // (as opposed to, say, GCC which treats it as a C++ file.)
        if (0 == strcasecmp(extension, "c")) { return TYPE_C; }

        if (0 == strcasecmp(extension, "i")) { return TYPE_I; }

        if (0 == strcasecmp(extension, "oir")) { return TYPE_OIR; }

        // We support `.o` and `.s` file extensions to be compatible with a
        // typical UNIX buildsystem. The files of course must contain Onramp
        // bytecode or assembly.

        if (0 == strcasecmp(extension, "os")) { return TYPE_OS; }
        if (0 == strcasecmp(extension, "s")) { return TYPE_OS; }

        // We don't differentiate between .oo and .oa files.
        if (0 == strcasecmp(extension, "oo")) { return TYPE_OO; }
        if (0 == strcasecmp(extension, "o")) { return TYPE_OO; }
        if (0 == strcasecmp(extension, "oa")) { return TYPE_OO; }
        if (0 == strcasecmp(extension, "a")) { return TYPE_OO; }
    }

    fputs("In file: ", stderr);
    fputs(name, stderr);
    fputc('\n', stderr);
    fatal_cleanup("Unknown file type.");
    return -1;
}

/**
 * Makes a temporary output file with the given extension.
 *
 * The file is placed in the same directory as the output. The filename is the
 * same as the output file but prefixed with `.tmp.` and suffixed with the
 * given extension.
 */
static char* make_temp_filename(const char* extension) {
    const char* prefix = ".tmp.";
    size_t prefix_len = strlen(prefix);
    size_t extension_len = strlen(extension);

    size_t total = ((strlen(output_filename) + prefix_len) + (extension_len + 1));
    char* ret = malloc(total);
    char* pos = ret;

    // separate the filename from the path, appending the path to ret
    const char* filename = strrchr(output_filename, '/');
    if (filename != NULL) {
        filename = (filename + 1);
        size_t pathlen = (filename - output_filename);
        memcpy(ret, output_filename, pathlen);
        pos = (ret + pathlen);
    }
    if (filename == NULL) {
        filename = output_filename;
    }
    size_t filename_len = strlen(filename);

    // append the rest
    memcpy(pos, prefix, prefix_len);
    pos = (pos + prefix_len);
    memcpy(pos, filename, filename_len);
    pos = (pos + filename_len);
    strcpy(pos, extension);

    string_array_append(&temp_files, &temp_files_count, &temp_files_capacity, ret);
    return ret;
}

static void delete_temp_files(void) {
    while (temp_files_count > 0) {
        temp_files_count = (temp_files_count - 1);
        char* temp_file = *(temp_files + temp_files_count);
        if (!disable_run) {
            // TODO remove in C, unlink in VM
            //remove(temp_file);
        }
        free(temp_file);
    }
    free(temp_files);
}

#ifdef __onramp__
static void run_onramp(size_t argc, char** argv) {
//    fputs("spawning: ", stdout);
//    puts(*argv);

    // TODO: We should be making a copy of argv and environ here in case our
    // child process modifies them. We happen to know that none of our Onramp
    // tools modify them so for now we don't worry about it.

    // open the child program
    FILE* file = fopen(*argv, "rb");
    if (!file) {
        fatal_cleanup("Child program not found.");
    }

    // get the program size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // allocate the program
    // TODO eventually this should allocate the program plus stack space. For
    // now the program image is copied into a new heap by __onramp_spawn_pit().
    char* program = malloc(size);
    if (program == NULL) {
        fatal_cleanup("Out of memory.");
    }

    // read the program into the child memory
    char* p = program;
    char* end = (p + size);
    while (p != end) {
        size_t step = fread(p, 1, end - p, file);
        if (step == 0) {
            fatal_cleanup("Failed to read child program.");
        }
        p = (p + step);
    }
    fclose(file);

    // allocate a process information table for the child as a copy of ours
    int* parent_pit = __process_info_table;
    int* child_pit = __memdup(parent_pit, sizeof(int) * 12);

    // setup the child pit
    *(child_pit + PIT_ARGS) = (int)argv;
    *(child_pit + PIT_ENVIRON) = (int)environ;

    // run it
    int ret = __onramp_spawn_pit(program, size, child_pit, *argv);

    // clean up
    //free(child_pit);
    __onramp_load_debug(program, NULL);
    //free(program);

    if (ret != 0) {
        // Child program failed. Assume it printed an error message; just clean
        // up the temporary files and return the same result.
        delete_temp_files();
        exit(ret);
    }
}
#endif

#ifndef __onramp__
static void run_posix(size_t argc, char** argv) {
    char** old_argv = argv;
    char** new_argv = NULL;

    // Check if the name of the executable has a .oe extension
    size_t len = strlen(*argv);
    if (len > 3 && 0 == strcmp(*argv + len - 3, ".oe")) {

        // It does. Insert "onrampvm" at the front of the args.
        char** new_argv = malloc(sizeof(char*) * (argc + 2));
        *new_argv = "onrampvm";
        memcpy(new_argv + 1, argv, sizeof(char*) * argc);
        *(new_argv + (1 + argc)) = NULL;
        argv = new_argv;
    }

    // Run the subprocess
    pid_t pid;
    if (0 != (new_argv ? posix_spawnp : posix_spawn)(&pid, *argv, NULL, NULL, argv, environ)) {
        fputs("Attempting to run: ", stderr);
        fputs(*old_argv, stderr);
        fputc('\n', stderr);
        fatal_cleanup("Failed to spawn subprocess");
    }
    free(new_argv);

    // Wait until it's done
    int status;
    wait(&status);
    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    if (exit_code) {
        // Assume the subprocess printed some kind of error. We don't print
        // anything; just exit.
        delete_temp_files();
        exit(exit_code);
    }
}
#endif

static void run(size_t argc, char** argv) {
    if (verbose) {
        size_t i = 0;
        while (i < argc) {
            if (i > 0) {
                fputc(' ', stdout);
            }
            fputs(*(argv + i), stdout);
            i = (i + 1);
        }
        fputc('\n', stdout);
    }

    if (disable_run) {
        return;
    }

    // flush our streams before running the subprogram to make sure our output
    // isn't mixed with it.
    // TODO probably this should move to the libc __onramp_spawn_pit; or if not
    // libc/0, at least libc/2 should flush any streams associated with the
    // file descriptors given to the subprogram
    fflush(stdout);
    fflush(stderr);

    // TODO we should probably check here that the tool actually exists, would
    // make for nicer error messages

    #ifdef __onramp__
    run_onramp(argc, argv);
    #endif

    #ifndef __onramp__
    run_posix(argc, argv);
    #endif
}

static void preprocess_file(const char* input, const char* output) {
    char** args = 0;
    size_t args_count = 0;
    size_t args_capacity = 0;

    string_array_append(&args, &args_count, &args_capacity, tool_cpp);

    if (dump_macros) {
        string_array_append(&args, &args_count, &args_capacity, "-dM");
    }

    size_t i = 0;
    while (i < cpp_opts_count) {
        string_array_append(&args, &args_count, &args_capacity, *(cpp_opts + i));
        i = (i + 1);
    }

    string_array_append(&args, &args_count, &args_capacity, (char*)input);
    string_array_append(&args, &args_count, &args_capacity, "-o");
    string_array_append(&args, &args_count, &args_capacity, (char*)output);

    string_array_append(&args, &args_count, &args_capacity, NULL);
    run(args_count - 1, args);

    free(args);
}

static void compile_file(const char* input, const char* output) {
    char** args = 0;
    size_t args_count = 0;
    size_t args_capacity = 0;

    string_array_append(&args, &args_count, &args_capacity, tool_cci);

    if (debug_info) {
        string_array_append(&args, &args_count, &args_capacity, "-g");
    }
    if (optimize) {
        string_array_append(&args, &args_count, &args_capacity, "-O");
    }

    string_array_append(&args, &args_count, &args_capacity, (char*)input);
    string_array_append(&args, &args_count, &args_capacity, "-o");
    string_array_append(&args, &args_count, &args_capacity, (char*)output);

    string_array_append(&args, &args_count, &args_capacity, NULL);
    run(args_count - 1, args);

    free(args);
}

// TODO this is nearly identical to assemble_file(), they will also take the
// same debug and optimize options, we should merge them
static void codegen_file(const char* input, const char* output) {
    char** args = 0;
    size_t args_count = 0;
    size_t args_capacity = 0;

    string_array_append(&args, &args_count, &args_capacity, tool_cg);

    // TODO debug info, optimize

    string_array_append(&args, &args_count, &args_capacity, (char*)input);
    string_array_append(&args, &args_count, &args_capacity, "-o");
    string_array_append(&args, &args_count, &args_capacity, (char*)output);

    string_array_append(&args, &args_count, &args_capacity, NULL);
    run(args_count - 1, args);

    free(args);
}

static void assemble_file(const char* input, const char* output) {
    char** args = 0;
    size_t args_count = 0;
    size_t args_capacity = 0;

    string_array_append(&args, &args_count, &args_capacity, tool_as);

    /*TODO
    if (debug_info) {
        string_array_append(&args, &args_count, &args_capacity, "-g");
    }
    if (optimize) {
        string_array_append(&args, &args_count, &args_capacity, "-O");
    }
    */

    string_array_append(&args, &args_count, &args_capacity, (char*)input);
    string_array_append(&args, &args_count, &args_capacity, "-o");
    string_array_append(&args, &args_count, &args_capacity, (char*)output);

    string_array_append(&args, &args_count, &args_capacity, NULL);
    run(args_count - 1, args);

    free(args);
}

static void translate_file(const char* input) {

    // figure out what stage based on the file extension
    // TODO we should support -xc or -xassembler later
    int type = file_type(input);

    // preprocess
    if (type == TYPE_C) {
        if (mode == MODE_PREPROCESS) {
            preprocess_file(input, output_filename);
            return;
        }
        if (mode != MODE_PREPROCESS) {
            char* output = make_temp_filename(".i");
            preprocess_file(input, output);
            input = output;
        }
    }

    // compile
    if (type >= TYPE_I) {
        if (mode == MODE_COMPILE) {
            compile_file(input, output_filename);
            return;
        }
        if (mode != MODE_COMPILE) {
            char* extension = ".oir";
            if (tool_cg == NULL) {
                // If we don't have a code generator, the compiler is assumed
                // to output assembly directly.
                extension = ".os";
            }
            char* output = make_temp_filename(extension);
            compile_file(input, output);
            input = output;
        }
    }

    // codegen
    if ((type >= TYPE_OIR) & (tool_cg != NULL)) {
        if (mode == MODE_CODEGEN) {
            codegen_file(input, output_filename);
            return;
        }
        if (mode != MODE_CODEGEN) {
            char* output = make_temp_filename(".os");
            codegen_file(input, output);
            input = output;
        }
    }

    // assemble
    if (type >= TYPE_OS) {
        if (mode == MODE_ASSEMBLE) {
            assemble_file(input, output_filename);
            return;
        }
        if (mode != MODE_ASSEMBLE) {
            char* output = make_temp_filename(".oo");
            assemble_file(input, output);
            input = output;
        }
    }

    // link mode. add file to linker inputs
    string_array_append(&objects, &objects_count, &objects_capacity, (char*)input);
}

static void translate_files(void) {
    size_t i = 0;
    while (i < inputs_count) {
        translate_file(*(inputs + i));
        i = (i + 1);
    }
}

static void do_link(void) {
    if (mode != MODE_LINK) {
        return;
    }

    char** args = 0;
    size_t args_count = 0;
    size_t args_capacity = 0;

    string_array_append(&args, &args_count, &args_capacity, tool_ld);

    if (wrap_header != NULL) {
        string_array_append(&args, &args_count, &args_capacity, "-wrap-header");
        string_array_append(&args, &args_count, &args_capacity, wrap_header);
    }

    if (debug_info) {
        string_array_append(&args, &args_count, &args_capacity, "-g");
    }
    if (optimize) {
        string_array_append(&args, &args_count, &args_capacity, "-O");
    }
    if (!nostdlib) {
        string_array_append(&args, &args_count, &args_capacity, libc_archive);
    }

    // input files
    size_t i = 0;
    while (i < objects_count) {
        string_array_append(&args, &args_count, &args_capacity, *(objects + i));
        i = (i + 1);
    }

    // output at the end, to match our other invocations
    string_array_append(&args, &args_count, &args_capacity, "-o");
    string_array_append(&args, &args_count, &args_capacity, (char*)output_filename);

    // null-terminator
    string_array_append(&args, &args_count, &args_capacity, NULL);

    run(args_count - 1, args);

    free(args);
}

int main(int argc, char** argv) {

    // Collect and verify command-line options
    set_default_options(*argv);
    parse_options(argv + 1);
    check_options();

    // Each file is preprocessed, compiled and assembled first (depending on mode.)
    translate_files();

    // Then all files are linked together (if in linking mode.)
    do_link();

    // Clean up
    delete_temp_files();
    free_options();
}
