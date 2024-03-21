; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main

    ; sign extend positive short in positive word
    imw r0 0x12345678
    sxs r1 r0
    imw r3 0x5678
    sub r2 r1 r3
    jnz r2 &fail

    ; sign extend positive short in negative word
    imw r0 0xfedc4a78
    sxs r1 r0
    imw r3 0x4a78
    sub r2 r1 r3
    jnz r2 &fail

    ; sign extend negative short in positive word
    imw r0 0x456789a2
    sxs r0 r0
    imw r1 0xffff89a2
    sub r2 r0 r1
    jnz r2 &fail

    ; sign extend negative short in negative word
    imw r0 0x8000f7fe
    sxs r0 r0
    imw r1 0xfffff7fe
    sub r2 r0 r1
    jnz r2 &fail

    zero r0
    ret

:fail
    mov r0 1
    ret
