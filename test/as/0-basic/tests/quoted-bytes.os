; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=test
   '01 '02
   'fe 'FF
   '01'23'45'67'89'aB'Cd'ef

=main
    add r0 '00 '00     ; zero r0
    ldw rip '00 rsp    ; ret
