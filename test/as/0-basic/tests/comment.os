=test
;hello\
'12
'34;world
'56

=main
    add r0 '00 '00      ; zero r0
    ldw rip '00 rsp    ; ret
