@_F_main 
  add r0 rfp -4 
  push r0 
  imw r0 3 
  sub r0 0 r0 
  pop r1 
  sxb r0 r0 
  stb r0 0 r1 
  add r0 rfp -8 
  push r0 
  imw r0 0x333 
  sub r0 0 r0 
  pop r1 
  sxs r0 r0 
  sts r0 0 r1 
  add r0 rfp -12 
  push r0 
  imw r0 0x33333 
  sub r0 0 r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -16 
  push r0 
  imw r0 3 
  pop r1 
  trb r0 r0 
  stb r0 0 r1 
  add r0 rfp -20 
  push r0 
  imw r0 0x333 
  pop r1 
  trs r0 r0 
  sts r0 0 r1 
  add r0 rfp -24 
  push r0 
  imw r0 0x33333 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -12 
  ldw r0 0 r0 
  sxb r0 r0 
  push r0 
  imw r0 0x33 
  sub r0 0 r0 
  pop r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  add r0 rfp -24 
  ldw r0 0 r0 
  sxb r0 r0 
  push r0 
  imw r0 0x33 
  pop r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 1 
  leave 
  ret 
:_Lx1 
  add r0 rfp -8 
  lds r0 0 r0 
  sxb r0 r0 
  push r0 
  imw r0 0x33 
  sub r0 0 r0 
  pop r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 1 
  leave 
  ret 
:_Lx2 
  add r0 rfp -20 
  lds r0 0 r0 
  sxb r0 r0 
  push r0 
  imw r0 0x33 
  pop r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
  imw r0 1 
  leave 
  ret 
:_Lx3 
  add r0 rfp -4 
  ldb r0 0 r0 
  push r0 
  imw r0 3 
  sub r0 0 r0 
  pop r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
  imw r0 1 
  leave 
  ret 
:_Lx4 
  add r0 rfp -16 
  ldb r0 0 r0 
  sxb r0 r0 
  push r0 
  imw r0 3 
  pop r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx5 
  imw r0 1 
  leave 
  ret 
:_Lx5 
  add r0 rfp -12 
  ldw r0 0 r0 
  trb r0 r0 
  push r0 
  imw r0 0xCD 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx6 
  imw r0 1 
  leave 
  ret 
:_Lx6 
  add r0 rfp -24 
  ldw r0 0 r0 
  trb r0 r0 
  push r0 
  imw r0 0x33 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx7 
  imw r0 1 
  leave 
  ret 
:_Lx7 
  add r0 rfp -8 
  lds r0 0 r0 
  trb r0 r0 
  push r0 
  imw r0 0xCD 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx8 
  imw r0 1 
  leave 
  ret 
:_Lx8 
  add r0 rfp -20 
  lds r0 0 r0 
  trb r0 r0 
  push r0 
  imw r0 0x33 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx9 
  imw r0 1 
  leave 
  ret 
:_Lx9 
  add r0 rfp -4 
  ldb r0 0 r0 
  trb r0 r0 
  push r0 
  imw r0 0xFD 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxA 
  imw r0 1 
  leave 
  ret 
:_LxA 
  add r0 rfp -16 
  ldb r0 0 r0 
  push r0 
  imw r0 3 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxB 
  imw r0 1 
  leave 
  ret 
:_LxB 
  add r0 rfp -12 
  ldw r0 0 r0 
  sxs r0 r0 
  push r0 
  imw r0 0x3333 
  sub r0 0 r0 
  pop r1 
  sxs r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxC 
  imw r0 1 
  leave 
  ret 
:_LxC 
  add r0 rfp -24 
  ldw r0 0 r0 
  sxs r0 r0 
  push r0 
  imw r0 0x3333 
  pop r1 
  sxs r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxD 
  imw r0 1 
  leave 
  ret 
:_LxD 
  add r0 rfp -8 
  lds r0 0 r0 
  push r0 
  imw r0 0x333 
  sub r0 0 r0 
  pop r1 
  sxs r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxE 
  imw r0 1 
  leave 
  ret 
:_LxE 
  add r0 rfp -20 
  lds r0 0 r0 
  sxs r0 r0 
  push r0 
  imw r0 0x333 
  pop r1 
  sxs r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxF 
  imw r0 1 
  leave 
  ret 
