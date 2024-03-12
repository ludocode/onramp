=test
push 0
push 0xFFFFFFFF
push -1
push -0x55
push r0
push "a"

=main
    zero r0
    ret
