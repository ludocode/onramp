@_F_main 
  imw r0 0 
  push r0 
  imw r0 2 
  push r0 
  imw r0 1 
  pop r1 
  add r0 r1 r0 
  pop r1 
  cmpu r0 r1 r0 
  add r0 r0 1 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  imw r0 5 
  push r0 
  imw r0 3 
  pop r1 
  mul r0 r1 r0 
  push r0 
  imw r0 10 
  push r0 
  imw r0 2 
  pop r1 
  divs r0 r1 r0 
  pop r1 
  add r0 r1 r0 
  push r0 
  imw r0 30 
  push r0 
  imw r0 2 
  pop r1 
  mul r0 r1 r0 
  push r0 
  imw r0 3 
  pop r1 
  divs r0 r1 r0 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 1 
  leave 
  ret 
:_Lx1 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
