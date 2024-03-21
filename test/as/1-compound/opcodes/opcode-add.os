; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=test
add r1 r2 r3
add r4 r5 '06
add r7 '08 '09
add rsp 'AB rfp
add ra 0 10
add rb -2 -16
add rb 0x12 0x34
add rb 0x5A 0x7F

=main
    zero r0
    ret