:_LxF 
  add r0 rfp -4 
  ldb r0 0 r0 
  sxb r0 r0 
  push r0 
  imw r0 3 
  sub r0 0 r0 
  pop r1 
  sxs r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx10 
  imw r0 1 
  leave 
  ret 
:_Lx10 
  add r0 rfp -16 
  ldb r0 0 r0 
  push r0 
  imw r0 3 
  pop r1 
  sxs r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx11 
  imw r0 1 
  leave 
  ret 
:_Lx11 
  add r0 rfp -12 
  ldw r0 0 r0 
  trs r0 r0 
  push r0 
  imw r0 0xCCCD 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx12 
  imw r0 1 
  leave 
  ret 
:_Lx12 
  add r0 rfp -24 
  ldw r0 0 r0 
  trs r0 r0 
  push r0 
  imw r0 0x3333 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx13 
  imw r0 1 
  leave 
  ret 
:_Lx13 
  add r0 rfp -8 
  lds r0 0 r0 
  trs r0 r0 
  push r0 
  imw r0 0xFCCD 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx14 
  imw r0 1 
  leave 
  ret 
:_Lx14 
  add r0 rfp -20 
  lds r0 0 r0 
  push r0 
  imw r0 0x333 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx15 
  imw r0 1 
  leave 
  ret 
:_Lx15 
  add r0 rfp -4 
  ldb r0 0 r0 
  sxb r0 r0 
  trs r0 r0 
  push r0 
  imw r0 0xFFFD 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx16 
  imw r0 1 
  leave 
  ret 
:_Lx16 
  add r0 rfp -16 
  ldb r0 0 r0 
  push r0 
  imw r0 3 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx17 
  imw r0 1 
  leave 
  ret 
:_Lx17 
  add r0 rfp -12 
  ldw r0 0 r0 
  push r0 
  imw r0 0x33333 
  sub r0 0 r0 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx18 
  imw r0 1 
  leave 
  ret 
:_Lx18 
  add r0 rfp -24 
  ldw r0 0 r0 
  push r0 
  imw r0 0x33333 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx19 
  imw r0 1 
  leave 
  ret 
:_Lx19 
  add r0 rfp -8 
  lds r0 0 r0 
  sxs r0 r0 
  push r0 
  imw r0 0x333 
  sub r0 0 r0 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1A 
  imw r0 1 
  leave 
  ret 
:_Lx1A 
  add r0 rfp -20 
  lds r0 0 r0 
  push r0 
  imw r0 0x333 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1B 
  imw r0 1 
  leave 
  ret 
:_Lx1B 
  add r0 rfp -4 
  ldb r0 0 r0 
  sxb r0 r0 
  push r0 
  imw r0 3 
  sub r0 0 r0 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1C 
  imw r0 1 
  leave 
  ret 
:_Lx1C 
  add r0 rfp -16 
  ldb r0 0 r0 
  push r0 
  imw r0 3 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1D 
  imw r0 1 
  leave 
  ret 
:_Lx1D 
  add r0 rfp -12 
  ldw r0 0 r0 
  push r0 
  imw r0 0xFFFCCCCD 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1E 
  imw r0 1 
  leave 
  ret 
:_Lx1E 
  add r0 rfp -24 
  ldw r0 0 r0 
  push r0 
  imw r0 0x33333 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1F 
  imw r0 1 
  leave 
  ret 
:_Lx1F 
  add r0 rfp -8 
  lds r0 0 r0 
  sxs r0 r0 
  push r0 
  imw r0 0xFFFFFCCD 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx20 
  imw r0 1 
  leave 
  ret 
:_Lx20 
  add r0 rfp -20 
  lds r0 0 r0 
  push r0 
  imw r0 0x333 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx21 
  imw r0 1 
  leave 
  ret 
:_Lx21 
  add r0 rfp -4 
  ldb r0 0 r0 
  sxb r0 r0 
  push r0 
  imw r0 0xFFFFFFFD 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx22 
  imw r0 1 
  leave 
  ret 
:_Lx22 
  add r0 rfp -16 
  ldb r0 0 r0 
  push r0 
  imw r0 3 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx23 
  imw r0 1 
  leave 
  ret 
:_Lx23 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  sub rsp rsp 24 
  jmp ^_F_main 
