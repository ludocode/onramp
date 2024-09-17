=a 0 0
  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
  0
@{50_Ix0_a
  enter
  imw r0 ^a
  add r0 rpp r0
  imw r1 1
  stw r1 r0 0
  imw r1 4
  add r1 r1 r0
  stw 0 r1 0
  stw 0 r1 4
  stw 0 r1 8
  stw 0 r1 12
  stw 0 r1 16
  stw 0 r1 20
  stw 0 r1 24
  stw 0 r1 28
  imw r1 2
  imw r2 132
  add r2 r2 r0
  stw r1 r2 0
  imw r1 3
  imw r2 136
  add r2 r2 r0
  stw r1 r2 0
  leave
  ret
=main
  enter
  sub rsp rsp 4
  imw r0 ^a
  add r0 rpp r0
  mov r0 r0
  imw r1 0
  shl r1 r1 2
  add r0 r0 r1
  ldw r0 0 r0
  imw r1 1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx1
  jmp &_Lx2
:_Lx1
  imw r0 1
  leave
  ret
  jmp &_Lx2
:_Lx2
  imw r0 ^a
  add r0 rpp r0
  mov r0 r0
  imw r1 1
  shl r1 r1 2
  add r0 r0 r1
  ldw r0 0 r0
  imw r1 0
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx3
  jmp &_Lx4
:_Lx3
  imw r0 2
  leave
  ret
  jmp &_Lx4
:_Lx4
  imw r0 ^a
  add r0 rpp r0
  imw r1 132
  add r0 r0 r1
  ldw r0 0 r0
  imw r1 2
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx5
  jmp &_Lx6
:_Lx5
  imw r0 3
  leave
  ret
  jmp &_Lx6
:_Lx6
  imw r0 ^a
  add r0 rpp r0
  imw r1 136
  add r0 r0 r1
  ldw r0 0 r0
  imw r1 3
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx7
  jmp &_Lx8
:_Lx7
  imw r0 4
  leave
  ret
  jmp &_Lx8
:_Lx8
  add r0 rfp -4
  imw r1 ^a
  add r1 rpp r1
  stw r1 r0 0
  ldw r0 rfp -4
  mov r0 r0
  imw r1 0
  shl r1 r1 2
  add r0 r0 r1
  ldw r0 0 r0
  imw r1 1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx9
  jmp &_LxA
:_Lx9
  imw r0 5
  leave
  ret
  jmp &_LxA
:_LxA
  ldw r0 rfp -4
  mov r0 r0
  imw r1 1
  shl r1 r1 2
  add r0 r0 r1
  ldw r0 0 r0
  imw r1 0
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxB
  jmp &_LxC
:_LxB
  imw r0 6
  leave
  ret
  jmp &_LxC
:_LxC
  ldw r0 rfp -4
  imw r1 132
  add r0 r0 r1
  ldw r0 0 r0
  imw r1 2
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxD
  jmp &_LxE
:_LxD
  imw r0 7
  leave
  ret
  jmp &_LxE
:_LxE
  ldw r0 rfp -4
  imw r1 136
  add r0 r0 r1
  ldw r0 0 r0
  imw r1 3
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxF
  jmp &_Lx10
:_LxF
  imw r0 8
  leave
  ret
  jmp &_Lx10
:_Lx10
  ldw r0 rfp -4
  mov r0 r0
  imw r1 0
  shl r1 r1 2
  add r0 r0 r1
  ldw r0 0 r0
  imw r1 1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx11
  jmp &_Lx12
:_Lx11
  imw r0 9
  leave
  ret
  jmp &_Lx12
:_Lx12
  ldw r0 rfp -4
  mov r0 r0
  imw r1 1
  shl r1 r1 2
  add r0 r0 r1
  ldw r0 0 r0
  imw r1 0
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx13
  jmp &_Lx14
:_Lx13
  imw r0 10
  leave
  ret
  jmp &_Lx14
:_Lx14
  ldw r0 rfp -4
  imw r1 132
  add r0 r0 r1
  ldw r0 0 r0
  imw r1 2
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx15
  jmp &_Lx16
:_Lx15
  imw r0 11
  leave
  ret
  jmp &_Lx16
:_Lx16
  ldw r0 rfp -4
  imw r1 136
  add r0 r0 r1
  ldw r0 0 r0
  imw r1 3
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx17
  jmp &_Lx18
:_Lx17
  imw r0 12
  leave
  ret
  jmp &_Lx18
:_Lx18
  zero r0
  leave
  ret
