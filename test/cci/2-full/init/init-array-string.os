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
=main
  enter
  imw r1 ^x
  add r1 rpp r1
  imw r2 0
  add r1 r1 r2
  ldb r0 0 r1
  sxb r0 r0
  mov r1 72
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
  imw r1 ^x
  add r1 rpp r1
  imw r2 1
  add r1 r1 r2
  ldb r0 0 r1
  sxb r0 r0
  mov r1 101
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
  imw r1 ^x
  add r1 rpp r1
  imw r2 2
  add r1 r1 r2
  ldb r0 0 r1
  sxb r0 r0
  mov r1 108
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx7
  jmp &_Lx8
:_Lx7
  imw r0 3
  leave
  ret
  jmp &_Lx8
:_Lx8
  imw r1 ^x
  add r1 rpp r1
  imw r2 3
  add r1 r1 r2
  ldb r0 0 r1
  sxb r0 r0
  mov r1 108
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx9
  jmp &_LxA
:_Lx9
  imw r0 4
  leave
  ret
  jmp &_LxA
:_LxA
  imw r1 ^x
  add r1 rpp r1
  imw r2 4
  add r1 r1 r2
  ldb r0 0 r1
  sxb r0 r0
  mov r1 111
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxB
  jmp &_LxC
:_LxB
  imw r0 5
  leave
  ret
  jmp &_LxC
:_LxC
  imw r1 ^x
  add r1 rpp r1
  imw r2 5
  add r1 r1 r2
  ldb r0 0 r1
  sxb r0 r0
  imw r1 0
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxD
  jmp &_LxE
:_LxD
  imw r0 6
  leave
  ret
  jmp &_LxE
:_LxE
  zero r0
  leave
  ret
