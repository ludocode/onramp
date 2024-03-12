=main

    ; truncate positive
    imw r0 0x12345678
    trb r1 r0
    sub r2 r1 0x78
    jnz r2 &fail

    ; truncate negative
    imw r0 0x8000fffe
    trb r0 r0
    imw r1 0x000000fe
    sub r2 r0 r1
    jnz r2 &fail

    zero r0
    ret

:fail
    mov r0 1
    ret
