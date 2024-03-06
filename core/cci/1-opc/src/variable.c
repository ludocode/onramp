#include "variable.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "common.h"

static char** variable_names;
static type_t** variable_types;
static int* variable_offsets;

int variable_count;
int variable_global_count;

#define VARIABLE_MAX 128

void variable_init(void) {
    variable_names = malloc(VARIABLE_MAX * sizeof(char*));
    variable_types = malloc(VARIABLE_MAX * sizeof(type_t*));
    variable_offsets = malloc(VARIABLE_MAX * sizeof(int));
}

void variable_destroy(void) {
    variable_pop(0);
    free(variable_names);
    free(variable_offsets);
    free(variable_types);
}

void variable_add(char* name, type_t* type, bool global) {
    //printf("%s\n",name);
    if (variable_count == VARIABLE_MAX) {
        fatal("Too many variables.");
    }
    variable_names[variable_count] = name;
    variable_types[variable_count] = type;

    if (global) {
        variable_offsets[variable_count] = 0;
        ++variable_global_count;
    } else {
        variable_offsets[variable_count] =
            -((variable_count - variable_global_count) + 1) * 4;
    }

    //printf("    adding variable %s with offset %i\n", name, variable_offsets[variable_count]);
    ++variable_count;
}

void variable_pop(int previous_variable_count) {
    //printf("popping variables to %i\n", variable_count);
    for (int i = previous_variable_count; i < variable_count; ++i) {
        //printf("    popping variable %s\n", variable_names[i]);
        free(variable_names[i]);
        type_delete(variable_types[i]);
    }
    variable_count = previous_variable_count;
}

bool variable_find(const char* name, const type_t** type, int* offset) {

    // We search backwards in order to handle shadowing properly.
    int i = variable_count;
    while (i > 0) {
        i = (i - 1);
        if (0 != strcmp(variable_names[i], name)) {
            continue;
        }

        // Found.
        *type = variable_types[i];
        *offset = variable_offsets[i];
        return true;
    }

    // Not found.
    return false;
}

int variable_local_size(void) {
    //printf("%i %i\n", variable_count, variable_global_count);
    return (variable_count - variable_global_count) * 4;
}

void dump_variables(void) {
    printf("%i variables\n", variable_count);
    for (int i = 0; i < variable_count; ++i) {
        fputs("  ", stdout);
        fputs(variable_names[i], stdout);
        fputs("  ", stdout);
        printf("%i", variable_offsets[i]);
        putchar('\n');
    }
}
