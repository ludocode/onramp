; The MIT License (MIT)
; Copyright (c) 2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main
    ; 5 >> 2 == 1
    mov r0 5
    mov r1 2
    shrs r0 r0 r1
    sub r0 r0 1
    jz r0 &ok_1
    mov r0 1
    ret
:ok_1

    ; 5 >> 0 == 5
    mov r0 5
    mov r1 0
    shrs r0 r0 r1
    sub r0 r0 5
    jz r0 &ok_2
    mov r0 2
    ret
:ok_2

    ; -5 >> 2 == -2
    mov r0 -5
    mov r1 2
    shrs r0 r0 r1
    sub r0 r0 -2
    jz r0 &ok_3
    mov r0 3
    ret
:ok_3

    ; -5 >> 0 == -5
    mov r0 -5
    mov r1 0
    shrs r0 r0 r1
    sub r0 r0 -5
    jz r0 &ok_4
    mov r0 4
    ret
:ok_4

    ; 4 >> 8 == 0
    mov r0 4
    mov r1 8
    shrs r0 r0 r1
    jz r0 &ok_5
    mov r0 5
    ret
:ok_5

    zero r0
    ret
