=main
  enter
  sub rsp rsp 4
  imw r0 ^main
  add r0 rpp r0
  imw r1 ^main
  add r1 rpp r1
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
  add r0 rfp -4
  imw r1 ^main
  add r1 rpp r1
  stw r1 r0 0
  ldw r0 rfp -4
  imw r1 ^main
  add r1 rpp r1
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
  ldw r0 rfp -4
  imw r1 ^main
  add r1 rpp r1
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx4
  jmp &_Lx5
:_Lx4
  mov r0 3
  leave
  ret
  jmp &_Lx5
:_Lx5
  ldw r0 rfp -4
  ldw r1 rfp -4
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx6
  jmp &_Lx7
:_Lx6
  mov r0 4
  leave
  ret
  jmp &_Lx7
:_Lx7
  add r0 rfp -4
  ldw r0 0 r0
  ldw r1 rfp -4
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx8
  jmp &_Lx9
:_Lx8
  mov r0 5
  leave
  ret
  jmp &_Lx9
:_Lx9
  mov r0 4
  mov r1 4
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_LxA
  jmp &_LxB
:_LxA
  mov r0 6
  leave
  ret
  jmp &_LxB
:_LxB
  mov r0 4
  mov r1 4
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_LxC
  jmp &_LxD
:_LxC
  mov r0 7
  leave
  ret
  jmp &_LxD
:_LxD
  zero r0
  leave
  ret
