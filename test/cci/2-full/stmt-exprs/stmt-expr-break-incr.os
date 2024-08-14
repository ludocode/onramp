=main
  enter
  sub rsp rsp 4
  add r0 rfp -4
  mov r1 0
  stw r1 r0 0
  jmp &_Lx0
:_Lx1
  jmp &_Lx0
:_Lx0
  jmp &_Lx3
:_Lx2
  ldw r0 rfp -4
  mov r1 1
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx6
  jmp &_Lx7
:_Lx4
  jmp &_Lx2
  jmp &_Lx3
:_Lx3
  add r1 rfp -4
  ldw r0 0 r1
  add r0 r0 1
  stw r0 r1 0
  jmp &_Lx4
:_Lx5
  mov r0 1
  leave
  ret
  jmp &_Lx1
:_Lx6
  mov r0 1
  leave
  ret
  jmp &_Lx7
:_Lx7
  zero r0
  leave
  ret
