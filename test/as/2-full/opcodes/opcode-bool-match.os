; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main

    ; bool test where the destination register matches the source

    imw r0 0x12345678
    bool r0 r0
    jz r0 &fail

    zero r9
    bool r9 r9
    jnz r9 &fail

    mov r1 'ee
    bool r1 r1
    jz r1 &fail

    zero r0
    ret
    
:fail
    mov r0 1
    ret
