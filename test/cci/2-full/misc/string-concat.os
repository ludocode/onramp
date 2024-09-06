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
  imw r0 0
  push r0
  imw r0 ^_Sx0
  add r0 rpp r0
  imw r1 ^_Sx1
  add r1 rpp r1
  call ^strcmp
  mov r1 r0
  pop r0
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
  imw r0 0
  leave
  ret
