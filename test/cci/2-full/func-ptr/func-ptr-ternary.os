=compare
  enter
  sub rsp rsp 12
  stw r0 rfp -4
  stw r1 rfp -8
  stw r2 rfp -12
  ldw r0 rfp -8
  ldw r1 rfp -12
  ldb r2 rfp -4
  trb r2 r2
  jnz r2 &_Lx0
  jmp &_Lx1
:_Lx0
  imw r2 ^strcmp
  add r2 rpp r2
  jmp &_Lx2
:_Lx1
  imw r2 ^strcasecmp
  add r2 rpp r2
  jmp &_Lx2
:_Lx2
  call r2
  mov r0 r0
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
  imw r0 1
  bool r0 r0
  imw r1 ^_Sx0
  add r1 rpp r1
  imw r2 ^_Sx1
  add r2 rpp r2
  call ^compare
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
  imw r0 0
  bool r0 r0
  imw r1 ^_Sx2
  add r1 rpp r1
  imw r2 ^_Sx3
  add r2 rpp r2
  call ^compare
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
