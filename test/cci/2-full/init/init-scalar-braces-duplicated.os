=main
  enter
  sub rsp rsp 4
  add r0 rfp -4
  imw r1 4
  stw r1 r0 0
  ldw r0 rfp -4
  imw r1 4
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
