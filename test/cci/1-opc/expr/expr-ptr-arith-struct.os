@_F_main 
  add r0 rfp -28 
  push r0 
  add r0 rfp -24 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -32 
  push r0 
  add r0 rfp -24 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 3 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -36 
  push r0 
  add r0 rfp -24 
  push r0 
  imw r0 2 
  pop r1 
  shl r0 r0 3 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -32 
  push r0 
  add r0 rfp -28 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  sub r0 r1 r0 
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
  add r0 rfp -36 
  push r0 
  add r0 rfp -32 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  sub r0 r1 r0 
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
  add r0 rfp -24 
  push r0 
  imw r0 2 
  pop r1 
  shl r0 r0 3 
  add r0 r1 r0 
  push r0 
  add r0 rfp -24 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 3 
  add r0 r1 r0 
  pop r1 
  sub r0 r1 r0 
  shrs r0 r0 3 
  push r0 
  imw r0 1 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 3 
  leave 
  ret 
:_Lx2 
  imw r0 -1636 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  imw r9 800 
  mul r0 r0 r9 
  add r0 r1 r0 
  push r0 
  imw r0 -1636 
  add r0 rfp r0 
  pop r1 
  sub r0 r1 r0 
  push r0 
  imw r0 800 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
  imw r0 4 
  leave 
  ret 
:_Lx3 
  imw r0 -1636 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  imw r9 800 
  mul r0 r0 r9 
  add r0 r1 r0 
  push r0 
  imw r0 -1636 
  add r0 rfp r0 
  pop r1 
  sub r0 r1 r0 
  imw r9 800 
  divs r0 r0 r9 
  push r0 
  imw r0 1 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
  imw r0 5 
  leave 
  ret 
:_Lx4 
  zero r0 
  leave 
  ret 
=main 
  enter 
  imw r9 1636 
  sub rsp rsp r9 
  jmp ^_F_main 
