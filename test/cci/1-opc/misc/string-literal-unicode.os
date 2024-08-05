@_F_main 
  add r0 rfp -4 
  push r0 
  imw r0 ^_Sx0 
  add r0 rpp r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  push r0 
  imw r0 226 
  sxb r0 r0 
  pop r1 
  sxb r0 r0 
  ldb r1 0 r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  push r0 
  imw r0 148 
  sxb r0 r0 
  pop r1 
  sxb r0 r0 
  ldb r1 0 r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 2 
  leave 
  ret 
:_Lx1 
  add r0 rfp -4 
  push r0 
  imw r0 2 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  push r0 
  imw r0 156 
  sxb r0 r0 
  pop r1 
  sxb r0 r0 
  ldb r1 0 r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 3 
  leave 
  ret 
:_Lx2 
  add r0 rfp -4 
  push r0 
  imw r0 3 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  push r0 
  imw r0 226 
  sxb r0 r0 
  pop r1 
  sxb r0 r0 
  ldb r1 0 r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
  imw r0 4 
  leave 
  ret 
:_Lx3 
  add r0 rfp -4 
  push r0 
  imw r0 4 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  push r0 
  imw r0 148 
  sxb r0 r0 
  pop r1 
  sxb r0 r0 
  ldb r1 0 r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
  imw r0 5 
  leave 
  ret 
:_Lx4 
  add r0 rfp -4 
  push r0 
  imw r0 5 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  push r0 
  imw r0 148 
  sxb r0 r0 
  pop r1 
  sxb r0 r0 
  ldb r1 0 r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx5 
  imw r0 6 
  leave 
  ret 
:_Lx5 
  zero r0 
  leave 
  ret 
=main 
  enter 
  sub rsp rsp 4 
  jmp ^_F_main 
@_Sx0 
'E2'94'9C'E2'94'94'00
