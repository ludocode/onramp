=main
  enter
  sub rsp rsp 12
  add r0 rfp -4
  push r0
  call ^get_a
  mov r1 r0
  pop r0
  stw r1 r0 0
  add r0 rfp -8
  push r0
  ldw r0 rfp -4
  call ^decode_a
  mov r1 r0
  pop r0
  stw r1 r0 0
  ldw r0 rfp -12
  ldw r0 rfp -8
  leave
  ret
@get_a
  enter
  imw r0 ^a
  add r0 rpp r0
  leave
  ret
@decode_a
  enter
  sub rsp rsp 4
  stw r0 rfp -4
  ldw r0 rfp -4
  ldb r0 0 r0
  sxb r0 r0
  leave
  ret
=a
  0
