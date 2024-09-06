=main
  enter
  sub rsp rsp 40
  add r0 rfp -8
  imw r1 -1412567278
  stw r1 r0 0
  imw r1 305419896
  stw r1 r0 4
  sub rsp rsp 16
  mov r0 rsp
  add r1 rfp -8
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  add r1 rsp 8
  imw r2 -1412567278
  stw r2 r1 0
  imw r2 305419896
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
  add r0 rfp -16
  imw r1 305419896
  stw r1 r0 0
  imw r1 -1412567125
  stw r1 r0 4
  sub rsp rsp 16
  mov r0 rsp
  add r1 rfp -16
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  add r1 rsp 8
  imw r2 305419896
  stw r2 r1 0
  imw r2 -1412567125
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
  imw r0 2
  leave
  ret
  jmp &_Lx3
:_Lx3
  add r0 rfp -24
  imw r1 1
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  sub rsp rsp 16
  mov r0 rsp
  add r1 rfp -24
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
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
  jnz r0 &_Lx4
  jmp &_Lx5
:_Lx4
  imw r0 3
  leave
  ret
  jmp &_Lx5
:_Lx5
  add r0 rfp -40
  imw r1 -1412567278
  stw r1 r0 0
  imw r1 305419896
  stw r1 r0 4
  imw r1 8
  add r1 r1 r0
  imw r2 305419896
  stw r2 r1 0
  imw r2 -1412567125
  stw r2 r1 4
  sub rsp rsp 16
  mov r0 rsp
  add r1 rfp -40
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  add r1 rsp 8
  imw r2 -1412567278
  stw r2 r1 0
  imw r2 305419896
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
  jnz r0 &_Lx6
  jmp &_Lx7
:_Lx6
  imw r0 4
  leave
  ret
  jmp &_Lx7
:_Lx7
  sub rsp rsp 16
  mov r0 rsp
  add r1 rfp -40
  add r1 r1 8
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  add r1 rsp 8
  imw r2 305419896
  stw r2 r1 0
  imw r2 -1412567125
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
  imw r0 5
  leave
  ret
  jmp &_Lx9
:_Lx9
  zero r0
  leave
  ret
