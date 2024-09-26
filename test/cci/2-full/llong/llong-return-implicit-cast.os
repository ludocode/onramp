=foo
  enter
  ldw r0 rfp 8
  imw r1 5
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  leave
  ret
=main
  enter
  sub rsp rsp 4
  add r0 rfp -4
  sub rsp rsp 8
  mov r1 rsp
  push r0
  mov r0 r1
  push r0
  call ^foo
  mov r1 r0
  add rsp rsp 4
  pop r0
  ldw r1 r1 0
  add rsp rsp 8
  stw r1 r0 0
  imw r0 5
  ldw r1 rfp -4
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
