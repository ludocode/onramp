; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=hello_world"Hello world!"'0A=foo"~"'00&hello_world

=main
    add r0 '00 '00     ; zero r0
    ldw rip '00 rsp    ; ret
