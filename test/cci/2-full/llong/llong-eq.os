=main
  enter
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
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
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  jmp &_Lx2
:_Lx1
  imw r0 1
  leave
  ret
  jmp &_Lx2
:_Lx2
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
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
  jnz r0 &_Lx3
  jmp &_Lx4
:_Lx3
  imw r0 2
  leave
  ret
  jmp &_Lx4
:_Lx4
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
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
  isz r0 r0
  jnz r0 &_Lx5
  jmp &_Lx6
:_Lx5
  imw r0 3
  leave
  ret
  jmp &_Lx6
:_Lx6
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
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
  jnz r0 &_Lx7
  jmp &_Lx8
:_Lx7
  jmp &_Lx9
:_Lx8
  imw r0 4
  leave
  ret
  jmp &_Lx9
:_Lx9
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 1
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
  jnz r0 &_LxA
  jmp &_LxB
:_LxA
  jmp &_LxC
:_LxB
  imw r0 5
  leave
  ret
  jmp &_LxC
:_LxC
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 1
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
  jnz r0 &_LxD
  jmp &_LxE
:_LxD
  imw r0 6
  leave
  ret
  jmp &_LxE
:_LxE
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 1
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
  jnz r0 &_LxF
  jmp &_Lx10
:_LxF
  imw r0 7
  leave
  ret
  jmp &_Lx10
:_Lx10
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 1
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
  jnz r0 &_Lx11
  jmp &_Lx12
:_Lx11
  jmp &_Lx13
:_Lx12
  imw r0 8
  leave
  ret
  jmp &_Lx13
:_Lx13
  sub rsp rsp 16
  mov r0 rsp
  imw r1 -1
  stw r1 r0 0
  imw r1 -1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 -1
  stw r2 r1 0
  imw r2 -1
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
  jnz r0 &_Lx14
  jmp &_Lx15
:_Lx14
  jmp &_Lx16
:_Lx15
  imw r0 9
  leave
  ret
  jmp &_Lx16
:_Lx16
  sub rsp rsp 16
  mov r0 rsp
  imw r1 -1
  stw r1 r0 0
  imw r1 -1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 -1
  stw r2 r1 0
  imw r2 -1
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
  jnz r0 &_Lx17
  jmp &_Lx18
:_Lx17
  imw r0 10
  leave
  ret
  jmp &_Lx18
:_Lx18
  zero r0
  leave
  ret
