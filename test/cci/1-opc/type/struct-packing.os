@_F_main 
  imw r0 4 
  push r0 
  add r0 rfp -72 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 4 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  imw r0 1 
  push r0 
  add r0 rfp -72 
  add r0 r0 8 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 9 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 2 
  leave 
  ret 
:_Lx1 
  imw r0 3 
  push r0 
  add r0 rfp -72 
  add r0 r0 9 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 12 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 3 
  leave 
  ret 
:_Lx2 
  imw r0 1 
  push r0 
  add r0 rfp -72 
  add r0 r0 16 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 17 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
  imw r0 4 
  leave 
  ret 
:_Lx3 
  imw r0 1 
  push r0 
  add r0 rfp -72 
  add r0 r0 17 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 18 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
  imw r0 5 
  leave 
  ret 
:_Lx4 
  imw r0 2 
  push r0 
  add r0 rfp -72 
  add r0 r0 18 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 20 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx5 
  imw r0 6 
  leave 
  ret 
:_Lx5 
  imw r0 1 
  push r0 
  add r0 rfp -72 
  add r0 r0 24 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 25 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx6 
  imw r0 7 
  leave 
  ret 
:_Lx6 
  imw r0 1 
  push r0 
  add r0 rfp -72 
  add r0 r0 25 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 26 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx7 
  imw r0 8 
  leave 
  ret 
:_Lx7 
  imw r0 1 
  push r0 
  add r0 rfp -72 
  add r0 r0 26 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 27 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx8 
  imw r0 9 
  leave 
  ret 
:_Lx8 
  imw r0 1 
  push r0 
  add r0 rfp -72 
  add r0 r0 27 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 28 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx9 
  imw r0 10 
  leave 
  ret 
:_Lx9 
  imw r0 4 
  push r0 
  add r0 rfp -72 
  add r0 r0 32 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 36 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxA 
  imw r0 11 
  leave 
  ret 
:_LxA 
  imw r0 2 
  push r0 
  add r0 rfp -72 
  add r0 r0 40 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 42 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxB 
  imw r0 12 
  leave 
  ret 
:_LxB 
  imw r0 2 
  push r0 
  add r0 rfp -72 
  add r0 r0 42 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 44 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxC 
  imw r0 13 
  leave 
  ret 
:_LxC 
  imw r0 2 
  push r0 
  add r0 rfp -72 
  add r0 r0 48 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 50 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxD 
  imw r0 14 
  leave 
  ret 
:_LxD 
  imw r0 2 
  push r0 
  add r0 rfp -72 
  add r0 r0 50 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 52 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxE 
  imw r0 15 
  leave 
  ret 
:_LxE 
  imw r0 1 
  push r0 
  add r0 rfp -72 
  add r0 r0 56 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 57 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxF 
  imw r0 16 
  leave 
  ret 
:_LxF 
  imw r0 1 
  push r0 
  add r0 rfp -72 
  add r0 r0 57 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 58 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx10 
  imw r0 17 
  leave 
  ret 
:_Lx10 
  imw r0 2 
  push r0 
  add r0 rfp -72 
  add r0 r0 58 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 60 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx11 
  imw r0 18 
  leave 
  ret 
:_Lx11 
  imw r0 4 
  push r0 
  add r0 rfp -72 
  add r0 r0 64 
  pop r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -72 
  add r0 r0 68 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx12 
  imw r0 19 
  leave 
  ret 
:_Lx12 
  add r0 rfp -72 
  add r0 r0 8 
  push r0 
  imw r0 1 
  pop r1 
  sxb r0 r0 
  stb r0 0 r1 
  add r0 rfp -72 
  add r0 r0 9 
  push r0 
  imw r0 2 
  pop r1 
  sxb r0 r0 
  stb r0 0 r1 
  add r0 rfp -72 
  add r0 r0 12 
  push r0 
  imw r0 3 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -72 
  add r0 r0 8 
  push r0 
  imw r0 1 
  pop r1 
  ldb r1 0 r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx13 
  imw r0 20 
  leave 
  ret 
:_Lx13 
  add r0 rfp -72 
  add r0 r0 8 
  push r0 
  imw r0 1 
  pop r1 
  add r0 r1 r0 
  push r0 
  imw r0 2 
  pop r1 
  ldb r1 0 r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx14 
  imw r0 21 
  leave 
  ret 
:_Lx14 
  add r0 rfp -72 
  add r0 r0 8 
  push r0 
  imw r0 4 
  pop r1 
  add r0 r1 r0 
  push r0 
  imw r0 3 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx15 
  imw r0 22 
  leave 
  ret 
:_Lx15 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  sub rsp rsp 72 
  jmp ^_F_main 
