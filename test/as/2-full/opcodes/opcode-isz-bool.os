=main

    imw r0 0x12345678
    isz r1 r0
    jnz r1 &fail

    zero r0
    isz r1 r0
    jz r1 &fail

    mov r0 'ee
    isz r9 r0
    jnz r9 &fail

    zero r0
    ret
    
:fail
    mov r0 1
    ret
