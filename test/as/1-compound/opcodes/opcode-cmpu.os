; The MIT License (MIT)
; Copyright (c) 2025 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main

    ; less
    cmpu r0 3 7
    sub r0 r0 -1
    jnz r0 &fail
    cmpu r0 3 -7
    sub r0 r0 -1
    jnz r0 &fail

    ; greater
    cmpu r0 7 3
    sub r0 r0 1
    jnz r0 &fail
    cmpu r0 -7 3
    sub r0 r0 1
    jnz r0 &fail

    ; equal
    cmpu r0 7 7
    sub r0 r0 0
    jnz r0 &fail
    cmpu r0 -7 -7
    sub r0 r0 0
    jnz r0 &fail

    ; TODO test register args, if we decide to keep this opcode

    zero r0
    ret

:fail
    mov r0 1
    ret
