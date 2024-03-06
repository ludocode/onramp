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
    if (locals_count == LOCALS_MAX) {
        fatal("Too many local variables.");
    }
    *(locals_names + locals_count) = name;
    *(locals_types + locals_count) = type;

    // We don't pack vars on the stack; we just use at least a full word for
    // each.
    int size = type_size(type);
    size = ((size + 3) & ~3);

    int offset = 0;
    if (locals_count > 0) {
        offset = *(locals_offsets + (locals_count - 1));
    }
    offset = (offset - size);
    *(locals_offsets + locals_count) = offset;

    locals_count = (locals_count + 1);
}

void locals_pop(int previous_locals_count) {
    int i = previous_locals_count;
    while (i < locals_count) {
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
    if (locals_count == 0) {
        return 0;
    }

    // The frame size is the offset of the last local rounded down to a
    // multiple of the word size. (We don't bother rounding now because the
    // size of all locals is already rounded up to the word size.)
    return -(*(locals_offsets + (locals_count - 1)) /*& ~3*/);
}

void dump_variables(void) {
    putd(locals_count);
    puts(" variables");
    int i = 0;
    while (i < locals_count) {
        fputs("  ", stdout);
        fputs(*(locals_names + i), stdout);
        fputs("  ", stdout);
        putd(*(locals_offsets + i));
        putchar('\n');
        i = (i + 1);
    }
}
