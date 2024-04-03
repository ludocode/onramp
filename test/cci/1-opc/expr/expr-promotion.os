@_F_main 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  trs r0 r0 
  sts r0 0 r1 
  add r0 rfp -8 
  push r0 
  imw r0 2 
  sub r0 0 r0 
  pop r1 
  sxs r0 r0 
  sts r0 0 r1 
  add r0 rfp -4 
  push r0 
  add r0 rfp -8 
  pop r1 
  lds r0 0 r0 
  sxs r0 r0 
  lds r1 0 r1 
  add r0 r1 r0 
  push r0 
  imw r0 0 
  pop r1 
  cmps r0 r1 r0 
  cmpu r0 r0 -1 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  add r0 rfp -4 
  push r0 
  add r0 rfp -8 
  pop r1 
  lds r0 0 r0 
  sxs r0 r0 
  lds r1 0 r1 
  add r0 r1 r0 
  push r0 
  imw r0 1 
  sub r0 0 r0 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 1 
  leave 
  ret 
:_Lx1 
  add r0 rfp -12 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -16 
  push r0 
  imw r0 2 
  sub r0 0 r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -12 
  push r0 
  add r0 rfp -16 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  add r0 r1 r0 
  push r0 
  imw r0 0 
  pop r1 
  cmpu r0 r1 r0 
  cmpu r0 r0 1 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 2 
  leave 
  ret 
:_Lx2 
  add r0 rfp -12 
  push r0 
  add r0 rfp -16 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  add r0 r1 r0 
  push r0 
  imw r0 1 
  sub r0 0 r0 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
  imw r0 2 
  leave 
  ret 
:_Lx3 
  add r0 rfp -12 
  push r0 
  add r0 rfp -16 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  add r0 r1 r0 
  push r0 
  imw r0 -1 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
  imw r0 2 
  leave 
  ret 
:_Lx4 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  sub rsp rsp 16 
  jmp ^_F_main 
