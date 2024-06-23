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
  add r2 r2 0
  ldw r1 0 r2
  add r3 rfp -8
  add r3 r3 0
  ldw r2 0 r3
  sub r1 r1 r2
  cmpu r1 r1 0
  and r1 r1 1
  jnz r1 &_Lx0
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
