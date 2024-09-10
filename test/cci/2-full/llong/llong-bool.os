=main
  enter
  sub rsp rsp 12
  add r0 rfp -8
  imw r1 0
  stw r1 r0 0
  imw r1 1
  stw r1 r0 4
  sub rsp rsp 8
  mov r0 rsp
  add r1 rfp -8
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  ldw r0 r0 0
  add rsp rsp 8
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
  sub rsp rsp 8
  mov r0 rsp
  add r1 rfp -8
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  ldw r1 r0 4
  ldw r0 r0 0
  or r0 r0 r1
  bool r0 r0
  add rsp rsp 8
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  jmp &_Lx4
:_Lx3
  imw r0 2
  leave
  ret
  jmp &_Lx4
:_Lx4
  add r0 rfp -9
  sub rsp rsp 8
  mov r1 rsp
  add r2 rfp -8
  ldw r3 r2 0
  stw r3 r1 0
  ldw r3 r2 4
  stw r3 r1 4
  ldw r2 r1 4
  ldw r1 r1 0
  or r1 r1 r2
  bool r1 r1
  add rsp rsp 8
  stb r1 r0 0
  ldb r0 rfp -9
  trb r0 r0
  jnz r0 &_Lx5
  jmp &_Lx6
:_Lx5
  jmp &_Lx7
:_Lx6
  imw r0 3
  leave
  ret
  jmp &_Lx7
:_Lx7
  ldb r0 rfp -9
  trb r0 r0
  imw r1 1
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
  sub rsp rsp 8
  mov r0 rsp
  add r1 rfp -8
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  ldw r1 r0 0
  ldw r0 r0 4
  or r0 r0 r1
  add rsp rsp 8
  isz r0 r0
  jnz r0 &_LxA
  jmp &_LxB
:_LxA
  imw r0 5
  leave
  ret
  jmp &_LxB
:_LxB
  zero r0
  leave
  ret
