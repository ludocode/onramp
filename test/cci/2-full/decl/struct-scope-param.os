=foo
  enter
  sub rsp rsp 12
  stw r0 rfp -4
  stw r1 rfp -8
  ldw r0 rfp -4
  imw r1 0
  sub r0 r0 r1
  isz r0 r0
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  imw r0 2
  add r1 rfp -12
  stw r0 r1 0
  add r0 rfp -12
  imw r1 0
  call ^foo
  mov r0 r0
  leave
  ret
  jmp &_Lx1
:_Lx1
  ldw r0 rfp -4
  ldw r0 0 r0
  leave
  ret
=main
  enter
  imw r0 2
  push r0
  imw r0 0
  imw r1 0
  call ^foo
  mov r1 r0
  pop r0
  sub r0 r0 r1
  leave
  ret
