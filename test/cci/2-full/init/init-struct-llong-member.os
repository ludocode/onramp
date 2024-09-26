=main
  enter
  sub rsp rsp 8
  add r0 rfp -8
  imw r1 4
  stw r1 r0 0
  shru r1 r1 31
  sub r1 0 r1
  stw r1 r0 4
  sub rsp rsp 16
  mov r0 rsp
  imw r1 4
  stw r1 r0 0
  shru r1 r1 31
  sub r1 0 r1
  stw r1 r0 4
  add r1 rsp 8
  add r2 rfp -8
  ldw r3 r2 0
  stw r3 r1 0
  ldw r3 r2 4
  stw r3 r1 4
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
  zero r0
  leave
  ret
