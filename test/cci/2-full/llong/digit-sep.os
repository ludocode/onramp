=main
  enter
  sub rsp rsp 16
  add r0 rfp -4
  imw r1 493
  stw r1 r0 0
  ldw r0 rfp -4
  imw r1 493
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
  add r0 rfp -8
  imw r1 15
  stw r1 r0 0
  ldw r0 rfp -8
  imw r1 15
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
  imw r1 65535
  stw r1 r0 0
  ldw r0 rfp -12
  imw r1 65535
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
  add r0 rfp -16
  imw r1 1000000000
  stw r1 r0 0
  ldw r0 rfp -16
  imw r1 1000000000
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx6
  jmp &_Lx7
:_Lx6
  imw r0 4
  leave
  ret
  jmp &_Lx7
:_Lx7
  zero r0
  leave
  ret
