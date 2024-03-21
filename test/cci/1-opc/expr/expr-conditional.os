@_F_main 
  add r0 rfp -4 
  push r0 
  imw r0 5 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -4 
  push r0 
  imw r0 5 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 4 
  push r0 
  imw r0 3 
  pop r1 
  sub r0 r1 r0 
  jmp &_Lx2 
:_Lx1 
  imw r0 6 
  push r0 
  imw r0 6 
  pop r1 
  sub r0 r1 r0 
:_Lx2 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  add r0 rfp -4 
  push r0 
  imw r0 5 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
  imw r0 4 
  push r0 
  imw r0 4 
  pop r1 
  sub r0 r1 r0 
  jmp &_Lx5 
:_Lx4 
  imw r0 1 
  push r0 
  imw r0 0 
  pop r1 
  sub r0 r1 r0 
:_Lx5 
  jz r0 &_Lx3 
  imw r0 0 
  leave 
  ret 
:_Lx3 
  imw r0 1 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  sub rsp rsp 4 
  jmp ^_F_main 
