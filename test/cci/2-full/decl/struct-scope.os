=main
  enter
  sub rsp rsp 8
  imw r0 0
  add r1 rfp -8
  add r1 r1 4
  stw r0 r1 0
  imw r0 1
  add r1 rfp -8
  add r1 r1 0
  stw r0 r1 0
  add r0 rfp -8
  add r0 r0 4
  ldw r0 0 r0
  leave
  ret
