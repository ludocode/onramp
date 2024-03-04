=foo
'00:bar
'10@baz
<a>b^c
'ab'cd&d

=a
=b
=c
=d

=main
    add r0 '00 '00     ; zero r0
    ldw rip '00 rsp    ; ret
