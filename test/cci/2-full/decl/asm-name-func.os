@_Sx0
  "hello"
  '00
@_Sx1
  "hello"
  '00
@_Sx2
  "helzo"
  '00
@_Sx3
  "hello"
  '00
@_Sx4
  "bello"
  '00
@_Sx5
  "hello"
  '00
=main
  enter
  mov r1 0
  push r0
  push r1
  imw r0 ^_Sx0
  add r0 rpp r0
  imw r1 ^_Sx1
  add r1 rpp r1
  call ^strcmp
  mov r2 r0
  pop r1
  pop r0
  sub r1 r1 r2
  cmpu r1 r1 0
  and r1 r1 1
  jnz r1 &_Lx0
  jmp &_Lx1
:_Lx0
  mov r0 1
  leave
  ret
  jmp &_Lx1
:_Lx1
  mov r1 0
  push r0
  push r1
  imw r0 ^_Sx2
  add r0 rpp r0
  imw r1 ^_Sx3
  add r1 rpp r1
  call ^strcmp
  mov r2 r0
  pop r1
  pop r0
  cmps r1 r1 r2
  cmpu r1 r1 -1
  and r1 r1 1
  jnz r1 &_Lx2
  jmp &_Lx3
:_Lx2
  mov r0 1
  leave
  ret
  jmp &_Lx3
:_Lx3
  mov r1 0
  push r0
  push r1
  imw r0 ^_Sx4
  add r0 rpp r0
  imw r1 ^_Sx5
  add r1 rpp r1
  call ^strcmp
  mov r2 r0
  pop r1
  pop r0
  cmps r1 r1 r2
  cmpu r1 r1 1
  and r1 r1 1
  jnz r1 &_Lx4
  jmp &_Lx5
:_Lx4
  mov r0 1
  leave
  ret
  jmp &_Lx5
:_Lx5
  mov r0 0
  leave
  ret
