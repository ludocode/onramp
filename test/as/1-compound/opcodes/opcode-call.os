; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=foo
    mov r0 1
    ret

=bar
    mov r0 2
    ret

=main
    call ^foo
    sub r0 1 r0
    jnz r0 &fail

    imw r3 ^bar
    add r3 rpp r3
    call r3
    sub r0 2 r0
    jnz r0 &fail

    zero r0
    ret

:fail
    mov r0 1
    ret
