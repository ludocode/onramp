@_Sx0
  "abcdefgh"
  '00
=x
  0
@{50_Ix0_x
  enter
  imw r0 ^x
  add r0 rpp r0
  imw r1 ^_Sx0
  add r1 rpp r1
  imw r2 0
  add r2 r2 r0
  ldb r3 r1 0
  stb r3 r2 0
  ldb r3 r1 1
  stb r3 r2 1
  ldb r3 r1 2
  stb r3 r2 2
  ldb r3 r1 3
  stb r3 r2 3
  leave
  ret
=y
  0
@{50_Ix1_y
  enter
  imw r0 ^y
  add r0 rpp r0
  imw r1 305419896
  stw r1 r0 0
  leave
  ret
=main
  enter
  imw r0 ^x
  add r0 rpp r0
  imw r1 0
  add r0 r0 r1
  ldb r0 0 r0
  sxb r0 r0
  mov r1 97
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
  imw r0 ^x
  add r0 rpp r0
  imw r1 1
  add r0 r0 r1
  ldb r0 0 r0
  sxb r0 r0
  mov r1 98
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
  imw r0 ^x
  add r0 rpp r0
  imw r1 2
  add r0 r0 r1
  ldb r0 0 r0
  sxb r0 r0
  mov r1 99
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
  imw r0 ^x
  add r0 rpp r0
  imw r1 3
  add r0 r0 r1
  ldb r0 0 r0
  sxb r0 r0
  mov r1 100
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
  imw r0 ^y
  ldw r0 rpp r0
  imw r1 305419896
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxA
  jmp &_LxB
:_LxA
  imw r0 5
  leave
  ret
  jmp &_LxB
:_LxB
  zero r0
  leave
  ret
