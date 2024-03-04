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



; This is the implementation of local variables for the first stage Onramp
; compiler.
;
; Local variables in scope are stored in a stack, in the `locals_names` and
; `locals_types` arrays in the order they are defined in the source. All
; variable lookup is done by linear search in reverse order to handle shadowing
; correctly.
;
; All local variables take up exactly one word of space. Variables of type
; `char` are accessed via load/store byte instructions; all other variables are
; accessed via load/store word. Local variables are stored in the current stack
; frame. A variable's index in these arrays (plus one) is its position in words
; under the frame pointer.
;
; New local variables are defined with `locals_add()`. Local variables are
; destroyed by `locals_pop()` at the end of each block and each function.
;
; Global variables are not stored here; they are stored along with functions in
; globals.os.



; ==========================================================
; char** locals_names;
; ==========================================================
; An array of variable names in scope.
; ==========================================================

@locals_names
    0



; ==========================================================
; char* locals_types;
; ==========================================================
; The types of variables defined in scope. These correspond to the names in
; locals_names.
; ==========================================================

@locals_types
    0



; ==========================================================
; int locals_count;
; ==========================================================
; The total number of local variables currently defined.
; ==========================================================

=locals_count
    0



; ==========================================================
; const int locals_capacity;
; ==========================================================
; The maximum number of variables that can be in scope at any given time.
;
; This is the number of entries in the `locals_names` and `locals_types`
; arrays.
; ==========================================================

@locals_capacity
    64



; ==========================================================
; void locals_init(void);
; ==========================================================

=locals_init
    enter

    ; allocate locals_types
    imw r0 ^locals_capacity
    ldw r0 r0 rpp
    call ^malloc
    jz r0 &locals_init_oom
    imw r1 ^locals_types
    stw r0 rpp r1

    ; allocate locals_names
    imw r0 ^locals_capacity
    ldw r0 r0 rpp
    shl r0 r0 2
    call ^malloc
    jz r0 &locals_init_oom
    imw r1 ^locals_names
    stw r0 rpp r1

    leave
    ret

:locals_init_oom
    imw r0 ^error_out_of_memory
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; void locals_destroy(void);
; ==========================================================

=locals_destroy

    ; free all variable names
    zero r0
    call ^locals_pop

    ; free locals_names
    imw r0 ^locals_names
    ldw r0 r0 rpp
    call ^free

    ; free locals_types
    imw r0 ^locals_types
    ldw r0 r0 rpp
    call ^free

    ret



; ==========================================================
; void locals_pop(int count);
; ==========================================================
; Destroys all variables beyond the given count.
;
; After calling this, `locals_count` will equal the given count.
;
; vars:
; - count: r0, rfp-4
; - index: r1, rfp-8
; ==========================================================

=locals_pop
    enter
    push r0

    ; get the current variable count
    imw r1 ^locals_count
    ldw r1 r1 rpp
    push r1

:locals_pop_loop

    ; if the count matches expected, we're done
    cmpu r0 r0 r1
    je r0 &locals_pop_done

    ; decrement the count
    dec r1
    stw r1 rfp -8

    ; free the associated name
    imw r2 ^locals_names
    ldw r2 r2 rpp
    shl r3 r1 2
    ldw r0 r2 r3
    call ^free

    ; reload our registers
    ldw r0 rfp -4
    ldw r1 rfp -8

    ; loop
    jmp &locals_pop_loop

:locals_pop_done

    imw r1 ^locals_count
    ldw r0 rfp -4
    stw r0 r1 rpp

    leave
    ret



; ==========================================================
; void locals_add(char type, char* name);
; ==========================================================
; Adds a newly defined local variable.
;
; This takes ownership of the given name. The name will be freed when the
; variable goes out of scope.
; ==========================================================

=locals_add
    enter

    ; void variables are not allowed.
    jz r0 &locals_add_void

    ; get the current variable count
    imw r2 ^locals_count
    ldw r3 r2 rpp

    ; store the type
    imw r4 ^locals_types
    ldw r4 r4 rpp
    stb r0 r4 r3

    ; store the name
    imw r4 ^locals_names
    ldw r4 r4 rpp
    shl r9 r3 2
    stw r1 r4 r9

    ; increment the current variable count
    inc r3
    stw r3 r2 rpp

    ;;; return the offset (4 + 4 * index == 4 * count)
    ; DISABLED, not used
    ;shl r0 r3 2
    leave
    ret

:locals_add_void
    imw r0 ^error_void_variable
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; char locals_find(const char* name, int* out_offset);
; ==========================================================
; Finds a local variable with the given name, returning its type and its offset
; within the stack frame.
;
; vars:
; - name: rfp-4
; - out_offset: rfp-8
; - index: rfp-12
; ==========================================================

=locals_find
    enter
    push r0
    push r1
    push 0

    ; TODO we need to search backwards! vars defined in inner scopes should
    ; shadow vars defined in outer scopes.

:locals_find_loop

    ; check if we've run out of locals
    ldw r0 rfp -12
    imw r1 ^locals_count
    ldw r1 rpp r1
    sub r2 r0 r1
    jz r2 &locals_find_not_found

    ; compare the name at this index
    shl r0 r0 2
    imw r1 ^locals_names
    ldw r1 rpp r1
    ldw r1 r1 r0
    ldw r0 rfp -4
    call ^strcmp
    jz r0 &locals_find_found

    ; increment the index
    ldw r0 rfp -12
    inc r0
    stw r0 rfp -12
    jmp &locals_find_loop

:locals_find_found

    ; variable found! its offset is (4 - 4 * index)
    ldw r8 rfp -12
    shl r0 r8 2
    add r0 r0 4
    ldw r1 rfp -8
    stw r0 r1 0

    ; return the type
    imw r9 ^locals_types
    ldw r9 rpp r9
    ldb r0 r9 r8
    leave
    ret

:locals_find_not_found
    ; not found; return zero
    zero r0
    leave
    ret
