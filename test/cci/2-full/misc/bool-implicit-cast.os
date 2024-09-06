=main
  enter
  sub rsp rsp 4
  add r0 rfp -1
  imw r1 256
  bool r1 r1
  stb r1 r0 0
  ldb r0 rfp -1
  trb r0 r0
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
  imw r0 256
  bool r0 r0
  add r1 rfp -1
  stb r0 r1 0
  ldb r0 rfp -1
  trb r0 r0
  imw r1 1
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
  imw r0 512
  trb r0 r0
  bool r0 r0
  add r1 rfp -1
  stb r0 r1 0
  ldb r0 rfp -1
  trb r0 r0
  imw r1 0
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
  imw r0 -851968
  trs r0 r0
  bool r0 r0
  add r1 rfp -1
  stb r0 r1 0
  ldb r0 rfp -1
  trb r0 r0
  imw r1 0
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
  imw r0 0
  leave
  ret
