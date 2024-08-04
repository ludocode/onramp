@_F_main 
  imw r0 8 
  push r0 
  imw r0 8 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  imw r0 8 
  push r0 
  imw r0 8 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 2 
  leave 
  ret 
:_Lx1 
  add r0 rfp -12 
  push r0 
  add r0 rfp -8 
  add r0 r0 6 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -12 
  ldw r0 0 r0 
  push r0 
  add r0 rfp -8 
  pop r1 
  sub r0 r1 r0 
  push r0 
  imw r0 6 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 3 
  leave 
  ret 
:_Lx2 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  sub rsp rsp 12 
  jmp ^_F_main 
