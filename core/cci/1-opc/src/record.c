#include "record.h"

/*
 * record_t looks like this:
 *
 * typedef struct {
 *     char* name;
 *     field_t* fields;
 * } record_t;
 *
 * TODO we need an additional field to say whether it's a struct or union (or
 * do we? would that be something for the type table instead?)
 */

record_t* record_new(const char* name) {
    record_t* record = malloc(sizeof(void*) * 2);
    if (!record) {
        fatal("Out of memory.");
    }

    *record = strdup(name);
    if (!*record) {
        fatal("Out of memory.");
    }

    *(record + 1) = NULL; // fields
    return record;
}

static void record_destroy_fields(record_t* record) {
    field_t* field = record_fields(record);
    while (field) {
        field_t* next = field_next(field);
        field_delete(field);
        field = next;
    }
}

void record_delete(record_t* record) {
    record_destroy_fields(record);
    free(*record); // name
    free(record);
}

const char* record_name(record_t* record) {
    return *record;
}

field_t* record_fields(record_t* record) {
    return *(record + 1);
}

void record_set_fields(record_t* record, field_t* fields) {
    record_destroy_fields(record);
    *(record + 1) = fields;
}

size_t record_size(record_t* record) {
    size_t size = 4;
    field_t* field = record_fields(record);
    while (field) {
        size_t end = field_end(field);
        if (end > size) {
            size = end;
        }
        field = field_next(field);
    }
    field = ((field + 3) & (~3)); // round up to multiple of word size (4)
    return field;
}
