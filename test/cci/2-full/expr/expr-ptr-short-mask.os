@_Sx0
  "a"
  '00
=main
  enter
  sub rsp rsp 8
  add r0 rfp -2
  imw r1 3
  sts r1 r0 0
  add r0 rfp -8
  imw r1 ^_Sx0
  add r1 rpp r1
  stw r1 r0 0
  add r1 rfp -8
  ldw r0 0 r1
  ldw r0 rfp -8
  lds r2 rfp -2
  sxs r2 r2
  xor r0 r0 r2
  stw r0 r1 0
  add r1 rfp -8
  ldw r0 0 r1
  ldw r0 rfp -8
  imw r2 5
  add r3 rfp -2
  sts r2 r3 0
  sxs r2 r2
  xor r0 r0 r2
  stw r0 r1 0
  add r1 rfp -8
  ldw r0 0 r1
  ldw r0 rfp -8
  imw r2 6
  xor r0 r0 r2
  stw r0 r1 0
  ldw r0 rfp -8
  ldb r0 0 r0
  trb r0 r0
  mov r1 97
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  imw r0 1
  leave
  ret
  jmp &_Lx1
:_Lx1
  zero r0
  leave
  ret
