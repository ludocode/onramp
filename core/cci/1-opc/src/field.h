#ifndef FIELD_H_INCLUDED
#define FIELD_H_INCLUDED

/**
 * A field is one variable in a struct.
 *
 * Fields are stored in an intrusive singly-linked list and are owned by their
 * containing record.
 *
 * Fields are immutable.
 */
typedef void* field_t;

/**
 * Creates a new field.
 *
 * The field takes ownership of the given type.
 */
field_t* field_new(const char* name, type_t* type, size_t offset, field_t* next);

void field_delete(field_t* field);

const char* field_name(field_t* field);

type_t* field_type(field_t* field);

field_t* field_next(field_t* field);

/**
 * Returns the size (as in sizeof) of this field.
 */
size_t field_size(field_t* field);

/**
 * Returns the offset of this field from the start of its containing record.
 */
size_t field_offset(field_t* field);

/**
 * Returns the end offset of this field, i.e. its offset plus its size.
 */
size_t field_end(field_t* field);

#endif
