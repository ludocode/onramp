=main

    ; truncate positive
    imw r0 0x12345678
    trs r1 r0
    imw r3 0x5678
    sub r2 r1 r3
    jnz r2 &fail

    ; truncate negative
    imw r0 0x8000fffe
    trs r0 r0
    imw r1 0x0000fffe
    sub r2 r0 r1
    jnz r2 &fail

    zero r0
    ret

:fail
    mov r0 1
    ret
