@_F_main 
  imw r0 100 
  push r0 
  imw r0 1 
  pop r1 
  add r0 r1 r0 
  push r0 
  imw r0 101 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  imw r0 100 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 1 
  add r0 r1 r0 
  push r0 
  imw r0 102 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 1 
  leave 
  ret 
:_Lx1 
  imw r0 100 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 104 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 1 
  leave 
  ret 
:_Lx2 
  imw r0 100 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 104 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
  imw r0 1 
  leave 
  ret 
:_Lx3 
  imw r0 100 
  push r0 
  imw r0 1 
  pop r1 
  add r0 r1 r0 
  push r0 
  imw r0 101 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
  imw r0 1 
  leave 
  ret 
:_Lx4 
  imw r0 100 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 1 
  add r0 r1 r0 
  push r0 
  imw r0 102 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx5 
  imw r0 1 
  leave 
  ret 
:_Lx5 
  imw r0 100 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 104 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx6 
  imw r0 1 
  leave 
  ret 
:_Lx6 
  imw r0 100 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 104 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx7 
  imw r0 1 
  leave 
  ret 
:_Lx7 
  imw r0 100 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 104 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx8 
  imw r0 1 
  leave 
  ret 
:_Lx8 
  imw r0 100 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 104 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx9 
  imw r0 1 
  leave 
  ret 
:_Lx9 
  imw r0 100 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 104 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxA 
  imw r0 1 
  leave 
  ret 
:_LxA 
  imw r0 100 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 104 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxB 
  imw r0 1 
  leave 
  ret 
:_LxB 
  imw r0 100 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 104 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxC 
  imw r0 1 
  leave 
  ret 
:_LxC 
  imw r0 100 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 104 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxD 
  imw r0 1 
  leave 
  ret 
:_LxD 
  imw r0 100 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 104 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxE 
  imw r0 1 
  leave 
  ret 
:_LxE 
  imw r0 100 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 104 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxF 
  imw r0 1 
  leave 
  ret 
:_LxF 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
