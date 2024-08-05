@_F_main 
  add r0 rfp -8 
  push r0 
  imw r0 0 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 5 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -8 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 7 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -12 
  push r0 
  add r0 rfp -8 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -12 
  mov r1 r0 
  ldw r0 0 r0 
  push r0 
  push r1 
  imw r0 1 
  ldw r1 0 r1 
  shl r0 r0 2 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  pop r0 
  push r0 
  imw r0 5 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  add r0 rfp -12 
  mov r1 r0 
  ldw r0 0 r0 
  push r0 
  push r1 
  imw r0 1 
  ldw r1 0 r1 
  shl r0 r0 2 
  sub r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  pop r0 
  push r0 
  imw r0 7 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 2 
  leave 
  ret 
:_Lx1 
  add r0 rfp -12 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  shl r0 r0 2 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  push r0 
  imw r0 7 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 3 
  leave 
  ret 
:_Lx2 
  add r0 rfp -12 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  shl r0 r0 2 
  sub r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  push r0 
  imw r0 5 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
  imw r0 4 
  leave 
  ret 
:_Lx3 
  zero r0 
  leave 
  ret 
=main 
  enter 
  sub rsp rsp 12 
  jmp ^_F_main 
