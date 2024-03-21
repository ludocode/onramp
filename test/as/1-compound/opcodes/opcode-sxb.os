; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main

    ; sign extend positive byte in positive word
    imw r0 0x12345678
    sxb r1 r0
    sub r2 r1 0x78
    jnz r2 &fail

    ; sign extend positive byte in negative word
    imw r0 0xfedcba78
    sxb r1 r0
    sub r2 r1 '78
    jnz r2 &fail

    ; sign extend negative byte in positive word
    imw r0 0x456789a
    sxb r0 r0
    sub r2 r0 0xffffff9a
    jnz r2 &fail

    ; sign extend negative byte in negative word
    imw r0 0x8000fffe
    sxb r0 r0
    sub r2 r0 'fe
    jnz r2 &fail

    zero r0
    ret

:fail
    mov r0 1
    ret
