; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main

    ; divs test where the destination register matches the divisor.

    ; both positive
    mov r0 3
    divs r0 7 r0
    sub r0 r0 2
    jnz r0 &fail

    ; positive dividend, negative divisor
    imw r0 0x7FFFFFFF
    imw r1 -31
    divs r1 r0 r1
    imw r2 -69273666
    sub r0 r1 r2
    jnz r0 &fail

    ; negative dividend, positive divisor
    imw r0 -719
    imw r1 67
    divs r1 r0 r1
    sub r1 r1 -10
    jnz r1 &fail

    ; both negative
    imw r0 -600
    mov r1 -20
    divs r1 r0 r1
    sub r1 r1 30
    jnz r1 &fail

    ; division of INT_MIN (magnitude not representable as signed int)
    imw r0 0x80000000
    mov r1 1
    divs r1 r0 r1
    imw r2 0x80000000
    sub r1 r1 r2
    jnz r1 &fail

    zero r0
    ret
    
:fail
    mov r0 1
    ret
