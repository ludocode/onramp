=foo
  enter
  add r0 rfp 8
  ldw r0 0 r0
  add r1 rfp 8
  add r1 r1 4
  ldw r1 0 r1
  add r0 r0 r1
  leave
  ret
=main
  enter
  sub rsp rsp 8
  add r0 rfp -8
  imw r1 2
  stw r1 r0 0
  imw r1 3
  stw r1 r0 4
  sub rsp rsp 8
  mov r0 rsp
  add r1 rfp -8
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  call ^foo
  mov r0 r0
  add rsp rsp 8
  imw r1 5
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
