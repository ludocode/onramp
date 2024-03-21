; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=main
    ims r0 'aa 'aa
    ims r0 'aa 'aa
    shrs r0 r0 4
    add r0 '00 '00
    ret
