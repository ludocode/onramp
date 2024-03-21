@_F_main 
  add r0 rfp -4 
  push r0 
  imw r0 4 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -4 
  push r0 
  imw r0 2 
  pop r1 
  ldw r1 0 r1 
  mul r0 r1 r0 
  add r1 rfp -8 
  stw r0 0 r1 
  jmp &_Lx1 
  jmp &_Lx2 
:_Lx1 
  imw r0 10 
  ldw r1 rfp -8 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  jmp &_Lx3 
:_Lx2 
  imw r0 1 
  leave 
  ret 
:_Lx4 
  imw r0 1 
  leave 
  ret 
  jmp &_Lx5 
:_Lx3 
  imw r0 19 
  ldw r1 rfp -8 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx5 
  jmp &_Lx6 
:_Lx5 
  imw r0 1 
  leave 
  ret 
  jmp &_Lx7 
:_Lx6 
  imw r0 8 
  ldw r1 rfp -8 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx7 
  jmp &_Lx8 
:_Lx7 
  jmp &_Lx0 
  imw r0 1 
  leave 
  ret 
  jmp &_Lx0 
:_Lx8 
  jmp &_Lx4 
:_Lx0 
  mov r0 "x"
  add r1 rfp -12 
  stw r0 0 r1 
  jmp &_LxA 
  jmp &_LxB 
:_LxA 
  imw r0 97 
  ldw r1 rfp -12 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxB 
  jmp &_LxC 
:_LxB 
  imw r0 1 
  leave 
  ret 
  jmp &_LxD 
:_LxC 
  imw r0 98 
  ldw r1 rfp -12 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxD 
  jmp &_LxE 
:_LxD 
  imw r0 1 
  leave 
  ret 
  jmp &_LxF 
:_LxE 
  imw r0 99 
  ldw r1 rfp -12 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxF 
  jmp &_Lx10 
:_LxF 
  imw r0 1 
  leave 
  ret 
  jmp &_Lx9 
:_Lx10 
:_Lx9 
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
  mov r0 "b"
  add r1 rfp -16 
  stw r0 0 r1 
  jmp &_Lx12 
  jmp &_Lx13 
:_Lx12 
  imw r0 97 
  ldw r1 rfp -16 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx13 
  jmp &_Lx14 
:_Lx13 
  imw r0 1 
  leave 
  ret 
  jmp &_Lx15 
:_Lx14 
  imw r0 98 
  ldw r1 rfp -16 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx15 
  jmp &_Lx16 
:_Lx15 
  add r0 rfp -4 
  push r0 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  jmp &_Lx17 
:_Lx16 
  imw r0 99 
  ldw r1 rfp -16 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx17 
  jmp &_Lx18 
:_Lx17 
  add r0 rfp -4 
  push r0 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
:_Lx19 
  add r0 rfp -4 
  push r0 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  jmp &_Lx11 
:_Lx18 
  jmp &_Lx19 
:_Lx11 
  add r0 rfp -4 
  push r0 
  imw r0 3 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1A 
  imw r0 1 
  leave 
  ret 
:_Lx1A 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
  imw r0 0 
  add r1 rfp -20 
  stw r0 0 r1 
  jmp &_Lx1C 
  jmp &_Lx1D 
:_Lx1C 
  imw r0 0 
  ldw r1 rfp -20 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1D 
  jmp &_Lx1E 
:_Lx1D 
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
  jmp &_Lx1B 
:_Lx1E 
:_Lx1B 
  add r0 rfp -4 
  ldw r0 0 r0 
  jz r0 &_Lx1F 
  imw r0 1 
  leave 
  ret 
:_Lx1F 
  imw r0 0 
  add r1 rfp -24 
  stw r0 0 r1 
  jmp &_Lx21 
  jmp &_Lx22 
:_Lx21 
  imw r0 1 
  ldw r1 rfp -24 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx22 
  jmp &_Lx23 
:_Lx22 
  imw r0 1 
  leave 
  ret 
  jmp &_Lx20 
:_Lx23 
:_Lx20 
  imw r0 0 
  add r1 rfp -28 
  stw r0 0 r1 
  jmp &_Lx25 
  imw r0 0 
  add r1 rfp -32 
  stw r0 0 r1 
  jmp &_Lx27 
:_Lx28 
  imw r0 1 
  leave 
  ret 
  jmp &_Lx26 
:_Lx27 
  jmp &_Lx28 
:_Lx26 
  jmp &_Lx24 
:_Lx25 
:_Lx24 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  sub rsp rsp 32 
  jmp ^_F_main 
