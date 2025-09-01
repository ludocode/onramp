; The MIT License (MIT)
; Copyright (c) 2025 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main
    mov r1 1

    ; xor with two constants
    xor r0 3 5
    sub r0 r0 6
    jnz r0 &main_fail

    ; xor with left zero
    xor r0 0 r1
    sub r0 r0 1
    jnz r0 &main_fail

    ; xor with right zero
    xor r0 r1 0
    sub r0 r0 1
    jnz r0 &main_fail

    ; xor with left -1
    xor r0 r1 -1
    sub r0 r0 -2
    jnz r0 &main_fail

    ; xor with right -1
    xor r0 r1 -1
    sub r0 r0 -2
    jnz r0 &main_fail

    zero r0
    ret

:main_fail
    mov r0 1
    ret
