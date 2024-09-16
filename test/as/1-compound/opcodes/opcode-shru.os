; The MIT License (MIT)
; Copyright (c) 2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main
    ; 5u >> 2u == 1u
    mov r0 5
    mov r1 2
    shru r0 r0 r1
    sub r0 r0 1
    jz r0 &ok_1
    mov r0 1
    ret
:ok_1

    ; 5u >> 0u == 5u
    mov r0 5
    mov r1 0
    shru r0 r0 r1
    sub r0 r0 5
    jz r0 &ok_2
    mov r0 2
    ret
:ok_2

    zero r0
    ret
