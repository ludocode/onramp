; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

; TODO this was written before we were linking against libc. it makes direct syscalls.
=main

    ; get the output file descriptor
    ims r9 <__process_info_table
    ims r9 >__process_info_table
    ldw r9 rpp r9
    ldw r0 r9 '10

    ; write(stdout, "Hello world!", 12)
    ims r1 <hello_world
    ims r1 >hello_world
    add r1 rpp r1   ; set output buffer parameter ("Hello world!")
    add r2 '00 '0D  ; set output length parameter (13)
    sys fwrite '00 '00

    ; get the exit address
    ims r9 <__process_info_table
    ims r9 >__process_info_table
    ldw r9 rpp r9
    ldw ra r9 '08

    ; exit(0)
    add r0 '00 '00   ; set exit parameter to 0 (success)
    add rip ra '00   ; jump to exit address

=hello_world
    "Hello world!" '0A
