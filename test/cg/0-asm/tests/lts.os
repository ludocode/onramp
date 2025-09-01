; The MIT License (MIT)
; Copyright (c) 2025 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main
    lts r0 1 0
    jnz r0 &main_fail

    lts r0 0 1
    sub r0 r0 1
    jnz r0 &main_fail

    lts r0 -1 0
    sub r0 r0 1
    jnz r0 &main_fail

    lts r0 0xFFFFFF90 0x79
    sub r0 r0 1
    jnz r0 &main_fail

    zero r0
    ret

:main_fail
    mov r0 1
    ret
