=test
; registers
r0 r1 r2 r3
r4 r5 r6 r7
r8 r9 ra rb
rsp rfp rpp rip
; opcodes
add sub mul divu
and or xor ror
ldw stw ldb stb
ims cmpu jz sys
; syscalls
halt time spawn
open close read write seek tell truncate type
stat rename symlink unlink chmod

=main
    add r0 '00 '00      ; zero r0
    ldw rip '00 rsp    ; ret
