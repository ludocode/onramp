@_Mx0_x
  0
@{50_Ix1_x
  enter
  imw r0 ^_Mx0_x
  add r0 rpp r0
  mov r1 4
  stw r1 r0 0
  leave
  ret
@foo
  enter
  imw r1 ^_Mx0_x
  add r1 rpp r1
  ldw r0 0 r1
  add r0 r0 1
  stw r0 r1 0
  leave
  ret
=main
  enter
  call ^foo
  mov r0 r0
  mov r1 5
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
  call ^foo
  mov r0 r0
  mov r1 6
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
  call ^foo
  mov r0 r0
  mov r1 7
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
  zero r0
  leave
  ret
