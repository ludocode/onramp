; The MIT License (MIT)
; Copyright (c) 2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main

    ltu r0 0 1
    jz r0 &fail

    ltu r1 1 2
    jz r1 &fail

    ltu r2 0 0
    jnz r2 &fail

    ltu r9 1 0
    jnz r9 &fail

    imw r0 0xFFFFFFFF
    ltu r0 r0 0
    jnz r0 &fail

    imw r0 0xFFFFFFFF
    ltu r0 r0 r0
    jnz r0 &fail

    imw r0 0xFFFFFFFF
    imw r1 0x7FFFFFFF
    ltu r0 r1 r0
    jz r0 &fail

    zero r0
    ret

:fail
    mov r0 1
    ret
