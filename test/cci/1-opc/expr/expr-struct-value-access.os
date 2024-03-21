=foo 
'00'00'00'00 '00'00'00'00
@_F_main 
  imw r0 ^foo 
  add r0 rpp r0 
  push r0 
  imw r0 2 
  pop r1 
  stw r0 0 r1 
  imw r0 ^foo 
  add r0 rpp r0 
  add r0 r0 4 
  push r0 
  imw r0 3 
  pop r1 
  stw r0 0 r1 
  imw r0 ^foo 
  add r0 rpp r0 
  push r0 
  imw r0 2 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  imw r0 ^foo 
  add r0 rpp r0 
  add r0 r0 4 
  push r0 
  imw r0 3 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 1 
  leave 
  ret 
:_Lx1 
  imw r0 ^foo 
  add r0 rpp r0 
  push r0 
  imw r0 2 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 1 
  leave 
  ret 
:_Lx2 
  imw r0 ^foo 
  add r0 rpp r0 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 3 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
  imw r0 1 
  leave 
  ret 
:_Lx3 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
