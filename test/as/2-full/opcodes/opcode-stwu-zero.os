; The MIT License (MIT)
; Copyright (c) 2026 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

; stwu test where one argument is zero

=main
    push 0
    push 0
    push 0


    ; offset 1

    imw r0 0x87654321
    add r4 rsp 1
    stwu r0 r4 0

    ldw r1 rsp 0
    imw r2 0x65432100
    sub r3 r1 r2
    jnz r3 &fail

    ldw r1 rsp 4
    imw r2 0x00000087
    sub r3 r1 r2
    jnz r3 &fail

    ldw r1 rsp 8
    jnz r1 &fail

    stw 0 rsp 0
    stw 0 rsp 4


    ; offset 2

    imw r0 0x87654321
    add r4 rsp 2
    stwu r0 0 r4

    ldw r1 rsp 0
    imw r2 0x43210000
    sub r3 r1 r2
    jnz r3 &fail

    ldw r1 rsp 4
    imw r2 0x00008765
    sub r3 r1 r2
    jnz r3 &fail

    ldw r1 rsp 8
    jnz r1 &fail

    stw 0 rsp 0
    stw 0 rsp 4


    ; offset 3

    imw r0 0x87654321
    add r4 rsp 3
    stwu r0 r4 0

    ldw r1 rsp 0
    imw r2 0x21000000
    sub r3 r1 r2
    jnz r3 &fail

    ldw r1 rsp 4
    imw r2 0x00876543
    sub r3 r1 r2
    jnz r3 &fail

    ldw r1 rsp 8
    jnz r1 &fail

    stw 0 rsp 0
    stw 0 rsp 4


    ; offset 4

    imw r0 0x87654321
    add r4 rsp 4
    stwu r0 0 r4

    ldw r1 rsp 0
    jnz r1 &fail

    ldw r1 rsp 4
    imw r2 0x87654321
    sub r3 r1 r2
    jnz r3 &fail

    ldw r1 rsp 8
    jnz r1 &fail

    stw 0 rsp 0
    stw 0 rsp 4


    popd
    popd
    popd
    zero r0
    ret

:fail
    pop r0
    mov r0 1
    ret
