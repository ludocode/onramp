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
; Some of these are optimized for better performance.
;
; The null-terminated string functions (like strlen()) assume that for any byte
; that is readable, the word that contains it is also readable. This means all
; memory regions must be aligned to a 4-byte boundary. This will be required in
; the v4 spec; it wasn't explicit in earlier versions but we assume they
; provide the same guarantee.



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

    ; Start with a zero count
    zero r1

    ; Align the pointer to the next word, storing the alignment in r2
    and r2 r0 3
    jz r2 &__strlen_aligned
    add r8 r0 3
    and r8 r8 -4

    ; Our starting count is negative alignment; if a null-terminator is found
    ; we add alignment back in the jump table below.
    sub r1 0 r2

    ; Jump based on the alignment of the pointer
    shl r3 r2 3       ; jump two instructions (8 bytes) per alignment
    add rip rip r3

    ; This is our jump table. Non-zero cases fall through.
    0                         ; alignment 0 (unused)
    0
    ldb r9 r8 -3              ; alignment 1
    jz r9 &__strlen_null_1
    ldb r9 r8 -2              ; alignment 2
    jz r9 &__strlen_null_2
    ldb r9 r8 -1              ; alignment 3
    jz r9 &__strlen_null_3

    ; No null-terminator in the first few bytes. Our starting count is the
    ; inverse of the alignment.
    sub r1 4 r2

:__strlen_aligned

    ; Set up masks in r3-r7
    shru r3 -1 24   ; r3 = 0x000000ff
    shl r4 r3 8     ; r4 = 0x0000ff00
    shl r5 r3 16    ; r5 = 0x00ff0000
    shl r6 r3 24    ; r6 = 0xff000000

    ; Now we loop on words, checking each byte. Each iteration takes 11
    ; instructions to check for null bytes (with just a single load
    ; instruction), as opposed to 16 instructions (four of them loads) if we
    ; were to loop on bytes.
:__strlen_loop
    ldw r2 r0 r1
    and r9 r2 r3
    jz r9 &__strlen_null_0
    and r9 r2 r4
    jz r9 &__strlen_null_1
    and r9 r2 r5
    jz r9 &__strlen_null_2
    and r9 r2 r6
    jz r9 &__strlen_null_3
    add r1 r1 4
    jmp &__strlen_loop

    ; These are the function exits used once we've found a null-terminator. The
    ; suffix indicates at what byte offset it was found.
:__strlen_null_0
    add r0 r1 0
    ret
:__strlen_null_1
    add r0 r1 1
    ret
:__strlen_null_2
    add r0 r1 2
    ret
:__strlen_null_3
    add r0 r1 3
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
; This is the standard C memcpy() function. We simply defer to memmove().
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
; This is the standard C memmove() function. It is used everywhere (and is also
; used for memcpy()) so it has significant optimizations.
;
; If both pointers are word-aligned, we can copy by words until we have less
; than four bytes left. If we have any remaining bytes afterwards, or if either
; pointer is misaligned, we copy by bytes.
;
; params:
;     - dest: r0          ; original dest pointer, preserved as return value
;     - src_bytes: r1     ; src pointer when copying by bytes
;     - count_bytes: r2   ; bytes remaining when copying by bytes
; vars:
;     - dest_bytes: r3    ; dest pointer when copying by bytes
;     - dest_words: r4    ; dest pointer when copying by words
;     - src_words: r5     ; src pointer when copying by words
;     - count_words: r6   ; bytes (not words!) remaining when copying by words
;     - misaligned: r9    ; true if src or dest is misaligned (preventing copy by words)
; ==========================================================

