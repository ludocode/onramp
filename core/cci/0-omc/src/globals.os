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



; This is the implementation of global variables and functions for the first
; stage Onramp compiler.
;
; Globals are stored in an open hashtable with linear probing. The hashtable
; has `globals_buckets` buckets. Each bucket contains either a variable or a
; function. The contents of each bucket is spread across several arrays for
; simplicity of implementation:
;
; - `globals_names` contains the names of functions and variables.
; - `globals_types` contains the types of variables or the return types of functions.
; - `globals_argcounts` contains the number of arguments of functions, or -1 for variables.
; - `globals_argtypes` contains an allocated array containing the type of each argument (for functions only.)



; ==========================================================
; static char** globals_names;
; ==========================================================
; A hashtable of globals variable and function names.
;
; The hashtable uses open addressing with linear probing.
;
; Empty buckets contain null pointers. Non-empty buckets contain allocated strings.
; ==========================================================

@globals_names
    0



; ==========================================================
; static char* globals_types;
; ==========================================================
; A hashtable of the types of globals.
;
; For functions, this is the return type.
; ==========================================================

@globals_types
    0



; ==========================================================
; static int* globals_argcounts;
; ==========================================================
; A hashtable of the number of arguments for each global function.
;
; If the global is a variable, its value here is -1.
; ==========================================================

@globals_argcounts
    0



; ==========================================================
; static char** globals_argtypes;
; ==========================================================
; A hashtable of the argument types of functions.
;
; These entries correspond to the names in globals_names.
; ==========================================================

@globals_argtypes
    0



; ==========================================================
; static int globals_count;
; ==========================================================
; The total number of globals.
;
; This is limited to half of `globals_buckets` to avoid having too many
; collisions.
; ==========================================================

@globals_count
    0



; ==========================================================
; static const int globals_buckets;
; ==========================================================
; The size of the globals hashtables.
;
; This must be a power of two.
; ==========================================================

@globals_buckets
    512



; ==========================================================
; void globals_init(void);
; ==========================================================

=globals_init

    ; allocate globals_names (zeroed)
    imw r0 ^globals_buckets
    ldw r0 r0 rpp
    mov r1 4
    call ^calloc
    jz r0 &globals_init_oom
    imw r1 ^globals_names
    stw r0 rpp r1

    ; allocate globals_types
    imw r0 ^globals_buckets
    ldw r0 r0 rpp
    call ^malloc
    jz r0 &globals_init_oom
    imw r1 ^globals_types
    stw r0 rpp r1

    ; allocate globals_argcounts
    imw r0 ^globals_buckets
    ldw r0 r0 rpp
    shl r0 r0 2
    call ^malloc
    jz r0 &globals_init_oom
    imw r1 ^globals_argcounts
    stw r0 rpp r1

    ; allocate globals_argtypes
    imw r0 ^globals_buckets
    ldw r0 r0 rpp
    shl r0 r0 2
    call ^malloc
    jz r0 &globals_init_oom
    imw r1 ^globals_argtypes
    stw r0 rpp r1

    ret

:globals_init_oom
    imw r0 ^error_out_of_memory
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; void globals_destroy(void);
; ==========================================================

=globals_destroy
    ; TODO. we want to free everything to check for memory leaks. right now
    ; free doesn't do anything in the first stage libc anyway so we don't
    ; bother.
    ret



; ==========================================================
; static int globals_bucket(char* name);
; ==========================================================
; Finds the index of the bucket containing the global with the given name or
; the empty bucket where it should be inserted.
;
; vars:
; - name: rfp-4
; - mask: rfp-8
; - index: rfp-12
; ==========================================================

@globals_bucket
    enter
    sub rsp rsp 12
    stw r0 rfp -4

    ; calculate and store a table mask
    imw r2 ^globals_buckets
    ldw r2 r2 rpp
    dec r2
    stw r2 rfp -8

    ; calculate the hash and store the starting index
    ldw r0 rfp -4
    call ^fnv1a_cstr
    ldw r2 rfp -8
    and r3 r0 r2
    stw r3 rfp -12

:globals_bucket_loop

    ; get the name at this position
    imw r9 ^globals_names
    ldw r9 r9 rpp
    ldw r3 rfp -12
    shl r3 r3 2
    ldw r1 r9 r3

    ; if it's null, we're done
    jz r1 &globals_bucket_done

    ; if it matches, we're also done
    ldw r0 rfp -4
    call ^strcmp
    jz r0 &globals_bucket_done

    ; increment the index, wrapping around
    ldw r3 rfp -12
    inc r3
    ldw r2 rfp -8
    and r3 r3 r2
    stw r3 rfp -12

    ; keep searching
    jmp &globals_bucket_loop

