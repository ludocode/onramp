=main
  enter
  sub rsp rsp 4
  imw r1 256
  bool r1 r1
  add r2 rfp -1
  stb r1 0 r2
  ldb r0 rfp -1
  bool r0 r0
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
  imw r0 256
  bool r0 r0
  add r1 rfp -1
  stb r0 0 r1
  ldb r0 rfp -1
  bool r0 r0
  mov r1 1
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
  imw r0 512
  sxb r0 r0
  sxb r0 r0
  bool r0 r0
  add r1 rfp -1
  stb r0 0 r1
  ldb r0 rfp -1
  bool r0 r0
  mov r1 0
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
  mov r0 0
  leave
  ret
