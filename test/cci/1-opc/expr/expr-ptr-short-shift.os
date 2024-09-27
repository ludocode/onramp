@_F_main 
  add r0 rfp -4 
  push r0 
  imw r0 4 
  pop r1 
  sxs r0 r0 
  sts r0 0 r1 
  add r0 rfp -8 
  push r0 
  imw r0 ^_Sx0 
  add r0 rpp r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -8 
  ldw r0 0 r0 
  push r0 
  mov r0 "a"
  pop r1 
  ldb r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  add r0 rfp -8 
  push r0 
  add r0 rfp -4 
  ldw r1 rsp 0 
  lds r0 0 r0 
  sxs r0 r0 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -8 
  ldw r0 0 r0 
  push r0 
  mov r0 "e"
  pop r1 
  ldb r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 2 
  leave 
  ret 
:_Lx1 
  add r0 rfp -8 
  push r0 
  add r0 rfp -4 
  push r0 
  imw r0 5 
  pop r1 
  sxs r0 r0 
  sts r0 0 r1 
  ldw r1 rsp 0 
  sxs r0 r0 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -8 
  push r0 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  sxs r0 r0 
  sts r0 0 r1 
  ldw r1 rsp 0 
  sxs r0 r0 
  ldw r1 0 r1 
  sub r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -8 
  ldw r0 0 r0 
  push r0 
  imw r0 0 
  pop r1 
  ldb r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 3 
  leave 
  ret 
:_Lx2 
  add r0 rfp -4 
  push r0 
  imw r0 5 
  pop r1 
  sxs r0 r0 
  sts r0 0 r1 
  add r0 rfp -8 
  push r0 
  add r0 rfp -4 
  ldw r1 rsp 0 
  lds r0 0 r0 
  sxs r0 r0 
  ldw r1 0 r1 
  sub r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -8 
  ldw r0 0 r0 
  push r0 
  mov r0 "d"
  pop r1 
  ldb r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
  imw r0 4 
  leave 
  ret 
:_Lx3 
  add r0 rfp -8 
  push r0 
  add r0 rfp -4 
  push r0 
  imw r0 2 
  pop r1 
  lds r1 0 r1 
  sxs r1 r1 
  divs r0 r1 r0 
  ldw r1 rsp 0 
  ldw r1 0 r1 
  sub r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -8 
  ldw r0 0 r0 
  push r0 
  mov r0 "b"
  pop r1 
  ldb r1 0 r1 
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
  sub rsp rsp 8 
  jmp ^_F_main 
@_Sx0 
"abcdefgh"'00
