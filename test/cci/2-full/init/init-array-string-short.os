@_Sx0
  "foo"
  '00
=main
  enter
  sub rsp rsp 8
  add r0 rfp -4
  imw r1 305419896
  stw r1 r0 0
  add r0 rfp -8
  imw r1 305419896
  stw r1 r0 0
  add r0 rfp -8
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
  add r2 r2 4
  stb 0 r2 0
  stb 0 r2 1
  stb 0 r2 2
  stb 0 r2 3
  add r2 rfp -8
  mov r3 0
  add r2 r2 r3
  ldb r0 0 r2
  sxb r0 r0
  mov r1 102
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  mov r0 1
  leave
  ret
  jmp &_Lx1
:_Lx1
  add r2 rfp -8
  mov r3 1
  add r2 r2 r3
  ldb r0 0 r2
  sxb r0 r0
  mov r1 111
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  mov r0 2
  leave
  ret
  jmp &_Lx3
:_Lx3
  add r2 rfp -8
  mov r3 2
  add r2 r2 r3
  ldb r0 0 r2
  sxb r0 r0
  mov r1 111
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx4
  jmp &_Lx5
:_Lx4
  mov r0 3
  leave
  ret
  jmp &_Lx5
:_Lx5
  add r2 rfp -8
  mov r3 3
  add r2 r2 r3
  ldb r0 0 r2
  sxb r0 r0
  mov r1 0
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx6
  jmp &_Lx7
:_Lx6
  mov r0 4
  leave
  ret
  jmp &_Lx7
:_Lx7
  add r2 rfp -8
  mov r3 4
  add r2 r2 r3
  ldb r0 0 r2
  sxb r0 r0
  mov r1 0
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx8
  jmp &_Lx9
:_Lx8
  mov r0 5
  leave
  ret
  jmp &_Lx9
:_Lx9
  add r2 rfp -8
  mov r3 5
  add r2 r2 r3
  ldb r0 0 r2
  sxb r0 r0
  mov r1 0
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_LxA
  jmp &_LxB
:_LxA
  mov r0 6
  leave
  ret
  jmp &_LxB
:_LxB
  add r2 rfp -8
  mov r3 6
  add r2 r2 r3
  ldb r0 0 r2
  sxb r0 r0
  mov r1 0
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_LxC
  jmp &_LxD
:_LxC
  mov r0 7
  leave
  ret
  jmp &_LxD
:_LxD
  add r2 rfp -8
  mov r3 7
  add r2 r2 r3
  ldb r0 0 r2
  sxb r0 r0
  mov r1 0
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_LxE
  jmp &_LxF
:_LxE
  mov r0 8
  leave
  ret
  jmp &_LxF
:_LxF
  zero r0
  leave
  ret
