@A 
0 
@B 
1 
@C 
2 
@_F_main 
  add r0 rfp -4 
  push r0 
  imw r0 ^A 
  add r0 rpp r0 
  pop r1 
  ldw r0 0 r0 
  stw r0 0 r1 
  add r0 rfp -4 
  ldw r0 0 r0 
  add r1 rfp -8 
  stw r0 0 r1 
  jmp &_Lx1 
  jmp &_Lx2 
:_Lx1 
  imw r0 ^A 
  add r0 rpp r0 
  ldw r1 rfp -8 
  ldw r0 0 r0 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  jmp &_Lx3 
:_Lx2 
  jmp &_Lx0 
  jmp &_Lx4 
:_Lx3 
  imw r0 ^B 
  add r0 rpp r0 
  ldw r1 rfp -8 
  ldw r0 0 r0 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
  jmp &_Lx5 
:_Lx4 
  jmp &_Lx6 
:_Lx5 
  imw r0 ^C 
  add r0 rpp r0 
  ldw r1 rfp -8 
  ldw r0 0 r0 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx6 
  jmp &_Lx7 
:_Lx6 
  imw r0 1 
  leave 
  ret 
  jmp &_Lx0 
:_Lx7 
:_Lx0 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  sub rsp rsp 8 
  jmp ^_F_main 
