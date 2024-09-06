=main
  enter
  sub rsp rsp 4
  imw r0 5
  add r1 rfp -3
  stb r0 r1 0
  imw r0 7
  add r1 rfp -3
  imw r2 1
  add r1 r1 r2
  stb r0 r1 0
  imw r0 11
  add r1 rfp -3
  imw r2 2
  add r1 r1 r2
  stb r0 r1 0
  imw r0 1
  imw r1 1
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
  imw r0 3
  imw r1 3
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
  imw r0 4
  imw r1 4
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
  imw r0 4
  imw r1 4
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
  imw r0 4
  imw r1 4
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx8
  jmp &_Lx9
:_Lx8
  imw r0 5
  leave
  ret
  jmp &_Lx9
:_Lx9
  imw r0 3
  imw r1 3
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxA
  jmp &_LxB
:_LxA
  imw r0 6
  leave
  ret
  jmp &_LxB
:_LxB
  imw r0 1
  imw r1 1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxC
  jmp &_LxD
:_LxC
  imw r0 7
  leave
  ret
  jmp &_LxD
:_LxD
  add r0 rfp -3
  add r1 rfp -3
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxE
  jmp &_LxF
:_LxE
  imw r0 11
  leave
  ret
  jmp &_LxF
:_LxF
  add r0 rfp -3
  add r1 rfp -3
  imw r2 0
  add r1 r1 r2
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx10
  jmp &_Lx11
:_Lx10
  imw r0 12
  leave
  ret
  jmp &_Lx11
:_Lx11
  add r0 rfp -3
  add r1 rfp -3
  imw r2 0
  add r1 r1 r2
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx12
  jmp &_Lx13
:_Lx12
  imw r0 13
  leave
  ret
  jmp &_Lx13
:_Lx13
  add r0 rfp -3
  add r1 rfp -3
  imw r2 0
  add r1 r1 r2
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx14
  jmp &_Lx15
:_Lx14
  imw r0 14
  leave
  ret
  jmp &_Lx15
:_Lx15
  add r0 rfp -3
  add r1 rfp -3
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx16
  jmp &_Lx17
:_Lx16
  imw r0 15
  leave
  ret
  jmp &_Lx17
:_Lx17
  add r0 rfp -3
  add r1 rfp -3
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx18
  jmp &_Lx19
:_Lx18
  imw r0 16
  leave
  ret
  jmp &_Lx19
:_Lx19
  add r0 rfp -3
  ldb r0 0 r0
  sxb r0 r0
  imw r1 5
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx1A
  jmp &_Lx1B
:_Lx1A
  imw r0 21
  leave
  ret
  jmp &_Lx1B
:_Lx1B
  add r1 rfp -3
  imw r2 1
  add r1 r1 r2
  ldb r0 0 r1
  sxb r0 r0
  imw r1 7
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx1C
  jmp &_Lx1D
:_Lx1C
  imw r0 22
  leave
  ret
  jmp &_Lx1D
:_Lx1D
  add r0 rfp -3
  imw r1 2
  add r0 r0 r1
  ldb r0 0 r0
  sxb r0 r0
  imw r1 11
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx1E
  jmp &_Lx1F
:_Lx1E
  imw r0 23
  leave
  ret
  jmp &_Lx1F
:_Lx1F
  add r0 rfp -3
  ldb r0 0 r0
  sxb r0 r0
  imw r1 5
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx20
  jmp &_Lx21
:_Lx20
  imw r0 24
  leave
  ret
  jmp &_Lx21
:_Lx21
  add r0 rfp -3
  ldb r0 0 r0
  sxb r0 r0
  imw r1 5
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx22
  jmp &_Lx23
:_Lx22
  imw r0 25
  leave
  ret
  jmp &_Lx23
:_Lx23
  zero r0
  leave
  ret
