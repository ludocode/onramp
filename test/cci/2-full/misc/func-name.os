@_Sx0
  "foo"
  '00
@_Sx1
  "foo"
  '00
=foo
  enter
  imw r0 0
  push r0
  imw r0 ^_Sx0
  add r0 rpp r0
  imw r1 ^_Sx1
  add r1 rpp r1
  call ^strcmp
  mov r1 r0
  pop r0
  sub r0 r0 r1
  isz r0 r0
  bool r0 r0
  leave
  ret
@_Sx2
  "baz"
  '00
=baz
  enter
  imw r0 0
  push r0
  imw r0 ^_Sx2
  add r0 rpp r0
  imw r1 ^_Sx2
  add r1 rpp r1
  call ^strcmp
  mov r1 r0
  pop r0
  sub r0 r0 r1
  isz r0 r0
  bool r0 r0
  leave
  ret
@_Sx3
  "main"
  '00
=main
  enter
  call ^foo
  mov r0 r0
  trb r0 r0
  isz r0 r0
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  imw r0 1
  leave
  ret
  jmp &_Lx1
:_Lx1
  call ^baz
  mov r0 r0
  trb r0 r0
  isz r0 r0
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  imw r0 3
  leave
  ret
  jmp &_Lx3
:_Lx3
  imw r0 ^_Sx3
  add r0 rpp r0
  imw r1 ^_Sx3
  add r1 rpp r1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx4
  jmp &_Lx5
:_Lx4
  imw r0 0
  bool r0 r0
  trb r0 r0
  leave
  ret
  jmp &_Lx5
:_Lx5
  imw r0 ^_Sx3
  add r0 rpp r0
  imw r1 ^_Sx3
  add r1 rpp r1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx6
  jmp &_Lx7
:_Lx6
  imw r0 0
  bool r0 r0
  trb r0 r0
  leave
  ret
  jmp &_Lx7
:_Lx7
  imw r0 5
  imw r1 5
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx8
  jmp &_Lx9
:_Lx8
  imw r0 0
  bool r0 r0
  trb r0 r0
  leave
  ret
  jmp &_Lx9
:_Lx9
  zero r0
  leave
  ret
