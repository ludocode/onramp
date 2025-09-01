; The MIT License (MIT)
; Copyright (c) 2025 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main
    ltu r0 1 0
    jnz r0 &main_fail

    ltu r0 0 1
    sub r0 r0 1
    jnz r0 &main_fail

    ltu r0 -1 0  ; -1 here is actually 0xffffffff, the comparison is unsigned
    jnz r0 &main_fail

    ltu r0 0xFFFFFF90 0x79
    jnz r0 &main_fail

    zero r0
    ret

:main_fail
    mov r0 1
    ret
