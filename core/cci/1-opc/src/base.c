#include "base.h"

const char* base_to_string(base_t base) {
    if (base == BASE_RECORD) {return "record";}
    if (base == BASE_VOID) {return "void";}
    if (base == BASE_UNSIGNED_CHAR) {return "unsigned char";}
    if (base == BASE_UNSIGNED_SHORT) {return "unsigned short";}
    if (base == BASE_UNSIGNED_INT) {return "unsigned int";}
    if (base == BASE_UNSIGNED_LONG) {return "unsigned long";}
    if (base == BASE_UNSIGNED_LONG_LONG) {return "unsigned long long";}
    if (base == BASE_SIGNED_CHAR) {return "signed char";}
    if (base == BASE_SIGNED_SHORT) {return "signed short";}
    if (base == BASE_SIGNED_INT) {return "signed int";}
    if (base == BASE_SIGNED_LONG) {return "signed long";}
    if (base == BASE_SIGNED_LONG_LONG) {return "signed long long";}
    fatal("Invalid base type");
}

size_t base_size(base_t base) {
    if (base == BASE_VOID) {
        // For some reason sizeof(void) is 1.
        return 1;
    }
    if (base == BASE_UNSIGNED_CHAR) {return 1;}
    if (base == BASE_UNSIGNED_SHORT) {return 2;}
    if (base == BASE_UNSIGNED_INT) {return 4;}
    if (base == BASE_UNSIGNED_LONG) {return 4;}
    if (base == BASE_UNSIGNED_LONG_LONG) {return 8;}
    if (base == BASE_SIGNED_CHAR) {return 1;}
    if (base == BASE_SIGNED_SHORT) {return 2;}
    if (base == BASE_SIGNED_INT) {return 4;}
    if (base == BASE_SIGNED_LONG) {return 4;}
    if (base == BASE_SIGNED_LONG_LONG) {return 8;}

    if (base == BASE_RECORD) {
        // it's a record. should be calling record_size(), not base_size()
        fatal("Internal error: cannot base_size(BASE_RECORD)");
    }

    fatal("Internal error: invalid base type");
}

bool base_is_signed(base_t base) {
    if (base == BASE_SIGNED_CHAR) {return true;}
    if (base == BASE_SIGNED_SHORT) {return true;}
    if (base == BASE_SIGNED_INT) {return true;}
    if (base == BASE_SIGNED_LONG) {return true;}
    if (base == BASE_SIGNED_LONG_LONG) {return true;}
    return false;
}
