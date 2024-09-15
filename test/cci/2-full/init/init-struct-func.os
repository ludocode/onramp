=foo
  enter
  sub rsp rsp 8
  add r0 rfp -8
  imw r1 2
  stw r1 r0 0
  imw r1 3
  stw r1 r0 4
  ldw r0 rfp 8
  add r1 rfp -8
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  leave
  ret
=main
  enter
  sub rsp rsp 24
  imw r0 5
  imw r1 3
  mul r0 r0 r1
  imw r1 4
  imw r2 7
  mul r1 r1 r2
  add r0 r0 r1
  add r0 rfp -8
  mov r0 r0
  push r0
  call ^foo
  mov r0 r0
  add rsp rsp 4
  add r0 rfp -8
  ldw r0 0 r0
  imw r1 2
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
  add r0 rfp -8
  add r0 r0 4
  ldw r0 0 r0
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
  add r0 rfp -24
  mov r0 r0
  push r0
  call ^foo
  mov r0 r0
  add rsp rsp 4
  imw r1 8
  add r1 r1 r0
  push r0
  mov r0 r1
  push r0
  call ^foo
  mov r1 r0
  add rsp rsp 4
  pop r0
  add r0 rfp -24
  add r0 r0 0
  ldw r0 0 r0
  imw r1 2
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
  add r0 rfp -24
  add r0 r0 0
  add r0 r0 4
  ldw r0 0 r0
  imw r1 3
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
  add r0 rfp -24
  add r0 r0 8
  ldw r0 0 r0
  imw r1 2
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
  add r0 rfp -24
  add r0 r0 8
  add r0 r0 4
  ldw r0 0 r0
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
  zero r0
  leave
  ret
