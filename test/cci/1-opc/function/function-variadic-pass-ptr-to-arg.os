=x 
'00'00'00'00
@_F_bar 
  stw r0 rfp -4 
  imw r0 ^x 
  add r0 rpp r0 
  push r0 
  add r0 rfp -4 
  ldw r0 0 r0 
  mov r1 r0 
  ldw r0 0 r0 
  ldw r0 0 r0 
  push r0 
  mov r0 r1 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  shl r0 r0 2 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  pop r0 
  ldw r1 rsp 0 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  zero r0 
  leave 
  ret 
=bar 
  enter 
  sub rsp rsp 4 
  jmp ^_F_bar 
@_F_vfoo 
  stw r0 rfp -4 
  stw r1 rfp -8 
  add r0 rfp -4 
  ldw r0 0 r0 
  add r0 rfp -8 
  push r0 
  ldw r0 rsp 0 
  call ^bar 
  add rsp rsp 4 
  add r0 rfp -8 
  push r0 
  ldw r0 rsp 0 
  call ^bar 
  add rsp rsp 4 
  add r0 rfp -8 
  push r0 
  ldw r0 rsp 0 
  call ^bar 
  add rsp rsp 4 
  zero r0 
  leave 
  ret 
=vfoo 
  enter 
  sub rsp rsp 8 
  jmp ^_F_vfoo 
@_F_foo 
  stw r0 rfp -4 
  add r0 rfp -4 
  ldw r0 0 r0 
  add r0 rfp -8 
  mov r1 r0 
  add r0 rfp 8 
  stw r0 0 r1 
  add r0 rfp -4 
  ldw r0 0 r0 
  push r0 
  add r0 rfp -8 
  ldw r0 0 r0 
  push r0 
  ldw r0 rsp 4 
  ldw r1 rsp 0 
  call ^vfoo 
  add rsp rsp 8 
  add r0 rfp -8 
  zero r0 
  leave 
  ret 
=foo 
  enter 
  sub rsp rsp 8 
  jmp ^_F_foo 
@_F_main 
  imw r0 0 
  push r0 
  imw r0 1 
  push r0 
  imw r0 2 
  push r0 
  imw r0 3 
  push r0 
  ldw r0 rsp 12 
  ldw r9 rsp 0 
  push r9 
  ldw r9 rsp 8 
  push r9 
  ldw r9 rsp 16 
  push r9 
  call ^foo 
  add rsp rsp 28 
  imw r0 ^x 
  add r0 rpp r0 
  push r0 
  imw r0 6 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
