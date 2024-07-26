=main
  enter
  sub rsp rsp 8
  mov r0 1
  imw r1 ^b
  add r1 rpp r1
  add r1 r1 0
  stw r0 0 r1
  imw r2 ^b
  add r2 rpp r2
  ldw r0 0 r2
  mov r1 1
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
  mov r0 1
  add r1 rfp -8
  add r1 r1 0
  stw r0 0 r1
  mov r0 2
  add r1 rfp -8
  add r1 r1 4
  stw r0 0 r1
  mov r0 3
  add r1 rfp -8
  add r1 r1 0
  stw r0 0 r1
  mov r0 4
  add r1 rfp -8
  add r1 r1 0
  stw r0 0 r1
  add r2 rfp -8
  ldw r0 0 r2
  mov r1 4
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  mov r0 1
  leave
  ret
  jmp &_Lx3
:_Lx3
  add r2 rfp -8
  add r2 r2 4
  ldw r0 0 r2
  mov r1 2
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx4
  jmp &_Lx5
:_Lx4
  mov r0 2
  leave
  ret
  jmp &_Lx5
:_Lx5
  add r2 rfp -8
  ldw r0 0 r2
  mov r1 4
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx6
  jmp &_Lx7
:_Lx6
  mov r0 3
  leave
  ret
  jmp &_Lx7
:_Lx7
  add r2 rfp -8
  ldw r0 0 r2
  mov r1 4
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx8
  jmp &_Lx9
:_Lx8
  mov r0 4
  leave
  ret
  jmp &_Lx9
:_Lx9
  mov r0 0
  leave
  ret
=b
  0
