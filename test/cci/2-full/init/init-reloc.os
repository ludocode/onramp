@_Sx0
  "Hello"
  '00
=x 0
  0
@{50_Ix0_x
  enter
  imw r0 ^x
  add r0 rpp r0
  imw r1 ^_Sx0
  add r1 rpp r1
  imw r2 0
  add r2 r2 r0
  imw r4 6
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
=y
  0
@{50_Ix3_y
  enter
  imw r0 ^y
  add r0 rpp r0
  imw r1 ^x
  add r1 rpp r1
  imw r2 2
  add r1 r1 r2
  stw r1 r0 0
  leave
  ret
=main
  enter
  imw r0 ^y
  ldw r0 rpp r0
  imw r1 2
  sub r1 0 r1
  add r0 r0 r1
  ldb r0 0 r0
  sxb r0 r0
  mov r1 72
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
  imw r0 ^y
  ldw r0 rpp r0
  imw r1 1
  sub r1 0 r1
  add r0 r0 r1
  ldb r0 0 r0
  sxb r0 r0
  mov r1 101
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx6
  jmp &_Lx7
:_Lx6
  imw r0 2
  leave
  ret
  jmp &_Lx7
:_Lx7
  imw r0 ^y
  ldw r0 rpp r0
  imw r1 0
  add r0 r0 r1
  ldb r0 0 r0
  sxb r0 r0
  mov r1 108
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx8
  jmp &_Lx9
:_Lx8
  imw r0 3
  leave
  ret
  jmp &_Lx9
:_Lx9
  imw r0 ^y
  ldw r0 rpp r0
  imw r1 1
  add r0 r0 r1
  ldb r0 0 r0
  sxb r0 r0
  mov r1 108
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxA
  jmp &_LxB
:_LxA
  imw r0 4
  leave
  ret
  jmp &_LxB
:_LxB
  imw r0 ^y
  ldw r0 rpp r0
  imw r1 2
  add r0 r0 r1
  ldb r0 0 r0
  sxb r0 r0
  mov r1 111
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxC
  jmp &_LxD
:_LxC
  imw r0 5
  leave
  ret
  jmp &_LxD
:_LxD
  imw r0 ^y
  ldw r0 rpp r0
  imw r1 3
  add r0 r0 r1
  ldb r0 0 r0
  sxb r0 r0
  imw r1 0
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxE
  jmp &_LxF
:_LxE
  imw r0 6
  leave
  ret
  jmp &_LxF
:_LxF
  zero r0
  leave
  ret
