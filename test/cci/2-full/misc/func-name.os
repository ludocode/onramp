@_Sx0
  "foo"
  '00
@_Sx1
  "foo"
  '00
=foo
  enter
  imw r0 ^_Sx0
  add r0 rpp r0
  imw r1 ^_Sx1
  add r1 rpp r1
  call ^strcmp
  mov r0 r0
  leave
  ret
@_Sx2
  "bar"
  '00
=bar
  enter
  imw r0 ^_Sx2
  add r0 rpp r0
  imw r1 ^_Sx2
  add r1 rpp r1
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  leave
  ret
@_Sx3
  "baz"
  '00
=baz
  enter
  imw r0 ^_Sx3
  add r0 rpp r0
  imw r1 ^_Sx3
  add r1 rpp r1
  call ^strcmp
  mov r0 r0
  leave
  ret
=main
  enter
  call ^foo
  mov r0 r0
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  mov r0 1
  leave
  ret
  jmp &_Lx1
:_Lx1
  call ^bar
  mov r0 r0
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  mov r0 2
  leave
  ret
  jmp &_Lx3
:_Lx3
  call ^baz
  mov r0 r0
  jnz r0 &_Lx4
  jmp &_Lx5
:_Lx4
  mov r0 3
  leave
  ret
  jmp &_Lx5
:_Lx5
  zero r0
  leave
  ret
