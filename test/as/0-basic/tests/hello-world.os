; The MIT License (MIT)
; Copyright (c) 2023-2025 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main

    ; get arguments for fwrite("Hello world!\n", 1, 13, stdout)
    ims r0 <hello_world
    ims r0 >hello_world
    add r0 rpp r0   ; r0 = "Hello world!"
    add r1 '00 '01  ; r1 = 1
    add r2 '00 '0D  ; r2 = 13
    ims r3 <stdout
    ims r3 >stdout
    ldw r3 rpp r3

    ; call fwrite()
    ims ra <fwrite
    ims ra >fwrite
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra      ; jump
    add rsp rsp '04     ; pop return address

    ; return 0
    add r0 '00 '00
    ldw rip '00 rsp     ; ret

=hello_world
    "Hello world!" '0A
