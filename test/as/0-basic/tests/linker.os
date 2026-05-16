; The MIT License (MIT)
; Copyright (c) 2023-2026 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=foo
'00:bar
'10@baz
<a>b:x^c
'ab'cd&x

=a
=b
=c

=main
    add r0 '00 '00     ; zero r0
    ldw rip '00 rsp    ; ret
