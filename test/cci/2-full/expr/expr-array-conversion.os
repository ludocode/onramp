=main
  enter
  sub rsp rsp 4
  mov r0 5
  add r1 rfp -3
  stb r0 r1 0
  mov r0 7
  add r1 rfp -3
  mov r2 1
  add r1 r1 r2
  stb r0 r1 0
  mov r0 11
  add r1 rfp -3
  mov r2 2
  add r1 r1 r2
  stb r0 r1 0
  mov r0 1
  mov r1 1
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
  mov r0 3
  mov r1 3
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
  mov r0 4
  mov r1 4
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
  mov r0 4
  mov r1 4
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
  mov r0 4
  mov r1 4
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
  mov r0 3
  mov r1 3
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
  mov r0 1
  mov r1 1
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
  add r0 rfp -3
  add r1 rfp -3
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_LxE
  jmp &_LxF
:_LxE
  mov r0 11
  leave
  ret
  jmp &_LxF
:_LxF
  add r0 rfp -3
  add r1 rfp -3
  mov r2 0
  add r1 r1 r2
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx10
  jmp &_Lx11
:_Lx10
  mov r0 12
  leave
  ret
  jmp &_Lx11
:_Lx11
  add r0 rfp -3
  add r1 rfp -3
  mov r2 0
  add r1 r1 r2
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx12
  jmp &_Lx13
:_Lx12
  mov r0 13
  leave
  ret
  jmp &_Lx13
:_Lx13
  add r0 rfp -3
  add r1 rfp -3
  mov r2 0
  add r1 r1 r2
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx14
  jmp &_Lx15
:_Lx14
  mov r0 14
  leave
  ret
  jmp &_Lx15
:_Lx15
  add r0 rfp -3
  add r1 rfp -3
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx16
  jmp &_Lx17
:_Lx16
  mov r0 15
  leave
  ret
  jmp &_Lx17
:_Lx17
  add r0 rfp -3
  add r1 rfp -3
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx18
  jmp &_Lx19
:_Lx18
  mov r0 16
  leave
  ret
  jmp &_Lx19
:_Lx19
  add r0 rfp -3
  ldb r0 0 r0
  sxb r0 r0
  mov r1 5
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx1A
  jmp &_Lx1B
:_Lx1A
  mov r0 21
  leave
  ret
  jmp &_Lx1B
:_Lx1B
  add r2 rfp -3
  mov r3 1
  add r2 r2 r3
  ldb r0 0 r2
  sxb r0 r0
  mov r1 7
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx1C
  jmp &_Lx1D
:_Lx1C
  mov r0 22
  leave
  ret
  jmp &_Lx1D
:_Lx1D
  add r0 rfp -3
  mov r2 2
  add r0 r0 r2
  ldb r0 0 r0
  sxb r0 r0
  mov r1 11
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx1E
  jmp &_Lx1F
:_Lx1E
  mov r0 23
  leave
  ret
  jmp &_Lx1F
:_Lx1F
  add r0 rfp -3
  ldb r0 0 r0
  sxb r0 r0
  mov r1 5
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx20
  jmp &_Lx21
:_Lx20
  mov r0 24
  leave
  ret
  jmp &_Lx21
:_Lx21
  add r0 rfp -3
  ldb r0 0 r0
  sxb r0 r0
  mov r1 5
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx22
  jmp &_Lx23
:_Lx22
  mov r0 25
  leave
  ret
  jmp &_Lx23
:_Lx23
  zero r0
  leave
  ret
