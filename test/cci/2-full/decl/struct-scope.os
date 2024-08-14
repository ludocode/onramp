=main
  enter
  sub rsp rsp 8
  mov r0 0
  add r1 rfp -8
  add r1 r1 4
  stw r0 r1 0
  mov r0 1
  add r1 rfp -8
  add r1 r1 0
  stw r0 r1 0
  add r1 rfp -8
  add r1 r1 4
  ldw r0 0 r1
  leave
  ret
