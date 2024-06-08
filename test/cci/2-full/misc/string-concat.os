@_Sx0
  "Hello world!"
  '00
@_Sx1
  "Hello"
  " "
  "world"
  "!"
  '00
=main
  enter
  mov r1 0
  push r0
  push r1
  imw r0 ^_Sx0
  add r0 rpp r0
  imw r1 ^_Sx1
  add r1 rpp r1
  call ^strcmp
  mov r2 r0
  pop r1
  pop r0
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
  mov r0 0
  leave
  ret
