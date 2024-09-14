=x
  0
@{50_Ix0_x
  enter
  imw r0 ^x
  add r0 rpp r0
  imw r1 0
  stw r1 r0 0
  leave
  ret
=foo
  enter
  imw r1 ^x
  add r1 rpp r1
  ldw r0 0 r1
  add r0 r0 1
  stw r0 r1 0
  leave
  ret
=main
  enter
  imw r0 ^foo
  add r0 rpp r0
  imw r1 ^bar
  add r1 rpp r1
  stw r0 r1 0
  imw r1 ^baz
  add r1 rpp r1
  stw r0 r1 0
  call ^foo
  mov r0 r0
  imw r0 ^x
  ldw r0 rpp r0
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
  imw r0 ^bar
  ldw r0 rpp r0
  call r0
  mov r0 r0
  imw r0 ^x
  ldw r0 rpp r0
  imw r1 2
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
  imw r0 ^baz
  ldw r0 rpp r0
  call r0
  mov r0 r0
  imw r0 ^x
  ldw r0 rpp r0
  imw r1 3
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
  zero r0
  leave
  ret
=baz
  0
=bar
  0
