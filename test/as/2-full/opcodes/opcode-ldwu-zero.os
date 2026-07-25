; The MIT License (MIT)
; Copyright (c) 2026 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

; ldwu test where one argument is zero

=main

    imw r0 0xfedcba98
    push r0
    imw r0 0x76543210
    push r0

    imw r0 0xffffffff
    imw r2 0xffffffff
    imw r3 0xffffffff

    imw r1 0
    add r4 rsp 1
    ldwu r1 r4 0
    imw r2 0x98765432
    sub r3 r1 r2
    jnz r3 &fail

    imw r1 0x55555555
    add r4 rsp 2
    ldwu r1 0 r4
    imw r2 0xba987654
    sub r3 r1 r2
    jnz r3 &fail

    imw r1 0xBBBBBBBB
    add r4 rsp 3
    ldwu r1 r4 0
    imw r2 0xdcba9876
    sub r3 r1 r2
    jnz r3 &fail

    imw r1 0xffffffff
    add r4 rsp 4
    ldwu r1 0 r4
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
