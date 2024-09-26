=foo
  enter
  sub rsp rsp 16
  stw r0 rfp -4
  stw r1 rfp -8
  stw r2 rfp -12
  stw r3 rfp -16
  ldw r0 rfp 8
  ldw r1 rfp -4
  imw r2 2
  ldw r3 rfp -8
  mul r2 r2 r3
  add r1 r1 r2
  imw r2 3
  ldw r3 rfp -12
  mul r2 r2 r3
  add r1 r1 r2
  imw r2 4
  ldw r3 rfp -16
  mul r2 r2 r3
  add r1 r1 r2
  imw r2 5
  ldw r3 rfp 12
  mul r2 r2 r3
  add r1 r1 r2
  stw r1 r0 0
  stw 0 r0 4
  leave
  ret
=main
  enter
  sub rsp rsp 16
  mov r0 rsp
  imw r1 143
  stw r1 r0 0
  shru r1 r1 31
  sub r1 0 r1
  stw r1 r0 4
  add r1 rsp 8
  push r0
  mov r0 r1
  imw r1 13
  push r1
  push r0
  imw r0 3
  imw r1 5
  imw r2 7
  imw r3 11
  call ^foo
  mov r1 r0
  add rsp rsp 8
  pop r0
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
