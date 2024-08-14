=foo
  enter
  sub rsp rsp 12
  stw r0 rfp -4
  stw r1 rfp -8
  ldw r0 rfp -4
  mov r1 0
  sub r0 r0 r1
  cmpu r0 r0 0
  add r0 r0 1
  and r0 r0 1
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  mov r0 2
  add r1 rfp -12
  add r1 r1 0
  stw r0 r1 0
  add r0 rfp -12
  mov r1 0
  call ^foo
  mov r0 r0
  leave
  ret
  jmp &_Lx1
:_Lx1
  ldw r1 rfp -4
  ldw r0 0 r1
  leave
  ret
=main
  enter
  mov r0 2
  push r0
  mov r0 0
  mov r1 0
  call ^foo
  mov r1 r0
  pop r0
  sub r0 r0 r1
  leave
  ret
