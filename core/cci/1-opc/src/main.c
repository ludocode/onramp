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

static void usage(const char* name) {
    printf("\nUsage: %s <input_file> -o <output_file>\n", name);
    exit(EXIT_FAILURE);
}

int main(int argc, const char** argv) {
    #ifndef __onramp__
    setvbuf(stdout, NULL, _IONBF, 0); // TODO debugging
    #endif

    // parse command-line options
    const char* input_filename = NULL;
    const char* output_filename = NULL;
    for (int i = 1; i < argc; ++i) {
        // output
        if (0 == strcmp("-o", argv[i])) {
            if (output_filename != NULL) {
                puts("ERROR: -o cannot be specified twice.");
                usage(argv[0]);
            }
            if (++i == argc) {
                puts("ERROR: -o must be followed by a filename.");
                usage(argv[0]);
            }
            output_filename = argv[i];

        // unrecognized argument
        } else if (argv[i][0] == '-') {
            printf("ERROR: Unsupported option: %s", argv[i]);
            usage(argv[0]);

        // input
        } else {
            if (input_filename != NULL) {
                puts("ERROR: only one input filename can be provided.");
                usage(argv[0]);
            }
            input_filename = argv[i];
        }
    }

    if (input_filename == NULL) {
        fputs("ERROR: Input filename not specified.", stderr);
        usage(argv[0]);
    }
    if (output_filename == NULL) {
        fputs("ERROR: Output filename not specified.", stderr);
        usage(argv[0]);
    }

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
    return EXIT_SUCCESS;
}
