=main
  enter
  sub rsp rsp 12
  add r0 rfp -4
  mov r1 1
  stw r1 r0 0
  mov r0 1
  add r2 rfp -4
  mov r3 0
  shl r3 r3 2
  add r2 r2 r3
  ldw r1 0 r2
  sub r0 r0 r1
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  mov r0 1
  leave
  ret
  jmp &_Lx1
:_Lx1
  add r0 rfp -12
  mov r1 1
  stw r1 r0 0
  mov r1 2
  stw r1 r0 4
  mov r0 3
  add r2 rfp -12
  mov r3 0
  shl r3 r3 2
  add r2 r2 r3
  ldw r1 0 r2
  sub r0 r0 r1
  add r2 rfp -12
  mov r3 1
  shl r3 r3 2
  add r2 r2 r3
  ldw r1 0 r2
  sub r0 r0 r1
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
