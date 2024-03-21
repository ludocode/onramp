; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main

    push 0
    mov r0 rsp

    imw r2 0x5678
    sts r2 rsp 0
    imw r2 0x1234
    sts r2 rsp 2

    ldw r1 rsp 0
    imw r3 0x12345678
    sub r4 r1 r3
    jnz r4 &fail

    pop r0
    zero r0
    ret
    
:fail
    pop r0
    mov r0 1
    ret
