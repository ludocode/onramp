; The MIT License (MIT)
;
; Copyright (c) 2024 Fraser Heavy Software
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.



; This is the implementation of types and typedefs for the first stage Onramp
; compiler.
;
; Types are described in the README but here's a quick summary. A type is one
; byte and only the low 7 bits are used so types can be described in assembly
; with a single mix-type argument. The bits are:
;
; - bits 0-4: The pointer indirection count, 0 for non-pointers.
; - bits 5-6: The basic type. 1 is void, 2 is char and 3 is int.
; - bit 7: The lvalue flag.
;
; Typedefs are stored in an open hashtable with linear probing. The hashtable
; has `type_buckets` buckets. Each bucket contains the name of the typedef and
; the base type to which it is defined; these are spread across the arrays
; `type_names` and `type_types`.



; ==========================================================
; char** type_names;
; ==========================================================
; A hashtable of typedef names.
;
; The hashtable uses open addressing with linear probing. It has `type_buckets`
; buckets.
;
; Empty buckets contain null pointers.
; ==========================================================

@type_names
    0



; ==========================================================
; char* type_types;
; ==========================================================
; A hashtable of typedef types.
;
; These entries correspond to the names in type_names.
; ==========================================================

@type_types
    0



; ==========================================================
; int type_count;
; ==========================================================
; The total number of types.
;
; This is limited to half of `type_buckets` to avoid having too many
; collisions.
; ==========================================================

@type_count
    0



; ==========================================================
; const int type_buckets;
; ==========================================================
; The size of the type hashtables.
;
; This must be a power of two.
; ==========================================================

@type_buckets
    128



; ==========================================================
; void type_init(void);
; ==========================================================

=type_init
    enter

    ; allocate type_names (zeroed)
    imw r0 ^type_buckets
    ldw r0 r0 rpp
    mov r1 4
    call ^calloc
    jz r0 &type_init_oom
    imw r1 ^type_names
    stw r0 rpp r1

    ; allocate type_types
    imw r0 ^type_buckets
    ldw r0 r0 rpp
    call ^malloc
    jz r0 &type_init_oom
    imw r1 ^type_types
    stw r0 rpp r1

    ; add `void`
    imw r0 ^str_void
    add r0 r0 rpp
    call ^strdup
    jz r0 &type_init_oom
    mov r1 r0
    mov r0 0x10   ; 0x10 == void
    call ^type_add

    ; add `char`
    imw r0 ^str_char
    add r0 r0 rpp
    call ^strdup
    jz r0 &type_init_oom
    mov r1 r0
    mov r0 0x20   ; 0x20 == char
    call ^type_add

    ; add `int`
    imw r0 ^str_int
    add r0 r0 rpp
    call ^strdup
    jz r0 &type_init_oom
    mov r1 r0
    mov r0 0x30   ; 0x30 == int
    call ^type_add

    leave
    ret

:type_init_oom
    imw r0 ^error_out_of_memory
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; void type_destroy(void);
; ==========================================================

=type_destroy
    enter

    ; TODO loop type_names freeing each name

    ; free type_names
    imw r0 ^type_names
    ldw r0 r0 rpp
    call ^free

    ; free type_types
    imw r0 ^type_types
    ldw r0 r0 rpp
    call ^free

    leave
    ret



; TODO deduplicate the lookup code in the below functions



; ==========================================================
; void type_add(char type, char* name);
; ==========================================================
; Adds the given name as a typedef for the given type.
;
; This takes ownership of the given name string. It will be freed on exit.
;
; vars:
; - type: rfp-4
; - name: rfp-8
; - mask: r2, rfp-12
; - index: r3, rfp-16
; ==========================================================

=type_add
    enter

    ; store our args
    push r0
    push r1

    ; TODO might be good to do a safety check here that the type's lvalue flag
    ; is not set

    ; check that the table isn't full (the limit is half the size)
    imw r9 ^type_buckets
    ldw r9 r9 rpp
    shru r9 r9 1    ; divide by two
    imw r1 ^type_count
    ldw r1 r1 rpp
    cmpu r0 r1 r9
    cmpu r0 r0 -1
    jz r0 &type_add_count_ok

    ; the table is half full. fatal error
    imw r0 ^error_too_many_typedefs
    add r0 r0 rpp
    call ^fatal
:type_add_count_ok

    ; calculate and store a table mask
    imw r2 ^type_buckets
    ldw r2 r2 rpp
    dec r2
    push r2

    ; calculate the hash and store the starting index
    ldw r0 rfp -8
    call ^fnv1a_cstr
    ldw r2 rfp -12
    and r3 r0 r2
    push r3

