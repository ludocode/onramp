; The MIT License (MIT)
;
; Copyright (c) 2023-2024 Fraser Heavy Software
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



; ==========================================================
; int setjmp(int registers[4]);
; ==========================================================
; The standard setjmp() function.
;
; We preserve only the callee-preserved registers rsp, rfp, rpp, and setjmp()'s
; return address for rip, in that order. Since this is a function, the caller
; must have already preserved any other registers they care about.
;
; Although it's not reasonable for rpp to differ between setjmp() and
; longjmp(), it is technically callee-preserved so we preserve it anyway. This
; means it is technically possible to longjmp() out of a process into a parent
; process in Onramp. This would be insane, so please don't do it; there is no
; guarantee that this behaviour will continue to work in the future.
; ==========================================================

=setjmp
    ; store the first three callee-preserved registers as-is
    stw rsp r0 0
    stw rfp r0 4
    stw rpp r0 8

    ; the value we want to preserve for rip is the return value at rsp
    ldw r9 rsp 0
    stw r9 r0 12

    ; return 0
    zero r0
    ret



; ==========================================================
; [[noreturn]] void longjmp(int registers[4], int return_value);
; ==========================================================
; The standard longjmp() function.
;
; We restore the callee-preserved registers stored with setjmp(), returning
; from setjmp() again with the given return value.
; ==========================================================

=longjmp
    ; restore the first three callee-preserved registers
    ldw rsp r0 0
    ldw rfp r0 4
    ldw rpp r0 8

    ; the fourth value is the instruction pointer
    ldw r9 r0 12

    ; place the given return value in r0.
    ; if it's zero, we are supposed to coerce it to 1.
    mov r0 r1
    jnz r0 &__longjmp_ret
    mov r0 1
:__longjmp_ret

    ; jump
    mov rip r9
