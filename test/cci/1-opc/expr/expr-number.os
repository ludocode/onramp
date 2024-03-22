@_F_main 
  add r0 rfp -4 
  push r0 
  imw r0 0xBC614E 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -8 
  push r0 
  imw r0 0x5E30A78 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -12 
  push r0 
  imw r0 0x1234567 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -16 
  push r0 
  imw r0 0xABCDEF89 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -20 
  push r0 
  imw r0 493 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -24 
  push r0 
  imw r0 420 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -28 
  push r0 
  imw r0 0xBC614E 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -32 
  push r0 
  imw r0 0x5E30A78 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -36 
  push r0 
  imw r0 0x5E30A78 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -40 
  push r0 
  imw r0 0x5E30A78 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -44 
  push r0 
  imw r0 0x5E30A78 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -48 
  push r0 
  imw r0 0x1234567 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -52 
  push r0 
  imw r0 0xABCDEF89 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -56 
  push r0 
  imw r0 493 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -60 
  push r0 
  imw r0 420 
  pop r1 
  stw r0 0 r1 
  imw r0 -4 
  push r0 
  imw r0 0x3FFFFFFF 
  pop r1 
  divu r0 r1 r0 
  push r0 
  imw r0 4 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  sub rsp rsp 60 
  jmp ^_F_main 
