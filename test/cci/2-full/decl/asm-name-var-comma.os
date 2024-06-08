=main
  enter
  imw r1 ^b
  ldw r1 rpp r1
  mov r2 0
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
  imw r1 ^b
  ldw r1 rpp r1
  mov r2 0
  sub r1 r1 r2
  cmpu r1 r1 0
  and r1 r1 1
  jnz r1 &_Lx2
  jmp &_Lx3
:_Lx2
  mov r0 1
  leave
  ret
  jmp &_Lx3
:_Lx3
  imw r1 ^b
  ldw r1 rpp r1
  mov r2 0
  sub r1 r1 r2
  cmpu r1 r1 0
  and r1 r1 1
  jnz r1 &_Lx4
  jmp &_Lx5
:_Lx4
  mov r0 1
  leave
  ret
  jmp &_Lx5
:_Lx5
  mov r0 4
  imw r1 ^b
  add r1 rpp r1
  stw r0 0 r1
  imw r1 ^b
  ldw r1 rpp r1
  mov r2 4
  sub r1 r1 r2
  cmpu r1 r1 0
  and r1 r1 1
  jnz r1 &_Lx6
  jmp &_Lx7
:_Lx6
  mov r0 1
  leave
  ret
  jmp &_Lx7
:_Lx7
  imw r1 ^b
  ldw r1 rpp r1
  mov r2 4
  sub r1 r1 r2
  cmpu r1 r1 0
  and r1 r1 1
  jnz r1 &_Lx8
  jmp &_Lx9
:_Lx8
  mov r0 1
  leave
  ret
  jmp &_Lx9
:_Lx9
  imw r1 ^b
  ldw r1 rpp r1
  mov r2 4
  sub r1 r1 r2
  cmpu r1 r1 0
  and r1 r1 1
  jnz r1 &_LxA
  jmp &_LxB
:_LxA
  mov r0 1
  leave
  ret
  jmp &_LxB
:_LxB
  zero r0
  leave
  ret
=b
  0
