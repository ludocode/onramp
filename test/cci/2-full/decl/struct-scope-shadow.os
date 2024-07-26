=main
  enter
  sub rsp rsp 8
  mov r0 4
  add r1 rfp -4
  add r1 r1 0
  stw r0 0 r1
  mov r0 4
  add r1 rfp -8
  add r1 r1 0
  stw r0 0 r1
  add r2 rfp -4
  ldw r0 0 r2
  add r2 rfp -8
  ldw r1 0 r2
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
  zero r0
  leave
  ret
