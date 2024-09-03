@_Mx0_x 0 0
  0
@{50_Ix1_x
  enter
  imw r0 ^_Mx0_x
  add r0 rpp r0
  mov r1 1
  stw r1 r0 0
  mov r1 2
  stw r1 r0 4
  mov r1 3
  stw r1 r0 8
  leave
  ret
=main
  enter
  mov r0 12
  mov r1 3
  mov r2 4
  mul r1 r1 r2
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
  mov r0 6
  imw r2 ^_Mx0_x
  add r2 rpp r2
  mov r3 0
  shl r3 r3 2
  add r2 r2 r3
  ldw r1 0 r2
  imw r3 ^_Mx0_x
  add r3 rpp r3
  mov r4 1
  shl r4 r4 2
  add r3 r3 r4
  ldw r2 0 r3
  add r1 r1 r2
  imw r3 ^_Mx0_x
  add r3 rpp r3
  mov r4 2
  shl r4 r4 2
  add r3 r3 r4
  ldw r2 0 r3
  add r1 r1 r2
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
  zero r0
  leave
  ret
