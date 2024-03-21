; The MIT License (MIT)
; Copyright (c) 2023-2024 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

=test
call ^foo
call ^bar

=foo
=bar

=main
    zero r0
    ret
