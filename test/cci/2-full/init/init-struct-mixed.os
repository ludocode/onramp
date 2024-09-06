=foo
  enter
  sub rsp rsp 16
  stw r0 rfp -4
  stw r1 rfp -8
  add r0 rfp -16
  ldw r1 rfp -4
  stw r1 r0 0
  ldw r1 rfp -8
  stw r1 r0 4
  ldw r0 rfp 8
  add r1 rfp -16
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  leave
  ret
=main
  enter
  sub rsp rsp 64
  add r0 rfp -64
  imw r1 1
  stw r1 r0 0
  imw r1 2
  stw r1 r0 4
  imw r1 8
  add r1 r1 r0
  push r0
  mov r0 r1
  push r0
  imw r0 3
  imw r1 4
  call ^foo
  mov r1 r0
  add rsp rsp 4
  pop r0
  imw r1 5
  stw r1 r0 16
  imw r1 6
  stw r1 r0 20
  imw r1 24
  add r1 r1 r0
  push r0
  mov r0 r1
  push r0
  imw r0 7
  imw r1 8
  call ^foo
  mov r1 r0
  add rsp rsp 4
  pop r0
  imw r1 9
  stw r1 r0 32
  imw r1 10
  stw r1 r0 36
  imw r1 11
  stw r1 r0 40
  imw r1 44
  add r1 r1 r0
  stb 0 r1 0
  imw r1 48
  add r1 r1 r0
  stb 0 r1 0
  imw r1 15
  stw r1 r0 56
  imw r1 60
  add r1 r1 r0
  stb 0 r1 0
  add r1 rfp -64
  imw r2 0
  shl r2 r2 3
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 1
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
  add r1 rfp -64
  imw r2 0
  shl r2 r2 3
  add r1 r1 r2
  add r1 r1 4
  ldw r0 0 r1
  imw r1 2
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
  add r1 rfp -64
  imw r2 1
  shl r2 r2 3
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 3
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
  add r1 rfp -64
  imw r2 1
  shl r2 r2 3
  add r1 r1 r2
  add r1 r1 4
  ldw r0 0 r1
  imw r1 4
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
  add r1 rfp -64
  imw r2 2
  shl r2 r2 3
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 5
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
  add r1 rfp -64
  imw r2 2
  shl r2 r2 3
  add r1 r1 r2
  add r1 r1 4
  ldw r0 0 r1
  imw r1 6
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
  add r1 rfp -64
  imw r2 3
  shl r2 r2 3
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 7
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
  add r1 rfp -64
  imw r2 3
  shl r2 r2 3
  add r1 r1 r2
  add r1 r1 4
  ldw r0 0 r1
  imw r1 8
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
  add r1 rfp -64
  imw r2 4
  shl r2 r2 3
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 9
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx10
  jmp &_Lx11
:_Lx10
  imw r0 9
  leave
  ret
  jmp &_Lx11
:_Lx11
  add r1 rfp -64
  imw r2 4
  shl r2 r2 3
  add r1 r1 r2
  add r1 r1 4
  ldw r0 0 r1
  imw r1 10
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx12
  jmp &_Lx13
:_Lx12
  imw r0 10
  leave
  ret
  jmp &_Lx13
:_Lx13
  zero r0
  leave
  ret
