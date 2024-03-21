; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=test

; invocations
imw ra ^foo
imw rb ^bar

; numbers
imw r0 1234567890
imw r1 -1234567890
imw r2 0xabcdef98
imw r3 -0x76543210

; individual bytes
imw r4 '12 '34 '56 '78
imw r5 'ab 'cd 'ef "g"
imw r6 "h" 'ee "l" '00
imw r7 "a" "b" "c" "d"

=foo
=bar

=main
    zero r0
    ret
