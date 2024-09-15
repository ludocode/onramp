=main
  enter
  sub rsp rsp 12
  add r0 rfp -4
  imw r1 4
  stw r1 r0 0
  ldw r0 rfp -4
  imw r1 4
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
  add r0 rfp -12
  imw r1 7
  stw r1 r0 0
  imw r1 9
  stw r1 r0 4
  add r0 rfp -12
  ldw r0 0 r0
  imw r1 7
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
  add r0 rfp -12
  add r0 r0 4
  ldw r0 0 r0
  imw r1 9
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx4
  jmp &_Lx5
:_Lx4
  imw r0 3
  leave
  ret
  jmp &_Lx5
:_Lx5
  zero r0
  leave
  ret
