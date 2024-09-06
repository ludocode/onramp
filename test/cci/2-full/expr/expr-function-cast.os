=foo
  enter
  imw r0 5
  leave
  ret
=main
  enter
  sub rsp rsp 8
  imw r0 5
  push r0
  imw r0 ^foo
  add r0 rpp r0
  call r0
  mov r1 r0
  pop r0
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
  imw r0 5
  push r0
  imw r0 ^foo
  add r0 rpp r0
  call r0
  mov r1 r0
  pop r0
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
  add r0 rfp -4
  imw r1 ^foo
  add r1 rpp r1
  stw r1 r0 0
  add r0 rfp -8
  ldw r1 rfp -4
  stw r1 r0 0
  imw r0 5
  push r0
  ldw r0 rfp -8
  call r0
  mov r1 r0
  pop r0
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
  imw r0 ^foo
  add r0 rpp r0
  call r0
  mov r0 r0
  zero r0
  leave
  ret
