@X 
0 
@Y 
1 
@Z 
2 
@_F_main 
  imw r0 5 
  push r0 
  imw r0 ^X 
  add r0 rpp r0 
  pop r1 
  ldw r0 0 r0 
  and r0 r1 r0 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  imw r0 5 
  push r0 
  imw r0 ^Y 
  add r0 rpp r0 
  pop r1 
  ldw r0 0 r0 
  and r0 r1 r0 
  push r0 
  imw r0 1 
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
  imw r0 ^Z 
  add r0 rpp r0 
  pop r1 
  ldw r0 0 r0 
  and r0 r1 r0 
  jz r0 &_Lx2 
  imw r0 3 
  leave 
  ret 
:_Lx2 
  imw r0 5 
  push r0 
  imw r0 ^X 
  add r0 rpp r0 
  pop r1 
  ldw r0 0 r0 
  or r0 r1 r0 
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
  imw r0 ^Z 
  add r0 rpp r0 
  pop r1 
  ldw r0 0 r0 
  or r0 r1 r0 
  push r0 
  imw r0 7 
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
  imw r0 ^Y 
  add r0 rpp r0 
  pop r1 
  ldw r0 0 r0 
  add r0 r1 r0 
  push r0 
  imw r0 6 
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
  imw r0 ^Z 
  add r0 rpp r0 
  pop r1 
  ldw r0 0 r0 
  divs r0 r1 r0 
  push r0 
  imw r0 2 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx6 
  imw r0 7 
  leave 
  ret 
:_Lx6 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
