; The MIT License (MIT)
;
; Copyright (c) 2023-2026 Fraser Heavy Software
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



; This is a copy of libc/0 string.oo with the bytecode replaced by assembly.
;
; Eventually these functions will be optimized.



; ==========================================================
; size_t strlen(char* str)
; ==========================================================
; Returns the length of the given null-terminated string, not including the
; null-terminator.
;
; This is the standard C strlen() function.
;
; params:
;     - str: r0
; vars:
;     - count: r1
; ==========================================================

=strlen
    ; don't bother to set up a stack frame
    ; we accumulate the count in r1, then move it to r0
    zero r1
:__strlen_loop
    ldb ra r0 r1
    jz ra &__strlen_done
    inc r1
    jz 0 &__strlen_loop
:__strlen_done
    mov r0 r1
    ret



; ==========================================================
; char* strcpy(char* dest, const char* src)
; ==========================================================
; Copies null-terminated string src to dest, returning dest.
;
; This is the standard C strcpy() function.
;
; params:
;     - dest: r0
;     - src: r1
; vars:
;     - index: r2
; ==========================================================

=strcpy
    ; don't bother to set up a stack frame
    ; we use an index because it's faster than incrementing both src and dest
    zero r2
:__strcpy_loop
    ldb ra r1 r2
    stb ra r0 r2
    jz ra &__strcpy_done
    inc r2
    jmp &__strcpy_loop
:__strcpy_done
    ; dest is already in r0
    ret



; ==========================================================
; int strcmp(const char* left, const char* right)
; ==========================================================
; Compares null-terminated strings left to right lexicographically. Returns -1
; if left comes before right, 1 if left comes after right, 0 if they are equal.
;
; This is the standard C strcmp() function.
;
; params:
;     - left: r0
;     - right: r1
; vars:
;     - index: r2
; ==========================================================

=strcmp
    ; don't bother to set up a stack frame

    ; we use an index because it's faster than incrementing both src and dest
    zero r2

:__strcmp_loop
    ldb ra r0 r2
    ldb rb r1 r2
    sub r3 ra rb
    jz r3 &__strcmp_byte_match

    ; bytes not equal, return a comparison result
    ltu ra ra rb
    jz ra &__strcmp_greater
:__strcmp_less
    mov r0 -1
    ret
:__strcmp_greater
    mov r0 1
    ret

:__strcmp_byte_match
    ; bytes match. if zero, strings are equal, return 0
    jz rb &__strcmp_equal

    ; otherwise keep looping
    inc r2
    jmp &__strcmp_loop

:__strcmp_equal
    ; strings are equal, return 0
    zero r0
    ret



; ==========================================================
; void* memcpy(void* dest, const void* src, size_t count)
; ==========================================================
; Copies count bytes from src to the non-overlapping memory region dest.
;
; This is the standard C memcpy() function.
;
; params:
;     - dest: r0
;     - src: r1
;     - count: r2
; ==========================================================

=memcpy
    ; tail-call memmove
    jmp ^memmove



; ==========================================================
; void* memmove(void* dest, const void* src, size_t count)
; ==========================================================
; Copies count bytes from src to the potentially overlapping memory region
; dest.
;
; This is the standard C memmove() function.
;
; params:
;     - dest: r0
;     - src: r1
;     - count: r2
;     - index: r3
; ==========================================================

=memmove
    ; don't bother to set up a stack frame

    ; initialize
    zero r3

    ; check whether we should copy down or up
    ltu ra r1 r0
    jz ra &__memmove_up

    ; copy down
:__memmove_down
    jz r2 &__memmove_ret
    dec r2
    ldb ra r1 r2
    stb ra r0 r2
    jmp &__memmove_down

    ; copy up
:__memmove_up
    sub ra r2 r3
    jz ra &__memmove_ret
    ldb ra r1 r3
    stb ra r0 r3
    inc r3
    jmp &__memmove_up

    ; done
:__memmove_ret
    ; dest is already in r0
    ret



; ==========================================================
; void* memset(void* dest, int c, size_t count)
; ==========================================================
; Sets count bytes in dest to the byte c, returning dest.
;
; This is the standard C memset() function.
;
; params:
;     - dest: r0
;     - src: r1
;     - count: r2
; ==========================================================

=memset
    ; don't bother to set up a stack frame
    jz r2 &__memset_done
    dec r2
    stb r1 r0 r2
    jmp &memset
:__memset_done
    ; dest is already in r0
    ret



; ==========================================================
; int memcmp(const char* left, const char* right, size_t count)
; ==========================================================
; Compares left to right regions of count length lexicographically. Returns -1
; if left comes before right, 1 if left comes after right, 0 if they are equal.
;
; This is the standard C memcmp() function.
;
; params:
;     - left: r0
;     - right: r1
;     - count: r2
; vars:
;     - index: r3
; ==========================================================
=memcmp
    zero r3

:__memcmp_loop
    sub ra r2 r3
    jz ra &__memcmp_equal

    ldb ra r0 r3
    ldb rb r1 r3
    sub r4 ra rb
    jz r4 &__memcmp_byte_match

    ; bytes not equal, return a comparison result
    ; TODO this would be much simpler if we just used the subtraction result,
    ; except the rest of Onramp may assume the value is -1 or 1 because that's
    ; what cmpu did, this needs to be fixed
    ltu ra ra rb
    jz ra &__memcmp_greater
:__memcmp_less
    mov r0 -1
    ret
:__memcmp_greater
    mov r0 1
    ret

:__memcmp_byte_match
    ; bytes match, keep looping
    inc r3
    jmp &__memcmp_loop

:__memcmp_equal
    ; memory regions are equal, return 0
    zero r0
    ret



; ==========================================================
; char* strchr(const char* s, int c);
; ==========================================================
; Returns a pointer to the first occurrance of c in s, or NULL if s does not
; contain c.
; ==========================================================

=strchr

:strchr_loop
    ; we check for a match before checking for a null byte because the given
    ; character might be null.
    ldb ra r0 0
    sub rb ra r1
    jz rb &strchr_match
    jz ra &strchr_no_match
    inc r0
    jmp &strchr_loop

:strchr_no_match
    zero r0
    ; fallthrough
:strchr_match
    ret
