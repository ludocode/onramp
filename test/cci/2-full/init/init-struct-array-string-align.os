@_Sx0
  "abcd"
  '00
=x 0
  0
@{50_Ix0_x
  enter
  imw r0 ^x
  add r0 rpp r0
  imw r1 1
  stb r1 r0 0
  imw r1 2
  stb r1 r0 1
  imw r1 3
  stb r1 r0 2
  imw r1 ^_Sx0
  add r1 rpp r1
  imw r2 3
  add r2 r2 r0
  ldb r3 r1 0
  stb r3 r2 0
  ldb r3 r1 1
  stb r3 r2 1
  ldb r3 r1 2
  stb r3 r2 2
  ldb r3 r1 3
  stb r3 r2 3
  imw r1 4
  stb r1 r0 7
  leave
  ret
=main
  enter
  imw r0 8
  imw r1 8
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
  imw r0 ^x
  add r0 rpp r0
  ldb r0 0 r0
  sxb r0 r0
  imw r1 1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx3
  jmp &_Lx4
:_Lx3
  imw r0 2
  leave
  ret
  jmp &_Lx4
:_Lx4
  imw r0 ^x
  add r0 rpp r0
  add r0 r0 1
  ldb r0 0 r0
  sxb r0 r0
  imw r1 2
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx5
  jmp &_Lx6
:_Lx5
  imw r0 3
  leave
  ret
  jmp &_Lx6
:_Lx6
  imw r0 ^x
  add r0 rpp r0
  add r0 r0 2
  ldb r0 0 r0
  sxb r0 r0
  imw r1 3
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx7
  jmp &_Lx8
:_Lx7
  imw r0 4
  leave
  ret
  jmp &_Lx8
:_Lx8
  imw r0 ^x
  add r0 rpp r0
  add r0 r0 3
  mov r0 r0
  imw r1 0
  add r0 r0 r1
  ldb r0 0 r0
  sxb r0 r0
  mov r1 97
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx9
  jmp &_LxA
:_Lx9
  imw r0 5
  leave
  ret
  jmp &_LxA
:_LxA
  imw r0 ^x
  add r0 rpp r0
  add r0 r0 3
  mov r0 r0
  imw r1 1
  add r0 r0 r1
  ldb r0 0 r0
  sxb r0 r0
  mov r1 98
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxB
  jmp &_LxC
:_LxB
  imw r0 6
  leave
  ret
  jmp &_LxC
:_LxC
  imw r0 ^x
  add r0 rpp r0
  add r0 r0 3
  mov r0 r0
  imw r1 2
  add r0 r0 r1
  ldb r0 0 r0
  sxb r0 r0
  mov r1 99
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxD
  jmp &_LxE
:_LxD
  imw r0 7
  leave
  ret
  jmp &_LxE
:_LxE
  imw r0 ^x
  add r0 rpp r0
  add r0 r0 3
  mov r0 r0
  imw r1 3
  add r0 r0 r1
  ldb r0 0 r0
  sxb r0 r0
  mov r1 100
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxF
  jmp &_Lx10
:_LxF
  imw r0 8
  leave
  ret
  jmp &_Lx10
:_Lx10
  imw r0 ^x
  add r0 rpp r0
  add r0 r0 7
  ldb r0 0 r0
  sxb r0 r0
  imw r1 4
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx11
  jmp &_Lx12
:_Lx11
  imw r0 9
  leave
  ret
  jmp &_Lx12
:_Lx12
  zero r0
  leave
  ret
