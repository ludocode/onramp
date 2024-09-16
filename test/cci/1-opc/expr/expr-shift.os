@_F_main 
  imw r0 5 
  push r0 
  imw r0 2 
  pop r1 
  shru r0 r1 r0 
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
  imw r0 5 
  push r0 
  imw r0 0 
  pop r1 
  shru r0 r1 r0 
  push r0 
  imw r0 5 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 2 
  leave 
  ret 
:_Lx1 
  imw r0 5 
  push r0 
  imw r0 2 
  pop r1 
  shrs r0 r1 r0 
  push r0 
  imw r0 1 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 3 
  leave 
  ret 
:_Lx2 
  imw r0 5 
  push r0 
  imw r0 0 
  pop r1 
  shrs r0 r1 r0 
  push r0 
  imw r0 5 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
  imw r0 4 
  leave 
  ret 
:_Lx3 
  imw r0 5 
  push r0 
  imw r0 2 
  pop r1 
  shl r0 r1 r0 
  push r0 
  imw r0 20 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
  imw r0 5 
  leave 
  ret 
:_Lx4 
  imw r0 5 
  push r0 
  imw r0 0 
  pop r1 
  shl r0 r1 r0 
  push r0 
  imw r0 5 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx5 
  imw r0 6 
  leave 
  ret 
:_Lx5 
  imw r0 5 
  push r0 
  imw r0 2 
  pop r1 
  shl r0 r1 r0 
  push r0 
  imw r0 20 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx6 
  imw r0 7 
  leave 
  ret 
:_Lx6 
  imw r0 5 
  push r0 
  imw r0 0 
  pop r1 
  shl r0 r1 r0 
  push r0 
  imw r0 5 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx7 
  imw r0 8 
  leave 
  ret 
:_Lx7 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
