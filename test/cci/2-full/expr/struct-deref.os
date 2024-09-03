=b 0
  0
@{50_Ix0_b
  enter
  imw r0 ^b
  add r0 rpp r0
  mov r1 2
  stw r1 r0 0
  mov r1 3
  stw r1 r0 4
  leave
  ret
=c
  0
@{50_Ix1_c
  enter
  imw r0 ^c
  add r0 rpp r0
  imw r1 ^b
  add r1 rpp r1
  stw r1 r0 0
  leave
  ret
=main
  enter
  sub rsp rsp 8
  mov r0 rsp
  imw r1 ^c
  ldw r1 rpp r1
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  imw r1 ^a
  add r1 rpp r1
  ldw r2 r0 0
  stw r2 r1 0
  ldw r2 r0 4
  stw r2 r1 4
  add rsp rsp 8
  imw r2 ^a
  add r2 rpp r2
  ldw r0 0 r2
  mov r1 2
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
  imw r2 ^a
  add r2 rpp r2
  add r2 r2 4
  ldw r0 0 r2
  mov r1 3
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
=a 0
  0
