=main
  enter
  imw r0 ^x
  ldw r0 rpp r0
  mov r1 0
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  mov r0 1
  leave
  ret
  jmp &_Lx1
:_Lx1
  imw r0 ^y
  ldw r0 rpp r0
  mov r1 0
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  mov r0 2
  leave
  ret
  jmp &_Lx3
:_Lx3
  zero r0
  leave
  ret
=x
  0
@y
  0
