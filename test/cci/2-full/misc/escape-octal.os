@_Sx0
  '00"S"'FF
  '00
@_Sx1
  '00'01"8"'1B
  '00
=main
  enter
  sub rsp rsp 12
  add r0 rfp -4
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
  add r1 rfp -4
  imw r2 0
  add r1 r1 r2
  ldb r0 0 r1
  sxb r0 r0
  imw r1 0
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
  add r1 rfp -4
  imw r2 1
  add r1 r1 r2
  ldb r0 0 r1
  sxb r0 r0
  imw r1 83
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
  add r1 rfp -4
  imw r2 2
  add r1 r1 r2
  ldb r0 0 r1
  sxb r0 r0
  imw r1 1
  sub r1 0 r1
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
  add r0 rfp -9
  imw r1 ^_Sx1
  add r1 rpp r1
  imw r2 0
  add r2 r2 r0
  imw r4 5
  jmp &_Lx6
:_Lx6
  jz r4 &_Lx7
  sub r4 r4 1
  ldb r3 r1 r4
  stb r3 r2 r4
  jmp &_Lx6
:_Lx7
  add r1 rfp -9
  imw r2 0
  add r1 r1 r2
  ldb r0 0 r1
  sxb r0 r0
  imw r1 0
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
  add r1 rfp -9
  imw r2 1
  add r1 r1 r2
  ldb r0 0 r1
  sxb r0 r0
  imw r1 1
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
  add r1 rfp -9
  imw r2 2
  add r1 r1 r2
  ldb r0 0 r1
  sxb r0 r0
  mov r1 56
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxC
  jmp &_LxD
:_LxC
  imw r0 6
  leave
  ret
  jmp &_LxD
:_LxD
  add r1 rfp -9
  imw r2 3
  add r1 r1 r2
  ldb r0 0 r1
  sxb r0 r0
  imw r1 27
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxE
  jmp &_LxF
:_LxE
  imw r0 7
  leave
  ret
  jmp &_LxF
:_LxF
  zero r0
  leave
  ret
