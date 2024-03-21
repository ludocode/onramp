; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main

    imw r0 0x12345678
    bool r0 r0
    jz r0 &fail

    zero r0
    bool r1 r0
    jnz r1 &fail

    mov r0 'ee
    bool r9 r0
    jz r9 &fail

    zero r0
    ret
    
:fail
    mov r0 1
    ret
