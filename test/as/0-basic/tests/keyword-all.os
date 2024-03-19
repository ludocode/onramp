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
fopen fclose fread fwrite fseek ftell ftrunc
dopen dclose dread
stat rename symlink unlink chmod mkdir rmdir

=main
    add r0 '00 '00      ; zero r0
    ldw rip '00 rsp    ; ret
