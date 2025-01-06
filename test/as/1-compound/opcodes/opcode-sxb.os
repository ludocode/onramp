; The MIT License (MIT)
; Copyright (c) 2023-2025 Fraser Heavy Software
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

    ; boundary case 0xffffffff (-1 int)
    imw r0 0xffffffff
    sxb r0 r0
    imw r1 0xffffffff
    sub r2 r0 r1
    jnz r2 &fail

    ; boundary case 0xff (-1 byte)
    imw r0 0xff
    sxb r0 r0
    imw r1 0xffffffff
    sub r2 r0 r1
    jnz r2 &fail

    ; boundary case 0x7f (SCHAR_MAX == 127)
    imw r0 0x7f
    sxb r0 r0
    imw r1 0x7f
    sub r2 r0 r1
    jnz r2 &fail

    ; boundary case 0x80 (SCHAR_MIN == -128)
    imw r0 0x80
    sxb r0 r0
    imw r1 0xffffff80
    sub r2 r0 r1
    jnz r2 &fail

    ; boundary case 0
    imw r0 0
    sxb r0 r0
    imw r1 0
    sub r2 r0 r1
    jnz r2 &fail

    ; boundary case 1
    imw r0 1
    sxb r0 r0
    imw r1 1
    sub r2 r0 r1
    jnz r2 &fail

    zero r0
    ret

:fail
    mov r0 1
    ret
