; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main
    modu r0 19 7
    sub r1 r0 5
    jnz r1 &fail

    modu r0 30 5
    jnz r0 &fail

    zero r0
    ret
    
:fail
    mov r0 1
    ret