:globals_bucket_done

    ; return the index
    ldw r0 rfp -12
    leave
    ret



; ==========================================================
; static int globals_add(char type, char* name, int argcount);
; ==========================================================
; Adds a new global with the given name and type, checking for duplicates and
; returning its index.
;
; vars:
; - type: rfp-4
; - name: rfp-8
; - argcount: rfp-12
; - byte_offset: r0
; - word_offset: r1
; ==========================================================

@globals_add
    enter

    ; store args
    push r0
    push r1
    push r2

    ; find the bucket offsets for this global
    mov r0 r1
    call ^globals_bucket
    shl r1 r0 2

    ; check if the name already exists
    imw r9 ^globals_names
    ldw r9 r9 rpp
    ldw r2 r9 r1
    jnz r2 &globals_add_duplicate

    ; store the name
    ldw r2 rfp -8
    stw r2 r9 r1

    ; store the type
    imw r9 ^globals_types
    ldw r9 r9 rpp
    ldw r2 rfp -4
    stb r2 r9 r0

    ; store the argcount
    imw r9 ^globals_argcounts
    ldw r9 r9 rpp
    ldw r2 rfp -12
    stw r2 r9 r1

    ; done
    leave
    ret

:globals_add_duplicate
    ; TODO we are only supposed to forbid duplicate definitions, not duplicate
    ; declarations. for now we ignore duplicates and let the linker catch the
    ; error instead.
    leave
    ret
    ;imw r0 ^error_duplicate_global
    ;add r0 r0 rpp
    ;call ^fatal



; ==========================================================
; void globals_add_variable(char type, char* name);
; ==========================================================
; Declares a new global variable of the given name and type.
; ==========================================================

=globals_add_variable
    mov r2 -1
    jmp ^globals_add



; ==========================================================
; void globals_add_function(char return_type, char* name,
;                           int arg_count, char* arg_types);
; ==========================================================
; Declares a new function with the given properties.
; ==========================================================

=globals_add_function
    ; TODO arg_types is currently not sent and ignored.
    jmp ^globals_add



; ==========================================================
; char globals_find_function(char* name, int* out_arg_count);
; ==========================================================
; Finds a function with the given name, returning its return type and argument
; count.
;
; Returns 0 if no such function exists.
; ==========================================================

=globals_find_function

    ; find the bucket that would contain it
    push r1
    call ^globals_bucket
    shl r2 r0 2
    pop r1

    ; if the string is null, the bucket is empty; not found
    imw r9 ^globals_names
    ldw r9 r9 rpp
    ldw r9 r9 r2
    jnz r9 &globals_find_function_not_empty
    zero r0
    ret
:globals_find_function_not_empty

    ; get the arg_count
    imw r9 ^globals_argcounts
    ldw r9 r9 rpp
    ldw r9 r9 r2

    ; if it's -1, it's a variable; not found
    sub r8 r9 -1
    jnz r8 &globals_find_function_found
    zero r0
    ret
:globals_find_function_found

    ; found! return the arg_count and type
    stw r9 r1 0
    imw r9 ^globals_types
    ldw r9 r9 rpp
    ldb r0 r9 r0
    ret



; ==========================================================
; char globals_find_variable(char* name);
; ==========================================================
; Finds a global variable with the given name, returning its type.
;
; Returns 0 if no such variable exists. (Note that functions are not variables
; in omC.)
; ==========================================================

=globals_find_variable

    ; find the bucket that would contain it
    call ^globals_bucket
    shl r1 r0 2

    ; if the string is null, the bucket is empty; not found
    imw r9 ^globals_names
    ldw r9 r9 rpp
    ldw r2 r9 r1
    jnz r2 &globals_find_variable_not_empty
    zero r0
    ret
:globals_find_variable_not_empty

    ; if argcounts isn't -1, it's a function; not found
    imw r9 ^globals_argcounts
    ldw r9 r9 rpp
    ldw r2 r9 r1
    sub r2 r2 -1
    jz r2 &globals_find_variable_found
    zero r0
    ret
:globals_find_variable_found

    ; found! return the type
    imw r9 ^globals_types
    ldw r9 r9 rpp
    ldb r0 r9 r0
    ret
