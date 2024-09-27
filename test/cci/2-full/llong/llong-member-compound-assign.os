=main
  enter
  sub rsp rsp 4
  sub rsp rsp 8
  mov r0 rsp
  imw r1 ^f
  add r1 rpp r1
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  push r1
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 ^f
  add r2 rpp r2
  ldw r3 r2 0
  stw r3 r1 0
  ldw r3 r2 4
  stw r3 r1 4
  sub rsp rsp 8
  mov r2 rsp
  imw r3 1
  stw r3 r2 0
  shru r3 r3 31
  sub r3 0 r3
  stw r3 r2 4
  call ^__llong_add
  mov r0 r0
  add rsp rsp 16
  pop r1
  ldw r2 r0 0
  stw r2 r1 0
  ldw r2 r0 4
  stw r2 r1 4
  add rsp rsp 8
  sub rsp rsp 16
  mov r0 rsp
  imw r1 ^f
  add r1 rpp r1
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  add r1 rsp 8
  imw r2 1
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
  add r0 rfp -4
  imw r1 ^f
  add r1 rpp r1
  stw r1 r0 0
  sub rsp rsp 8
  mov r0 rsp
  ldw r1 rfp -4
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  push r1
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  ldw r2 rfp -4
  ldw r3 r2 0
  stw r3 r1 0
  ldw r3 r2 4
  stw r3 r1 4
  sub rsp rsp 8
  mov r2 rsp
  imw r3 1
  stw r3 r2 0
  shru r3 r3 31
  sub r3 0 r3
  stw r3 r2 4
  call ^__llong_add
  mov r0 r0
  add rsp rsp 16
  pop r1
  ldw r2 r0 0
  stw r2 r1 0
  ldw r2 r0 4
  stw r2 r1 4
  add rsp rsp 8
  sub rsp rsp 16
  mov r0 rsp
  ldw r1 rfp -4
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  add r1 rsp 8
  imw r2 2
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
=f 0
  0
