; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main

    ; divs test where the destination register matches the dividend.

    ; both positive
    mov r0 7
    divs r0 r0 3
    sub r0 r0 2
    jnz r0 &fail

    ; positive dividend, negative divisor
    imw r0 0x7FFFFFFF
    imw r1 -31
    divs r0 r0 r1
    imw r2 -69273666
    sub r0 r0 r2
    jnz r0 &fail

    ; negative dividend, positive divisor
    imw r0 -719
    divs r0 r0 67
    sub r1 r0 -10
    jnz r1 &fail

    ; both negative
    imw r0 -600
    divs r0 r0 -20
    sub r1 r0 30
    jnz r1 &fail

    ; division of INT_MIN (magnitude not representable as signed int)
    imw r0 0x80000000
    divs r0 r0 1
    imw r1 0x80000000
    sub r1 r0 r1
    jnz r1 &fail

    zero r0
    ret
    
:fail
    mov r0 1
    ret
