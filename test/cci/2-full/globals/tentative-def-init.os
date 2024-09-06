=x
  0
@{50_Ix0_x
  enter
  imw r0 ^x
  add r0 rpp r0
  imw r1 2
  stw r1 r0 0
  leave
  ret
=main
  enter
  imw r0 ^x
  ldw r0 rpp r0
  imw r1 2
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx1
  jmp &_Lx2
:_Lx1
  imw r0 1
  leave
  ret
  jmp &_Lx2
:_Lx2
  zero r0
  leave
  ret
