@_Sx0
  "Hello"
  " "
  "world"
  "!"
  '00
=x 0 0 0
  0
@{50_Ix0_x
  enter
  imw r0 ^x
  add r0 rpp r0
  imw r1 ^_Sx0
  add r1 rpp r1
  imw r2 0
  add r2 r2 r0
  imw r4 13
  jmp &_Lx1
:_Lx1
  jz r4 &_Lx2
  sub r4 r4 1
  ldb r3 r1 r4
  stb r3 r2 r4
  jmp &_Lx1
:_Lx2
  leave
  ret
@_Sx1
  "Hello world!"
  '00
=main
  enter
  imw r0 13
  imw r1 13
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx3
  jmp &_Lx4
:_Lx3
  imw r0 1
  leave
  ret
  jmp &_Lx4
:_Lx4
  imw r0 0
  push r0
  imw r0 ^x
  add r0 rpp r0
  imw r1 ^_Sx1
  add r1 rpp r1
  call ^strcmp
  mov r1 r0
  pop r0
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx5
  jmp &_Lx6
:_Lx5
  imw r0 2
  leave
  ret
  jmp &_Lx6
:_Lx6
  zero r0
  leave
  ret
