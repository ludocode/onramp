=main
  enter
  imw r0 ^b
  ldw r0 rpp r0
  imw r1 0
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
  imw r0 ^b
  ldw r0 rpp r0
  imw r1 0
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
  imw r0 ^b
  ldw r0 rpp r0
  imw r1 0
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx4
  jmp &_Lx5
:_Lx4
  imw r0 1
  leave
  ret
  jmp &_Lx5
:_Lx5
  imw r0 4
  imw r1 ^b
  add r1 rpp r1
  stw r0 r1 0
  imw r0 ^b
  ldw r0 rpp r0
  imw r1 4
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx6
  jmp &_Lx7
:_Lx6
  imw r0 1
  leave
  ret
  jmp &_Lx7
:_Lx7
  imw r0 ^b
  ldw r0 rpp r0
  imw r1 4
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx8
  jmp &_Lx9
:_Lx8
  imw r0 1
  leave
  ret
  jmp &_Lx9
:_Lx9
  imw r0 ^b
  ldw r0 rpp r0
  imw r1 4
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxA
  jmp &_LxB
:_LxA
  imw r0 1
  leave
  ret
  jmp &_LxB
:_LxB
  zero r0
  leave
  ret
=b
  0
