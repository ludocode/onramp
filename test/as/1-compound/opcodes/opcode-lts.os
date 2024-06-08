; The MIT License (MIT)
; Copyright (c) 2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main

    lts r0 0 1
    jz r0 &fail

    lts r1 -1 0
    jz r1 &fail

    lts r2 0 0
    jnz r2 &fail

    imw r1 0x7FFFFFFF
    imw r2 0x80000000
    lts r0 r1 r2
    jnz r0 &fail
    lts r0 r2 r1
    jz r0 &fail

    zero r0
    ret

:fail
    mov r0 1
    ret
