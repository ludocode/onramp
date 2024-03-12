=main

    imw r0 0x12345678
    isz r0 r0
    jnz r0 &fail

    zero r9
    isz r9 r9
    jz r9 &fail

    mov r1 'ee
    isz r1 r1
    jnz r1 &fail

    zero r0
    ret
    
:fail
    mov r0 1
    ret
