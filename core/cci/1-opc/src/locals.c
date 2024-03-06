#include "locals.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "common.h"

static char** locals_names;
static type_t** locals_types;
static int* locals_offsets;

int locals_count;
int locals_global_count;

#define LOCALS_MAX 128

void locals_init(void) {
    locals_names = malloc(LOCALS_MAX * sizeof(char*));
    locals_types = malloc(LOCALS_MAX * sizeof(type_t*));
    locals_offsets = malloc(LOCALS_MAX * sizeof(int));
}

void locals_destroy(void) {
    locals_pop(0);
    free(locals_names);
    free(locals_offsets);
    free(locals_types);
}

void locals_add(char* name, type_t* type) {
    //printf("%s\n",name);
    if (locals_count == LOCALS_MAX) {
        fatal("Too many local variables.");
    }
    *(locals_names + locals_count) = name;
    *(locals_types + locals_count) = type;

    *(locals_offsets + locals_count) = (-(locals_count + 1) * 4);

    //printf("    adding variable %s with offset %i\n", name, locals_offsets[locals_count]);
    locals_count = (locals_count + 1);
}

void locals_pop(int previous_locals_count) {
    //printf("popping variables to %i\n", locals_count);
    int i = previous_locals_count;
    while (i < locals_count) {
        //printf("    popping variable %s\n", locals_names[i]);
        free(*(locals_names + i));
        type_delete(*(locals_types + i));
        i = (i + 1);
    }
    locals_count = previous_locals_count;
}

bool locals_find(const char* name, const type_t** type, int* offset) {

    // We search backwards in order to handle shadowing properly.
    int i = locals_count;
    while (i > 0) {
        i = (i - 1);
        if (0 != strcmp(*(locals_names + i), name)) {
            continue;
        }

        // Found.
        *type = *(locals_types + i);
        *offset = *(locals_offsets + i);
        return true;
    }

    // Not found.
    return false;
}

int locals_frame_size(void) {
    //printf("%i %i\n", locals_count, locals_global_count);
    return locals_count * 4;
}

void dump_variables(void) {
    /*
    printf("%i variables\n", locals_count);
    for (int i = 0; i < locals_count; ++i) {
        fputs("  ", stdout);
        fputs(locals_names[i], stdout);
        fputs("  ", stdout);
        printf("%i", locals_offsets[i]);
        putchar('\n');
    }
    */
}
