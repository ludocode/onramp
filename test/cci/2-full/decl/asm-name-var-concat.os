=main
  enter
  imw r0 ^test_test_test
  ldw r0 rpp r0
  mov r1 0
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
  imw r0 ^test_test_test
  ldw r0 rpp r0
  mov r1 0
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
  mov r0 4
  imw r1 ^test_test_test
  add r1 rpp r1
  stw r0 0 r1
  imw r0 ^test_test_test
  ldw r0 rpp r0
  mov r1 4
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx4
  jmp &_Lx5
:_Lx4
  mov r0 1
  leave
  ret
  jmp &_Lx5
:_Lx5
  imw r0 ^test_test_test
  ldw r0 rpp r0
  mov r1 4
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx6
  jmp &_Lx7
:_Lx6
  mov r0 1
  leave
  ret
  jmp &_Lx7
:_Lx7
  zero r0
  leave
  ret
=test_test_test
  0
