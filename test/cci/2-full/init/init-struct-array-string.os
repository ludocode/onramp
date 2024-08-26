@_Sx0
  "Hello"
  '00
@_Sx1
  "Hello"
  '00
=main
  enter
  sub rsp rsp 16
  add r0 rfp -16
  mov r1 1
  stw r1 r0 0
  imw r1 ^_Sx0
  add r1 rpp r1
  imw r2 4
  add r2 r2 r0
  imw r4 5
  jmp &_Lx0
:_Lx0
  jz r4 &_Lx1
  sub r4 r4 1
  ldb r3 r1 r4
  stb r3 r2 r4
  jmp &_Lx0
:_Lx1
  mov r1 2
  stw r1 r0 12
  add r2 rfp -16
  ldw r0 0 r2
  mov r1 1
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  mov r0 1
  leave
  ret
  jmp &_Lx3
:_Lx3
  mov r0 0
  push r0
  add r0 rfp -16
  add r0 r0 4
  imw r1 ^_Sx1
  add r1 rpp r1
  mov r2 5
  call ^memcmp
  mov r1 r0
  pop r0
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx4
  jmp &_Lx5
:_Lx4
  mov r0 2
  leave
  ret
  jmp &_Lx5
:_Lx5
  add r2 rfp -16
  add r2 r2 12
  ldw r0 0 r2
  mov r1 2
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx6
  jmp &_Lx7
:_Lx6
  mov r0 3
  leave
  ret
  jmp &_Lx7
:_Lx7
  zero r0
  leave
  ret
