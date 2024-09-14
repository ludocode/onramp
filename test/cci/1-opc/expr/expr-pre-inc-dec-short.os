@_F_main 
  add r0 rfp -4 
  push r0 
  imw r0 30583 
  pop r1 
  trs r0 r0 
  sts r0 0 r1 
  add r0 rfp -8 
  push r0 
  imw r0 0 
  pop r1 
  trs r0 r0 
  sts r0 0 r1 
  add r0 rfp -12 
  push r0 
  imw r0 52428 
  pop r1 
  trs r0 r0 
  sts r0 0 r1 
  add r0 rfp -8 
  push r0 
  mov r1 r0 
  imw r0 1 
  lds r1 0 r1 
  add r0 r1 r0 
  pop r1 
  trs r0 r0 
  sts r0 0 r1 
  push r0 
  imw r0 1 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  add r0 rfp -8 
  push r0 
  mov r1 r0 
  imw r0 1 
  lds r1 0 r1 
  sub r0 r1 r0 
  pop r1 
  trs r0 r0 
  sts r0 0 r1 
  push r0 
  imw r0 0 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 2 
  leave 
  ret 
:_Lx1 
  add r0 rfp -8 
  push r0 
  mov r1 r0 
  imw r0 1 
  lds r1 0 r1 
  sub r0 r1 r0 
  pop r1 
  trs r0 r0 
  sts r0 0 r1 
  push r0 
  imw r0 65535 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 3 
  leave 
  ret 
:_Lx2 
  add r0 rfp -8 
  push r0 
  mov r1 r0 
  imw r0 1 
  lds r1 0 r1 
  add r0 r1 r0 
  pop r1 
  trs r0 r0 
  sts r0 0 r1 
  push r0 
  imw r0 0 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
  imw r0 4 
  leave 
  ret 
:_Lx3 
  add r0 rfp -4 
  push r0 
  imw r0 30583 
  pop r1 
  lds r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
  imw r0 5 
  leave 
  ret 
:_Lx4 
  add r0 rfp -12 
  push r0 
  imw r0 52428 
  pop r1 
  lds r1 0 r1 
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
  sub rsp rsp 12 
  jmp ^_F_main 
