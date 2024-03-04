#ifndef RECORD_H_INCLUDED
#define RECORD_H_INCLUDED

// A record is a struct or union in C. it needs to be a container type that has
// a name and list of fields.
//
// record just has name and list of fields.
//
// we need to be able to create a record_t that is initially empty on a
// forward-declaration of a struct/union, that way stuff can start referring to
// it. we then add the fields later. (empty struct isn't allowed in C so we can
// just check if fields is null to see whether the struct has been defined.)
//
// record maybe should also be reference counted? it goes in a scope, does
// anything that refers to a struct escape that scope or can the scope just own
// it and delete it when done? would be sooo much better if we didn't have to
// refcount it. (but if not refcounted, if struct is anonymous, who owns it?
// should scope own it anyway, just keep it separate from the hashtable? or do
// we just not support anonymous struct/union?)
//
// fields can simply be a singly linked list. each field needs:
// - type
// - name
// - offset
// - next field pointer
//
// only difference between struct and union is:
// - in a union all fields have offset 0
// - the prefix to name them is different (`struct` vs. `union` keywords.)


/**
 * A record is a struct or union type in C.
 *
 * A record is created when a struct or union is first declared. Its fields are
 * initially empty. When the struct or union is later defined, the fields are
 * parsed and assigned to the record.
 *
 * Records are owned by the scope in which they were declared. They are
 * destroyed when the scope is destroyed (which also destroys anything that
 * could reference them, preventing any dangling references.)
 */
typedef void* record_t;

record_t* record_new(const char* name);

void record_delete(record_t* record);

const char* record_name(record_t* record);

/**
 * Returns the fields of this record.
 *
 * If the record has not been defined yet (i.e. it has only been
 * forward-declared), this returns NULL.
 */
field_t* record_fields(record_t* record);

/**
 * Sets the fields of the given record.
 *
 * The record takes ownership and frees the fields when done.
 */
void record_set_fields(record_t* record, field_t* fields);

/**
 * Returns the size (as in sizeof) of this record.
 */
size_t record_size(record_t* record);




#endif
