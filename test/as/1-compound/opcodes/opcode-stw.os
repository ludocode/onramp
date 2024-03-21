; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=test
stw r0 rfp -4
stw 0 rfp -8

=main
    zero r0
    ret