=memmove
    ; don't bother to set up a stack frame

    ; A couple of checks here can avoid some useless copies. Not clear if this
    ; helps performance or not.
    jz r2 &__memmove_ret   ; if count is zero, nothing to do
    sub ra r0 r1
    jz ra &__memmove_ret   ; if pointers match, nothing to do

    ; Check if both dest and src are word-aligned. If so we can copy by words.
    or r9 r0 r1
    and r9 r9 3

    ; Check whether we should copy down or up
    ltu ra r1 r0
    jz ra &__memmove_up


    ; TODO we should actually split this into four sections:
    ;
    ; - matching alignment, copying up
    ; - matching alignment, copying down
    ; - differing alignment, copying up
    ; - differing alignment, copying down
    ;
    ; In cases of matching alignment, we shouldn't require that the pointers
    ; are word-aligned as above, but only that the alignment matches. We should
    ; copy bytes until word-aligned, then copy words, then finish copying any
    ; unaligned bytes.
    ;
    ; In cases of differing alignment, we should copy bytes until src is
    ; aligned, then read by words and store by bytes, then finish copying any
    ; unaligned bytes.
    ;
    ; In all of the above cases, we don't actually need to loop copying bytes;
    ; we will always copy at most three bytes. We can simply jump based on the
    ; number of bytes left.
    ;
    ; These additional optimizations are probably not that important because
    ; all pointers returned by malloc() are word-aligned so almost everything
    ; we copy will be word-aligned by default.


    ; Copy down (from the end to the start)
    ;
    ; These copy loops are very straightforward: we use the size as offset to
    ; the load/store instructions and decrement until it is zero.

:__memmove_down
    jz r9 &__memmove_down_words
    add r3 r0 0

    ; copy down by bytes
:__memmove_down_bytes_loop
    jz r2 &__memmove_ret
    dec r2
    ldb ra r1 r2
    stb ra r3 r2
    jmp &__memmove_down_bytes_loop

    ; copy down by words
:__memmove_down_words

    ; split counts
    and r6 r2 -4   ; count_words = count & ~3
    and r2 r2 3    ; count_bytes = count & 3

    ; split pointers
    add r4 r0 0    ; dest_words = dest
    add r3 r0 r6   ; dest_bytes = dest_words + count_words
    add r5 r1 0    ; src_words = src
    add r1 r1 r6   ; src_bytes = src + count_words

    ; loop on words, falling back to bytes when we run out of words
:__memmove_down_words_loop
    jz r6 &__memmove_down_bytes_loop
    sub r6 r6 4
    ldw ra r5 r6
    stw ra r4 r6
    jmp &__memmove_down_words_loop


    ; Copy up (from the start to the end)
    ;
    ; When copying up, we adjust the pointers to point to the end and we make
    ; the size negative. This allows us to increment the size in the loop and
    ; jump out when it reaches zero, avoiding a comparison inside the loop.

    ; copy up
:__memmove_up
    jz r9 &__memmove_up_words

    ; invert pointers and size
    add r3 r0 r2   ; dest = dest + size
    add r1 r1 r2   ; src = src + size
    sub r2 0 r2    ; size = -size

    ; copy up by bytes
:__memmove_up_bytes_loop
    jz r2 &__memmove_ret
    ldb ra r1 r2
    stb ra r3 r2
    inc r2
    jmp &__memmove_up_bytes_loop

:__memmove_up_words

    ; split counts
    and r6 r2 -4   ; count_words = count & ~3
    and r2 r2 3    ; count_bytes = count & 3

    ; split and invert pointers
    add r4 r0 r6   ; dest_words = dest + count_words
    add r3 r4 r2   ; dest_bytes = dest_words + count_bytes
    add r5 r1 r6   ; src_words = src_bytes + count_words
    add r1 r5 r2   ; src_bytes = src_words + count_bytes

    ; invert counts
    sub r6 0 r6
    sub r2 0 r2

    ; loop on words, falling back to bytes when we run out of words
:__memmove_up_words_loop
    jz r6 &__memmove_up_bytes_loop
    ldw ra r5 r6
    stw ra r4 r6
    add r6 r6 4
    jmp &__memmove_up_words_loop


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
:__memset_loop
    jz r2 &__memset_done
    dec r2
    stb r1 r0 r2
    jmp &__memset_loop
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
