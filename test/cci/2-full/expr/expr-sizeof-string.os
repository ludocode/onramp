@_Sx0
  "Hello"
  '00
@_Sx1
  "Hello"
  " "
  "world"
  "!"
  '00
=main
  enter
  mov r0 6
  mov r1 6
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
  mov r0 13
  mov r1 13
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
