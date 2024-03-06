#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compile.h"
#include "emit.h"
#include "lexer.h"
#include "parse.h"
#include "type.h"
#include "variable.h"
#include "global.h"

static void usage(const char* name) {
    fputs("\nUsage: ", stderr);
    fputs(name, stderr);
    fputs(" <input_file> -o <output_file>\n", stderr);
    _Exit(1);
}

int main(int argc, const char** argv) {
    #ifndef __onramp__
    setvbuf(stdout, NULL, _IONBF, 0); // TODO debugging
    #endif

    // parse command-line options
    const char* input_filename = NULL;
    const char* output_filename = NULL;
    int i = 1;
    while (i < argc) {

        // output
        if (0 == strcmp("-o", *(argv + i))) {
            if (output_filename != NULL) {
                puts("ERROR: -o cannot be specified twice.");
                usage(*argv);
            }
            i = (i + 1);
            if (i == argc) {
                puts("ERROR: -o must be followed by a filename.");
                usage(*argv);
            }
            output_filename = *(argv + i);
            i = (i + 1);
            continue;
        }

        // unrecognized option
        if (**(argv + i) == '-') {
            fputs("ERROR: Unsupported option: ", stderr);
            fputs(*(argv + i), stderr);
            usage(*argv);
        }

        // input
        if (input_filename != NULL) {
            puts("ERROR: only one input filename can be provided.");
            usage(*argv);
        }
        input_filename = *(argv + i);
        i = (i + 1);
    }

    if (input_filename == NULL) {
        fputs("ERROR: Input filename not specified.", stderr);
        usage(*argv);
    }
    if (output_filename == NULL) {
        fputs("ERROR: Output filename not specified.", stderr);
        usage(*argv);
    }

    globals_init();
    emit_init(output_filename);
    lexer_init(input_filename);
    typedef_init();
    variable_init();
    compile_init();
    parse_init();

    parse();

    parse_destroy();
    compile_destroy();
    variable_destroy();
    typedef_destroy();
    lexer_destroy();
    emit_destroy();
    globals_destroy();
    return EXIT_SUCCESS;
}
