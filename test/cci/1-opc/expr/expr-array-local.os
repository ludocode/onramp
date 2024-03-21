#line manual
#line 1 "./expr/expr-array-local.i"
@_F_main 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  sub r0 0 r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -28 
  push r0 
  imw r0 5 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -24 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -24 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -24 
  push r0 
  imw r0 2 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 2 
  pop r1 
  stw r0 0 r1 
  imw r0 3 
  push r0 
  add r0 rfp -24 
  pop r1 
  shl r1 r1 2 
  add r0 r1 r0 
  push r0 
  imw r0 3 
  pop r1 
  stw r0 0 r1 
  imw r0 4 
  push r0 
  add r0 rfp -24 
  pop r1 
  shl r1 r1 2 
  add r0 r1 r0 
  push r0 
  imw r0 4 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  sub r0 0 r0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  add r0 rfp -28 
  push r0 
  imw r0 5 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 1 
  leave 
  ret 
:_Lx1 
  add r0 rfp -24 
  push r0 
  imw r0 0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 1 
  leave 
  ret 
:_Lx2 
  add r0 rfp -24 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
  imw r0 1 
  leave 
  ret 
:_Lx3 
  imw r0 2 
  push r0 
  add r0 rfp -24 
  pop r1 
  shl r1 r1 2 
  add r0 r1 r0 
  push r0 
  imw r0 2 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
  imw r0 1 
  leave 
  ret 
:_Lx4 
  add r0 rfp -24 
  push r0 
  imw r0 3 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 3 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx5 
  imw r0 1 
  leave 
  ret 
:_Lx5 
  imw r0 4 
  push r0 
  add r0 rfp -24 
  pop r1 
  shl r1 r1 2 
  add r0 r1 r0 
  push r0 
  imw r0 4 
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
  sub rsp rsp 28 
  jmp ^_F_main 
