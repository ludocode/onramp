@_F_main 
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
  imw r0 0 
  jz r0 &_Lx1 
  imw r0 0 
:_Lx1 
  bool r0 r0 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  imw r0 0 
  jz r0 &_Lx3 
  imw r0 1 
:_Lx3 
  bool r0 r0 
  jz r0 &_Lx2 
  imw r0 1 
  leave 
  ret 
:_Lx2 
  imw r0 1 
  jz r0 &_Lx5 
  imw r0 0 
:_Lx5 
  bool r0 r0 
  jz r0 &_Lx4 
  imw r0 1 
  leave 
  ret 
:_Lx4 
  imw r0 1 
  jz r0 &_Lx7 
  imw r0 1 
:_Lx7 
  bool r0 r0 
  isz r0 r0 
  jz r0 &_Lx6 
  imw r0 1 
  leave 
  ret 
:_Lx6 
  imw r0 0 
  jnz r0 &_Lx9 
  imw r0 0 
:_Lx9 
  bool r0 r0 
  jz r0 &_Lx8 
  imw r0 1 
  leave 
  ret 
:_Lx8 
  imw r0 0 
  jnz r0 &_LxB 
  imw r0 1 
:_LxB 
  bool r0 r0 
  isz r0 r0 
  jz r0 &_LxA 
  imw r0 1 
  leave 
  ret 
:_LxA 
  imw r0 1 
  jnz r0 &_LxD 
  imw r0 0 
:_LxD 
  bool r0 r0 
  isz r0 r0 
  jz r0 &_LxC 
  imw r0 1 
  leave 
  ret 
:_LxC 
  imw r0 1 
  jnz r0 &_LxF 
  imw r0 1 
:_LxF 
  bool r0 r0 
  isz r0 r0 
  jz r0 &_LxE 
  imw r0 1 
  leave 
  ret 
:_LxE 
  imw r0 0 
  jz r0 &_Lx11 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
:_Lx11 
  bool r0 r0 
  jz r0 &_Lx10 
  imw r0 1 
  leave 
  ret 
:_Lx10 
  add r0 rfp -4 
  ldw r0 0 r0 
  jz r0 &_Lx12 
  imw r0 1 
  leave 
  ret 
:_Lx12 
  imw r0 1 
  jz r0 &_Lx14 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
:_Lx14 
  bool r0 r0 
  isz r0 r0 
  jz r0 &_Lx13 
  imw r0 1 
  leave 
  ret 
:_Lx13 
  add r0 rfp -4 
  ldw r0 0 r0 
  isz r0 r0 
  jz r0 &_Lx15 
  imw r0 1 
  leave 
  ret 
:_Lx15 
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
  imw r0 1 
  jnz r0 &_Lx17 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
:_Lx17 
  bool r0 r0 
  isz r0 r0 
  jz r0 &_Lx16 
  imw r0 1 
  leave 
  ret 
:_Lx16 
  add r0 rfp -4 
  ldw r0 0 r0 
  jz r0 &_Lx18 
  imw r0 1 
  leave 
  ret 
:_Lx18 
  imw r0 0 
  jnz r0 &_Lx1A 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
:_Lx1A 
  bool r0 r0 
  isz r0 r0 
  jz r0 &_Lx19 
  imw r0 1 
  leave 
  ret 
:_Lx19 
  add r0 rfp -4 
  ldw r0 0 r0 
  isz r0 r0 
  jz r0 &_Lx1B 
  imw r0 1 
  leave 
  ret 
:_Lx1B 
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
  imw r0 1 
  jz r0 &_Lx1D 
  imw r0 0 
  jz r0 &_Lx1E 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
:_Lx1E 
  bool r0 r0 
:_Lx1D 
  bool r0 r0 
  jz r0 &_Lx1C 
  imw r0 1 
  leave 
  ret 
:_Lx1C 
  add r0 rfp -4 
  ldw r0 0 r0 
  jz r0 &_Lx1F 
  imw r0 1 
  leave 
  ret 
:_Lx1F 
  imw r0 0 
  jz r0 &_Lx21 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
  jz r0 &_Lx22 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
:_Lx22 
  bool r0 r0 
:_Lx21 
  bool r0 r0 
  jz r0 &_Lx20 
  imw r0 1 
  leave 
  ret 
:_Lx20 
  add r0 rfp -4 
  ldw r0 0 r0 
  jz r0 &_Lx23 
  imw r0 1 
  leave 
  ret 
:_Lx23 
  imw r0 0 
  jnz r0 &_Lx25 
  imw r0 1 
  jnz r0 &_Lx25 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
:_Lx25 
  bool r0 r0 
  isz r0 r0 
  jz r0 &_Lx24 
  imw r0 1 
  leave 
  ret 
:_Lx24 
  add r0 rfp -4 
  ldw r0 0 r0 
  jz r0 &_Lx26 
  imw r0 1 
  leave 
  ret 
:_Lx26 
  imw r0 1 
  jnz r0 &_Lx28 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
  jnz r0 &_Lx28 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
:_Lx28 
  bool r0 r0 
  isz r0 r0 
  jz r0 &_Lx27 
  imw r0 1 
  leave 
  ret 
:_Lx27 
  add r0 rfp -4 
  ldw r0 0 r0 
  jz r0 &_Lx29 
  imw r0 1 
  leave 
  ret 
:_Lx29 
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
