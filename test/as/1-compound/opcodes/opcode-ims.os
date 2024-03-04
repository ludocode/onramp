=test

; invocations
ims ra <foo
ims ra >foo
ims ra &foo

; numbers
ims r0 1234
ims r1 -1234
ims r2 0xabcd
ims r3 -0x9876

; individual bytes
ims r4 '12 '34
ims r5 'ab "c"
ims r6 "d" 'ef
ims r7 "h" "g"

=foo

=main
    zero r0
    ret
