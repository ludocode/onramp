@_Mx0_x
  0
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
  mov r1 1
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx1
  jmp &_Lx2
:_Lx1
  mov r0 1
  leave
  ret
  jmp &_Lx2
:_Lx2
  call ^foo
  mov r0 r0
  mov r1 2
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx3
  jmp &_Lx4
:_Lx3
  mov r0 2
  leave
  ret
  jmp &_Lx4
:_Lx4
  call ^foo
  mov r0 r0
  mov r1 3
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx5
  jmp &_Lx6
:_Lx5
  mov r0 3
  leave
  ret
  jmp &_Lx6
:_Lx6
  zero r0
  leave
  ret
