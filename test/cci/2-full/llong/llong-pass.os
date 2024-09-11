=max
  enter
  ldw r0 rfp 8
  sub rsp rsp 16
  mov r1 rsp
  add r2 rfp 20
  ldw r3 r2 0
  stw r3 r1 0
  ldw r3 r2 4
  stw r3 r1 4
  add r2 rsp 8
  add r3 rfp 12
  ldw r4 r3 0
  stw r4 r2 0
  ldw r4 r3 4
  stw r4 r2 4
  ldw r1 rsp 0
  ldw r3 rsp 8
  lts r1 r1 r3
  ldw r2 rsp 4
  ldw r4 rsp 12
  sub r3 r2 r4
  isz r3 r3
  and r1 r1 r3
  lts r2 r2 r4
  or r1 r1 r2
  add rsp rsp 16
  jnz r1 &_Lx0
  jmp &_Lx1
:_Lx0
  add r1 rfp 12
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  jmp &_Lx2
:_Lx1
  add r1 rfp 20
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  jmp &_Lx2
:_Lx2
  leave
  ret
=main
  enter
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  sub rsp rsp 8
  mov r1 rsp
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  push r0
  call ^max
  mov r0 r0
  add rsp rsp 20
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 0
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
  jnz r0 &_Lx3
  jmp &_Lx4
:_Lx3
  imw r0 1
  leave
  ret
  jmp &_Lx4
:_Lx4
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 -2
  stw r2 r1 0
  imw r2 2147483647
  stw r2 r1 4
  sub rsp rsp 8
  mov r1 rsp
  imw r2 -1
  stw r2 r1 0
  imw r2 2147483647
  stw r2 r1 4
  push r0
  call ^max
  mov r0 r0
  add rsp rsp 20
  add r1 rsp 8
  imw r2 -1
  stw r2 r1 0
  imw r2 2147483647
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
  jnz r0 &_Lx5
  jmp &_Lx6
:_Lx5
  imw r0 1
  leave
  ret
  jmp &_Lx6
:_Lx6
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  push r0
  mov r0 r1
  sub rsp rsp 8
  mov r1 rsp
  push r0
  mov r0 r1
  sub rsp rsp 8
  mov r1 rsp
  imw r2 -1
  stw r2 r1 0
  imw r2 2147483647
  stw r2 r1 4
  call ^__llong_negate
  mov r1 r0
  add rsp rsp 8
  pop r0
  sub rsp rsp 8
  mov r2 rsp
  imw r3 1
  stw r3 r2 0
  shru r3 r3 31
  sub r3 0 r3
  stw r3 r2 4
  call ^__llong_sub
  mov r1 r0
  add rsp rsp 16
  pop r0
  sub rsp rsp 8
  mov r1 rsp
  push r0
  mov r0 r1
  sub rsp rsp 8
  mov r1 rsp
  imw r2 -1
  stw r2 r1 0
  imw r2 2147483647
  stw r2 r1 4
  call ^__llong_negate
  mov r1 r0
  add rsp rsp 8
  pop r0
  push r0
  call ^max
  mov r0 r0
  add rsp rsp 20
  add r1 rsp 8
  push r0
  mov r0 r1
  sub rsp rsp 8
  mov r1 rsp
  imw r2 -1
  stw r2 r1 0
  imw r2 2147483647
  stw r2 r1 4
  call ^__llong_negate
  mov r1 r0
  add rsp rsp 8
  pop r0
  ldw r0 rsp 0
  ldw r2 rsp 8
  sub r0 r0 r2
  ldw r1 rsp 4
  ldw r2 rsp 12
  sub r1 r1 r2
  or r0 r0 r1
  add rsp rsp 16
  bool r0 r0
  jnz r0 &_Lx7
  jmp &_Lx8
:_Lx7
  imw r0 1
  leave
  ret
  jmp &_Lx8
:_Lx8
  imw r0 0
  leave
  ret
