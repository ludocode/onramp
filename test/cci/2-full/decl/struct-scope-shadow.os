=main
  enter
  sub rsp rsp 8
  imw r0 4
  add r1 rfp -4
  add r1 r1 0
  stw r0 r1 0
  imw r0 4
  add r1 rfp -8
  add r1 r1 0
  stw r0 r1 0
  add r1 rfp -4
  ldw r0 0 r1
  add r2 rfp -8
  ldw r1 0 r2
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
  zero r0
  leave
  ret