:type_add_loop

    ; get the name at this position
    imw r9 ^type_names
    ldw r9 r9 rpp
    ldw r3 rfp -16
    shl r3 r3 2  ; index in bytes
    ldw r8 r9 r3

    ; if it's null, we've found our insertion slot
    jz r8 &type_add_slot

    ; compare it to the name given, make sure it doesn't match
    ldw r0 rfp -8
    mov r1 r8
    call ^strcmp
    jz r0 &type_add_match

    ; increment the index, wrapping around
    ldw r3 rfp -16
    inc r3
    ldw r2 rfp -12
    and r3 r3 r2
    stw r3 rfp -16

    ; keep searching for a free slot
    jmp &type_add_loop

:type_add_slot

    ; store the name
    imw r9 ^type_names
    ldw r9 r9 rpp
    ldw r3 rfp -16
    shl r3 r3 2  ; index in bytes
    ldw r1 rfp -8
    stw r1 r9 r3

    ; store the type
    imw r9 ^type_types
    ldw r9 r9 rpp
    ldw r3 rfp -16
    ldb r0 rfp -4
    stb r0 r9 r3

    ; increment the count
    imw r1 ^type_count
    ldw r0 r1 rpp
    inc r0
    stw r0 r1 rpp

    leave
    ret

:type_add_match

    ; this type name is already defined. see if the type is the same
    imw r9 ^type_types
    ldw r9 r9 rpp
    ldw r3 rfp -16
    ldb r8 r9 r3
    ldb r1 rfp -4
    cmpu r0 r8 r1
    jnz r0 &type_add_duplicate

    ; the type is the same. duplicate typedefs are not an error.
    leave
    ret

:type_add_duplicate
    ; type name redefined as a different type. fatal error.
    imw r0 ^error_type_redefined
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; bool type_find(char* name, char* out_type);
; ==========================================================
; Finds the type with the given name.
;
; vars:
; - out_type: rfp-4
; - name: rfp-8
; - mask: rfp-12
; - index: rfp-16
; ==========================================================

=type_find
    enter

    ; store our args
    push r1
    push r0

    ; calculate and store a table mask
    imw r2 ^type_buckets
    ldw r2 r2 rpp
    dec r2
    push r2

    ; calculate the hash and store the starting index
    ldw r0 rfp -8
    call ^fnv1a_cstr
    ldw r2 rfp -12
    and r3 r0 r2
    push r3

:type_find_loop

    ; get the name at this position
    imw r9 ^type_names
    ldw r9 r9 rpp
    ldw r3 rfp -16
    shl r3 r3 2
    ldw r8 r9 r3

    ; if it's null, the type is not found
    jz r8 &type_find_false

    ; see if it matches
    ldw r0 rfp -8
    mov r1 r8
    call ^strcmp
    jz r0 &type_find_true

    ; increment the index, wrapping around
    ldw r3 rfp -16
    inc r3
    ldw r2 rfp -12
    and r3 r3 r2
    stw r3 rfp -16

    ; keep searching for the type name
    jmp &type_find_loop

:type_find_true

    ; output the type
    imw r9 ^type_types
    ldw r9 r9 rpp
    ldw r3 rfp -16  ; get the index in the hashtable
    ldb r0 r9 r3    ; get the type from the hashtable
    ldw r1 rfp -4   ; get the out_type pointer
    stb r0 r1 0     ; store the type in out_type

    ; return true
    mov r0 1
    leave
    ret

:type_find_false
    ; type not found. return false.
    zero r0
    leave
    ret



; ==========================================================
; char type_promote(char type);
; ==========================================================
; Promotes the given type, which must be an r-value (i.e. the result of an
; expression.)
;
; TODO does this make sense? If it's an r-value should we dereference it?
; ==========================================================

=type_promote

    ; If it's char, promote to int. That's our only promotion rule.
    ; TODO this should be a compile function, not a type function, we need to
    ; emit sxb on a type promotion
    sub r1 r0 0x20
    jz r1 &type_promote_int
    ret
:type_promote_int
    mov r0 0x30
    ret



; ==========================================================
; int type_size(char type);
; ==========================================================
; Returns the size in bytes of the given type (as in sizeof().)
; ==========================================================

=type_size

    ; we shouldn't be calling sizeof() on l-values or void
    and r1 r0 0x40   ; l-value flag
    jnz r1 &type_size_invalid
    sub r1 r0 0x10  ; void
    jz r1 &type_size_void

    ; `char` has size 1
    sub r1 r0 0x20   ; char
    jz r1 &type_size_1

    ; every other type has size 4.
    mov r0 4
    ret

:type_size_1
    mov r0 1
    ret

:type_size_invalid
    ; fatal, internal error
    imw r0 ^error_internal
    add r0 r0 rpp
    call ^fatal

:type_size_void
    ; fatal, sizeof(void) not allowed
    imw r0 ^error_sizeof_void
    add r0 r0 rpp
    call ^fatal



