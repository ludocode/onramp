@_F_main 
  imw r0 0x61 
  push r0 
  imw r0 ^stdout 
  add r0 rpp r0 
  ldw r0 0 r0 
  push r0 
  ldw r0 rsp 4 
  ldw r1 rsp 0 
  call ^fputc 
  add rsp rsp 8 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
