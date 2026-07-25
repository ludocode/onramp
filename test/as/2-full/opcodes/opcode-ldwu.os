; The MIT License (MIT)
; Copyright (c) 2026 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

; ldwu test where neither argument is zero

=main

    imw r0 0xfedcba98
    push r0
    imw r0 0x76543210
    push r0

    imw r0 0xffffffff
    imw r2 0xffffffff
    imw r3 0xffffffff

    imw r1 0
    ldwu r1 rsp 1
    imw r2 0x98765432
    sub r3 r1 r2
    jnz r3 &fail

    imw r1 0x55555555
    ldwu r1 rsp 2
    imw r2 0xba987654
    sub r3 r1 r2
    jnz r3 &fail

    imw r1 0xBBBBBBBB
    ldwu r1 rsp 3
    imw r2 0xdcba9876
    sub r3 r1 r2
    jnz r3 &fail

    imw r1 0xffffffff
    ldwu r1 rsp 4
    imw r2 0xfedcba98
    sub r3 r1 r2
    jnz r3 &fail

    popd
    popd
    zero r0
    ret

:fail
    pop r0
    mov r0 1
    ret
