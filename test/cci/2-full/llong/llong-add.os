=main
  enter
  sub rsp rsp 8
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 3
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  sub rsp rsp 8
  mov r2 rsp
  imw r3 5
  stw r3 r2 0
  imw r3 0
  stw r3 r2 4
  call ^__llong_add
  mov r0 r0
  add rsp rsp 16
  add rsp rsp 8
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  sub rsp rsp 8
  mov r2 rsp
  imw r3 0
  stw r3 r2 0
  imw r3 0
  stw r3 r2 4
  call ^__llong_add
  mov r0 r0
  add rsp rsp 16
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  sub r0 r0 r2
  ldw r1 rsp 4
  ldw r2 rsp 12
  sub r1 r1 r2
  or r0 r0 r1
  add rsp rsp 16
  bool r0 r0
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  imw r0 1
  leave
  ret
  jmp &_Lx1
:_Lx1
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  sub rsp rsp 8
  mov r2 rsp
  imw r3 0
  stw r3 r2 0
  imw r3 0
  stw r3 r2 4
  call ^__llong_add
  mov r0 r0
  add rsp rsp 16
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  sub r0 r0 r2
  ldw r1 rsp 4
  ldw r2 rsp 12
  sub r1 r1 r2
  or r0 r0 r1
  add rsp rsp 16
  bool r0 r0
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
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  sub rsp rsp 8
  mov r2 rsp
  imw r3 0
  stw r3 r2 0
  imw r3 0
  stw r3 r2 4
  call ^__llong_add
  mov r0 r0
  add rsp rsp 16
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  sub r0 r0 r2
  ldw r1 rsp 4
  ldw r2 rsp 12
  sub r1 r1 r2
  or r0 r0 r1
  add rsp rsp 16
  isz r0 r0
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
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 3
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  sub rsp rsp 8
  mov r2 rsp
  imw r3 5
  stw r3 r2 0
  imw r3 0
  stw r3 r2 4
  call ^__llong_add
  mov r0 r0
  add rsp rsp 16
  add r1 rsp 8
  imw r2 8
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  sub r0 r0 r2
  ldw r1 rsp 4
  ldw r2 rsp 12
  sub r1 r1 r2
  or r0 r0 r1
  add rsp rsp 16
  bool r0 r0
  jnz r0 &_Lx8
  jmp &_Lx9
:_Lx8
  imw r0 4
  leave
  ret
  jmp &_Lx9
:_Lx9
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 1
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  sub rsp rsp 8
  mov r2 rsp
  imw r3 16
  stw r3 r2 0
  imw r3 16
  stw r3 r2 4
  call ^__llong_add
  mov r0 r0
  add rsp rsp 16
  add r1 rsp 8
  imw r2 17
  stw r2 r1 0
  imw r2 17
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  sub r0 r0 r2
  ldw r1 rsp 4
  ldw r2 rsp 12
  sub r1 r1 r2
  or r0 r0 r1
  add rsp rsp 16
  bool r0 r0
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
