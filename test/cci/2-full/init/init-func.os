=main
  enter
  sub rsp rsp 32
  add r0 rfp -4
  imw r1 ^main
  add r1 rpp r1
  stw r1 r0 0
  add r0 rfp -8
  imw r1 ^main
  add r1 rpp r1
  stw r1 r0 0
  ldw r0 rfp -4
  imw r1 ^main
  add r1 rpp r1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  imw r0 1
  leave
  ret
  jmp &_Lx1
:_Lx1
  ldw r0 rfp -8
  imw r1 ^main
  add r1 rpp r1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  imw r0 2
  leave
  ret
  jmp &_Lx3
:_Lx3
  add r0 rfp -12
  imw r1 ^main
  add r1 rpp r1
  stw r1 r0 0
  add r0 rfp -16
  imw r1 ^main
  add r1 rpp r1
  stw r1 r0 0
  add r1 rfp -12
  ldw r0 0 r1
  imw r1 ^main
  add r1 rpp r1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx4
  jmp &_Lx5
:_Lx4
  imw r0 3
  leave
  ret
  jmp &_Lx5
:_Lx5
  add r1 rfp -16
  ldw r0 0 r1
  imw r1 ^main
  add r1 rpp r1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx6
  jmp &_Lx7
:_Lx6
  imw r0 4
  leave
  ret
  jmp &_Lx7
:_Lx7
  add r0 rfp -32
  imw r1 ^main
  add r1 rpp r1
  stw r1 r0 0
  imw r1 ^main
  add r1 rpp r1
  stw r1 r0 4
  imw r1 ^main
  add r1 rpp r1
  stw r1 r0 8
  imw r1 ^main
  add r1 rpp r1
  stw r1 r0 12
  add r1 rfp -32
  imw r2 0
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 ^main
  add r1 rpp r1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx8
  jmp &_Lx9
:_Lx8
  imw r0 5
  leave
  ret
  jmp &_Lx9
:_Lx9
  add r1 rfp -32
  imw r2 1
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 ^main
  add r1 rpp r1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxA
  jmp &_LxB
:_LxA
  imw r0 6
  leave
  ret
  jmp &_LxB
:_LxB
  add r1 rfp -32
  imw r2 2
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 ^main
  add r1 rpp r1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxC
  jmp &_LxD
:_LxC
  imw r0 7
  leave
  ret
  jmp &_LxD
:_LxD
  add r1 rfp -32
  imw r2 3
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 ^main
  add r1 rpp r1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxE
  jmp &_LxF
:_LxE
  imw r0 8
  leave
  ret
  jmp &_LxF
:_LxF
  zero r0
  leave
  ret
