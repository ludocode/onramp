@_Mx0_x 0 0
  0
@{50_Ix1_x
  enter
  imw r0 ^_Mx0_x
  add r0 rpp r0
  imw r1 1
  stw r1 r0 0
  imw r1 2
  stw r1 r0 4
  imw r1 3
  stw r1 r0 8
  leave
  ret
=main
  enter
  imw r0 12
  imw r1 3
  imw r2 4
  mul r1 r1 r2
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  imw r0 1
  leave
  ret
  jmp &_Lx3
:_Lx3
  imw r0 6
  imw r1 ^_Mx0_x
  add r1 rpp r1
  imw r2 0
  shl r2 r2 2
  add r1 r1 r2
  ldw r1 0 r1
  imw r2 ^_Mx0_x
  add r2 rpp r2
  imw r3 1
  shl r3 r3 2
  add r2 r2 r3
  ldw r2 0 r2
  add r1 r1 r2
  imw r2 ^_Mx0_x
  add r2 rpp r2
  imw r3 2
  shl r3 r3 2
  add r2 r2 r3
  ldw r2 0 r2
  add r1 r1 r2
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx4
  jmp &_Lx5
:_Lx4
  imw r0 2
  leave
  ret
  jmp &_Lx5
:_Lx5
  zero r0
  leave
  ret
