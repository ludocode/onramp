=foo
  enter
  mov r0 5
  leave
  ret
=main
  enter
  sub rsp rsp 4
  add r0 rfp -4
  imw r1 ^foo
  add r1 rpp r1
  stw r1 r0 0
  mov r0 5
  push r0
  ldw r0 rfp -4
  call r0
  mov r1 r0
  pop r0
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  mov r0 1
  leave
  ret
  jmp &_Lx1
:_Lx1
  mov r0 5
  push r0
  ldw r0 rfp -4
  call r0
  mov r1 r0
  pop r0
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  mov r0 2
  leave
  ret
  jmp &_Lx3
:_Lx3
  zero r0
  leave
  ret
