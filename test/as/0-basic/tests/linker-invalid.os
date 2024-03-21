; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=test
; TODO this is currently disabled because this doesn't link, and I don't really
; want to add code to run.sh to handle tests that assemble but don't link just
; for this one test. There's no point in checking this anyway.
;^0      ; TODO this is invalid but we don't diagnose it yet
;^       ; so is this
;^"foo"  ; and this

=main
    add r0 '00 '00     ; zero r0
    ldw rip '00 rsp    ; ret
