#ifndef BASE_H_INCLUDED
#define BASE_H_INCLUDED

/**
 * A qualified base type.
 */
typedef int base_t;

#define BASE_RECORD 0x01
#define BASE_VOID 0x02
// `char` and `signed char` are supposed to be distinct types in C but we don't
// bother with this in opC.
#define BASE_UNSIGNED_CHAR 0x03
#define BASE_UNSIGNED_SHORT 0x04
#define BASE_UNSIGNED_INT 0x05
#define BASE_UNSIGNED_LONG 0x06
#define BASE_UNSIGNED_LONG_LONG 0x07
#define BASE_SIGNED_CHAR 0x13
#define BASE_SIGNED_SHORT 0x14
#define BASE_SIGNED_INT 0x15
#define BASE_SIGNED_LONG 0x16
#define BASE_SIGNED_LONG_LONG 0x17

const char* base_to_string(base_t base);

size_t base_size(base_t base);

bool base_is_signed(base_t base);

#endif
