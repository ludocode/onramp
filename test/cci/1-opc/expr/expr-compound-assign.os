@_F_main 
  add r0 rfp -4 
  push r0 
  imw r0 4 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -4 
  push r0 
  imw r0 3 
  ldw r1 rsp 0 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -4 
  push r0 
  imw r0 7 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  add r0 rfp -4 
  push r0 
  imw r0 5 
  ldw r1 rsp 0 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  push r0 
  imw r0 12 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 1 
  leave 
  ret 
:_Lx1 
  add r0 rfp -4 
  push r0 
  imw r0 12 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 1 
  leave 
  ret 
:_Lx2 
  add r0 rfp -4 
  push r0 
  imw r0 2 
  ldw r1 rsp 0 
  ldw r1 0 r1 
  shl r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -4 
  push r0 
  imw r0 48 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
  imw r0 1 
  leave 
  ret 
:_Lx3 
  add r0 rfp -4 
  push r0 
  imw r0 17 
  ldw r1 rsp 0 
  ldw r1 0 r1 
  sub r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -4 
  push r0 
  imw r0 31 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
  imw r0 1 
  leave 
  ret 
:_Lx4 
  add r0 rfp -4 
  push r0 
  imw r0 85 
  ldw r1 rsp 0 
  ldw r1 0 r1 
  and r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -4 
  push r0 
  imw r0 21 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx5 
  imw r0 1 
  leave 
  ret 
:_Lx5 
  add r0 rfp -4 
  push r0 
  imw r0 3 
  ldw r1 rsp 0 
  ldw r1 0 r1 
  divs r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -4 
  push r0 
  imw r0 7 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx6 
  imw r0 1 
  leave 
  ret 
:_Lx6 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  sub rsp rsp 4 
  jmp ^_F_main 
