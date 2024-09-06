@_Sx0
  "Hello"
  '00
@_Sx1
  "World"
  '00
=main
  enter
  imw r0 6
  imw r1 6
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
  imw r0 6
  imw r1 6
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
  imw r0 0
  push r0
  imw r0 ^hello
  add r0 rpp r0
  imw r1 ^_Sx0
  add r1 rpp r1
  call ^strcmp
  mov r1 r0
  pop r0
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
  imw r0 0
  push r0
  imw r0 ^world
  add r0 rpp r0
  imw r1 ^_Sx1
  add r1 rpp r1
  call ^strcmp
  mov r1 r0
  pop r0
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
@_Sx2
  "Hello"
  '00
=hello 0
  0
@{50_Ix8_hello
  enter
  imw r0 ^hello
  add r0 rpp r0
  imw r1 ^_Sx2
  add r1 rpp r1
  imw r2 0
  add r2 r2 r0
  imw r4 6
  jmp &_Lx9
:_Lx9
  jz r4 &_LxA
  sub r4 r4 1
  ldb r3 r1 r4
  stb r3 r2 r4
  jmp &_Lx9
:_LxA
  leave
  ret
@_Sx3
  "World"
  '00
=world 0
  0
@{50_Ixb_world
  enter
  imw r0 ^world
  add r0 rpp r0
  imw r1 ^_Sx3
  add r1 rpp r1
  imw r2 0
  add r2 r2 r0
  imw r4 6
  jmp &_LxC
:_LxC
  jz r4 &_LxD
  sub r4 r4 1
  ldb r3 r1 r4
  stb r3 r2 r4
  jmp &_LxC
:_LxD
  leave
  ret
