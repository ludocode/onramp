; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main

    ; divs test where the destination register does not match either of the
    ; inputs. the assembler can emit shorter, more optimized code.

    ; both positive
    divs r0 7 3
    sub r0 r0 2
    jnz r0 &fail

    ; positive dividend, negative divisor
    imw r0 0x7FFFFFFF
    imw r1 -31
    divs r2 r0 r1
    imw r3 -69273666
    sub r0 r2 r3
    jnz r0 &fail

    ; negative dividend, positive divisor
    imw r0 -719
    divs r1 r0 67
    sub r1 r1 -10
    jnz r1 &fail

    ; both negative
    imw r0 -600
    divs r1 r0 -20
    sub r1 r1 30
    jnz r1 &fail

    ; division of INT_MIN (magnitude not representable as signed int)
    imw r0 0x80000000
    divs r1 r0 1
    sub r1 r0 r1
    jnz r1 &fail

    zero r0
    ret
    
:fail
    mov r0 1
    ret
