; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main

    imw r0 0x12345678
    push r0

    lds r1 rsp 0
    imw r2 0x5678
    sub r3 r1 r2
    jnz r3 &fail

    lds r1 rsp 2
    imw r2 0x1234
    sub r3 r1 r2
    jnz r3 &fail

    pop r0
    zero r0
    ret
    
:fail
    pop r0
    mov r0 1
    ret
