=main
  enter
  sub rsp rsp 8
  imw r0 3
  add r1 rfp -8
  imw r2 1
  stw r2 r1 0
  jmp &_Lx0
:_Lx0
  ldw r1 rfp -8
  jz r1 &_Lx1
  imw r1 0
  add r2 rfp -8
  stw r1 r2 0
  jmp &_Lx0
:_Lx1
  imw r1 5
  add r0 r0 r1
  add r1 rfp -4
  stw r0 r1 0
  ldw r0 rfp -4
  imw r1 8
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  imw r0 1
  leave
  ret
  jmp &_Lx3
:_Lx3
  imw r0 0
  leave
  ret
