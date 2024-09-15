=main
  enter
  sub rsp rsp 12
  add r0 rfp -12
  imw r1 1
  stw r1 r0 0
  imw r1 2
  stw r1 r0 4
  imw r1 3
  stw r1 r0 8
  imw r0 12
  imw r1 3
  imw r2 4
  mul r1 r1 r2
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  imw r0 1
  leave
  ret
  jmp &_Lx1
:_Lx1
  imw r0 6
  add r1 rfp -12
  imw r2 0
  shl r2 r2 2
  add r1 r1 r2
  ldw r1 0 r1
  add r2 rfp -12
  imw r3 1
  shl r3 r3 2
  add r2 r2 r3
  ldw r2 0 r2
  add r1 r1 r2
  add r2 rfp -12
  imw r3 2
  shl r3 r3 2
  add r2 r2 r3
  ldw r2 0 r2
  add r1 r1 r2
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  imw r0 2
  leave
  ret
  jmp &_Lx3
:_Lx3
  zero r0
  leave
  ret
