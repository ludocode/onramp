=compare
  enter
  sub rsp rsp 4
  stw r0 rfp -4
  ldb r0 rfp -4
  trb r0 r0
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  imw r0 ^strcmp
  add r0 rpp r0
  jmp &_Lx2
:_Lx1
  imw r0 ^strcasecmp
  add r0 rpp r0
  jmp &_Lx2
:_Lx2
  leave
  ret
@_Sx0
  "Hello"
  '00
@_Sx1
  "hello"
  '00
@_Sx2
  "Hello"
  '00
@_Sx3
  "hello"
  '00
=main
  enter
  imw r0 0
  push r0
  imw r0 ^_Sx0
  add r0 rpp r0
  imw r1 ^_Sx1
  add r1 rpp r1
  push r0
  push r1
  imw r0 1
  bool r0 r0
  call ^compare
  mov r2 r0
  pop r1
  pop r0
  call r2
  mov r1 r0
  pop r0
  sub r0 r0 r1
  isz r0 r0
  jnz r0 &_Lx3
  jmp &_Lx4
:_Lx3
  imw r0 1
  leave
  ret
  jmp &_Lx4
:_Lx4
  imw r0 0
  push r0
  imw r0 ^_Sx2
  add r0 rpp r0
  imw r1 ^_Sx3
  add r1 rpp r1
  push r0
  push r1
  imw r0 0
  bool r0 r0
  call ^compare
  mov r2 r0
  pop r1
  pop r0
  call r2
  mov r1 r0
  pop r0
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx5
  jmp &_Lx6
:_Lx5
  imw r0 2
  leave
  ret
  jmp &_Lx6
:_Lx6
  imw r0 0
  leave
  ret
