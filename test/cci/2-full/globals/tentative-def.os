=main
  enter
  imw r1 ^x
  ldw r1 rpp r1
  mov r2 0
  sub r1 r1 r2
  cmpu r1 r1 0
  and r1 r1 1
  jnz r1 &_Lx0
  jmp &_Lx1
:_Lx0
  mov r0 1
  leave
  ret
  jmp &_Lx1
:_Lx1
  zero r0
  leave
  ret
=x
  0
