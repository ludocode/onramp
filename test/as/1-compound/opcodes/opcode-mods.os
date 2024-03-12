=main
    ; positive dividend, positive divisor
    mods r0 19 7
    sub r1 r0 5
    jnz r1 &fail

    ; positive dividend, negative divisor
    mods r0 -19 7
    sub r1 r0 -5
    jnz r1 &fail

    ; negative dividend, positive divisor
    mods r0 19 -7
    sub r1 r0 5
    jnz r1 &fail

    ; negative dividend, negative divisor
    mods r0 -19 -7
    sub r1 r0 -5
    jnz r1 &fail

    ; modulo of INT_MIN (magnitude not representable as signed int)
    imw r2 0x80000000
    imw r3 7919
    mods r0 r2 r3
    imw r4 -1309
    sub r1 r0 r4
    jnz r1 &fail

    zero r0
    ret
    
:fail
    mov r0 1
    ret
