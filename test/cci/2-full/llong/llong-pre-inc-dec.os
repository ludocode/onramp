=main
  enter
  sub rsp rsp 24
  add r0 rfp -8
  imw r1 2004318071
  stw r1 r0 0
  imw r1 2004318071
  stw r1 r0 4
  add r0 rfp -16
  imw r1 0
  stw r1 r0 0
  shru r1 r1 31
  sub r1 0 r1
  stw r1 r0 4
  add r0 rfp -24
  imw r1 -858993460
  stw r1 r0 0
  imw r1 -858993460
  stw r1 r0 4
  sub rsp rsp 16
  mov r0 rsp
  add r1 rfp -16
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  ldw r2 r0 0
  inc r2
  stw r2 r0 0
  isz r2 r2
  ldw r3 r0 4
  add r3 r3 r2
  stw r3 r0 4
  ldw r2 r0 0
  stw r2 r1 0
  ldw r2 r0 4
  stw r2 r1 4
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  shru r2 r2 31
  sub r2 0 r2
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
  add r1 rfp -16
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  ldw r2 r0 0
  dec r2
  stw r2 r0 0
  sub r2 r2 -1
  isz r2 r2
  ldw r3 r0 4
  sub r3 r3 r2
  stw r3 r0 4
  ldw r2 r0 0
  stw r2 r1 0
  ldw r2 r0 4
  stw r2 r1 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  shru r2 r2 31
  sub r2 0 r2
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
  sub rsp rsp 16
  mov r0 rsp
  add r1 rfp -16
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  ldw r2 r0 0
  dec r2
  stw r2 r0 0
  sub r2 r2 -1
  isz r2 r2
  ldw r3 r0 4
  sub r3 r3 r2
  stw r3 r0 4
  ldw r2 r0 0
  stw r2 r1 0
  ldw r2 r0 4
  stw r2 r1 4
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
  jnz r0 &_Lx4
  jmp &_Lx5
:_Lx4
  imw r0 3
  leave
  ret
  jmp &_Lx5
:_Lx5
  sub rsp rsp 16
  mov r0 rsp
  add r1 rfp -16
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  ldw r2 r0 0
  inc r2
  stw r2 r0 0
  isz r2 r2
  ldw r3 r0 4
  add r3 r3 r2
  stw r3 r0 4
  ldw r2 r0 0
  stw r2 r1 0
  ldw r2 r0 4
  stw r2 r1 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  shru r2 r2 31
  sub r2 0 r2
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
  add r1 rfp -8
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  add r1 rsp 8
  imw r2 2004318071
  stw r2 r1 0
  imw r2 2004318071
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
  sub rsp rsp 16
  mov r0 rsp
  add r1 rfp -24
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  add r1 rsp 8
  imw r2 -858993460
  stw r2 r1 0
  imw r2 -858993460
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
  imw r0 6
  leave
  ret
  jmp &_LxB
:_LxB
  zero r0
  leave
  ret
