=x
  enter
  ldw r0 rfp 8
  imw r1 ^a
  add r1 rpp r1
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  leave
  ret
=main
  enter
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  push r0
  call ^x
  mov r0 r0
  add rsp rsp 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  shru r2 r2 31
  sub r2 0 r2
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  sub r0 r0 r2
  ldw r1 rsp 4
  ldw r2 rsp 12
  sub r1 r1 r2
  or r0 r0 r1
  add rsp rsp 16
  bool r0 r0
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  imw r0 1
  leave
  ret
  jmp &_Lx1
:_Lx1
  sub rsp rsp 8
  mov r0 rsp
  imw r1 4
  stw r1 r0 0
  shru r1 r1 31
  sub r1 0 r1
  stw r1 r0 4
  imw r1 ^a
  add r1 rpp r1
  ldw r2 r0 0
  stw r2 r1 0
  ldw r2 r0 4
  stw r2 r1 4
  add rsp rsp 8
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 4
  stw r2 r1 0
  shru r2 r2 31
  sub r2 0 r2
  stw r2 r1 4
  sub rsp rsp 8
  mov r2 rsp
  push r0
  push r1
  mov r0 r2
  push r0
  call ^x
  mov r2 r0
  add rsp rsp 4
  pop r1
  pop r0
  call ^__llong_sub
  mov r0 r0
  add rsp rsp 16
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  shru r2 r2 31
  sub r2 0 r2
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  sub r0 r0 r2
  ldw r1 rsp 4
  ldw r2 rsp 12
  sub r1 r1 r2
  or r0 r0 r1
  add rsp rsp 16
  bool r0 r0
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  imw r0 2
  leave
  ret
  jmp &_Lx3
:_Lx3
  zero r0
  leave
  ret
=a 0
  0
