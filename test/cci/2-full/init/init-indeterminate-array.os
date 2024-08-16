=main
  enter
  sub rsp rsp 12
  add r0 rfp -12
  mov r1 1
  stw r1 r0 0
  mov r1 2
  stw r1 r0 4
  mov r1 3
  stw r1 r0 8
  mov r0 12
  mov r1 3
  mov r2 4
  mul r1 r1 r2
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
  mov r0 6
  add r2 rfp -12
  mov r3 0
  shl r3 r3 2
  add r2 r2 r3
  ldw r1 0 r2
  add r3 rfp -12
  mov r4 1
  shl r4 r4 2
  add r3 r3 r4
  ldw r2 0 r3
  add r1 r1 r2
  add r3 rfp -12
  mov r4 2
  shl r4 r4 2
  add r3 r3 r4
  ldw r2 0 r3
  add r1 r1 r2
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
