=main
  enter
  sub rsp rsp 8
  imw r0 1
  imw r1 ^b
  add r1 rpp r1
  stw r0 r1 0
  imw r0 ^b
  add r0 rpp r0
  ldw r0 0 r0
  imw r1 1
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
  imw r0 1
  add r1 rfp -8
  stw r0 r1 0
  imw r0 2
  add r1 rfp -8
  add r1 r1 4
  stw r0 r1 0
  imw r0 3
  add r1 rfp -8
  stw r0 r1 0
  imw r0 4
  add r1 rfp -8
  stw r0 r1 0
  add r0 rfp -8
  ldw r0 0 r0
  imw r1 4
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  imw r0 1
  leave
  ret
  jmp &_Lx3
:_Lx3
  add r0 rfp -8
  add r0 r0 4
  ldw r0 0 r0
  imw r1 2
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx4
  jmp &_Lx5
:_Lx4
  imw r0 2
  leave
  ret
  jmp &_Lx5
:_Lx5
  add r0 rfp -8
  ldw r0 0 r0
  imw r1 4
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx6
  jmp &_Lx7
:_Lx6
  imw r0 3
  leave
  ret
  jmp &_Lx7
:_Lx7
  add r0 rfp -8
  ldw r0 0 r0
  imw r1 4
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx8
  jmp &_Lx9
:_Lx8
  imw r0 4
  leave
  ret
  jmp &_Lx9
:_Lx9
  imw r0 0
  leave
  ret
=b
  0
