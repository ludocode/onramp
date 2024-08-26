@_Sx0
  "abcd"
  '00
=x 0
  0
@{50_Ix0_x
  enter
  imw r0 ^x
  add r0 rpp r0
  mov r1 1
  sxb r1 r1
  stb r1 r0 0
  mov r1 2
  sxb r1 r1
  stb r1 r0 1
  mov r1 3
  sxb r1 r1
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
  mov r1 4
  sxb r1 r1
  stb r1 r0 7
  leave
  ret
=main
  enter
  mov r0 8
  mov r1 8
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx1
  jmp &_Lx2
:_Lx1
  mov r0 1
  leave
  ret
  jmp &_Lx2
:_Lx2
  imw r2 ^x
  add r2 rpp r2
  ldb r0 0 r2
  sxb r0 r0
  mov r1 1
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx3
  jmp &_Lx4
:_Lx3
  mov r0 2
  leave
  ret
  jmp &_Lx4
:_Lx4
  imw r2 ^x
  add r2 rpp r2
  add r2 r2 1
  ldb r0 0 r2
  sxb r0 r0
  mov r1 2
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx5
  jmp &_Lx6
:_Lx5
  mov r0 3
  leave
  ret
  jmp &_Lx6
:_Lx6
  imw r2 ^x
  add r2 rpp r2
  add r2 r2 2
  ldb r0 0 r2
  sxb r0 r0
  mov r1 3
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx7
  jmp &_Lx8
:_Lx7
  mov r0 4
  leave
  ret
  jmp &_Lx8
:_Lx8
  imw r2 ^x
  add r2 rpp r2
  add r2 r2 3
  mov r3 0
  add r2 r2 r3
  ldb r0 0 r2
  sxb r0 r0
  mov r1 97
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx9
  jmp &_LxA
:_Lx9
  mov r0 5
  leave
  ret
  jmp &_LxA
:_LxA
  imw r2 ^x
  add r2 rpp r2
  add r2 r2 3
  mov r3 1
  add r2 r2 r3
  ldb r0 0 r2
  sxb r0 r0
  mov r1 98
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_LxB
  jmp &_LxC
:_LxB
  mov r0 6
  leave
  ret
  jmp &_LxC
:_LxC
  imw r2 ^x
  add r2 rpp r2
  add r2 r2 3
  mov r3 2
  add r2 r2 r3
  ldb r0 0 r2
  sxb r0 r0
  mov r1 99
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_LxD
  jmp &_LxE
:_LxD
  mov r0 7
  leave
  ret
  jmp &_LxE
:_LxE
  imw r2 ^x
  add r2 rpp r2
  add r2 r2 3
  mov r3 3
  add r2 r2 r3
  ldb r0 0 r2
  sxb r0 r0
  mov r1 100
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_LxF
  jmp &_Lx10
:_LxF
  mov r0 8
  leave
  ret
  jmp &_Lx10
:_Lx10
  imw r2 ^x
  add r2 rpp r2
  add r2 r2 7
  ldb r0 0 r2
  sxb r0 r0
  mov r1 4
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx11
  jmp &_Lx12
:_Lx11
  mov r0 9
  leave
  ret
  jmp &_Lx12
:_Lx12
  zero r0
  leave
  ret
