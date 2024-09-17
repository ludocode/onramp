=main
  enter
  sub rsp rsp 4
  imw r0 0
  add r1 rfp -4
  stw r0 r1 0
  add r0 rfp -4
  ldw r0 0 r0
  leave
  ret
