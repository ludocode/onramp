; TODO this was written before we were linking against libc, hack to workaround it
;=_start
;    ; format indicator
;    "~Onr~amp~   "
=main
ims r0 <__process_info_table
ims r0 >__process_info_table
ldw r0 rpp r0

    ; store process info table in r9
    add r9 r0 '00

    ; get the output file descriptor
    add ra r9 '10
    ldw r0 ra '00

    ; write(stdout, "Hello world!", 12)
    ims r1 <hello_world
    ims r1 >hello_world
    add r1 rpp r1   ; set output buffer parameter ("Hello world!")
    add r2 '00 '0D  ; set output length parameter (13)
    sys write '00 '00

    ; get the exit address
    add ra r9 '08
    ldw ra ra '00

    ; exit(0)
    add r0 '00 '00   ; set exit parameter to 0 (success)
    add rip ra '00   ; jump to exit address

=hello_world
    "Hello world!" '0A
