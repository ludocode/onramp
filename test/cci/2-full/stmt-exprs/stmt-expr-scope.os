=main
  enter
  sub rsp rsp 8
  add r0 rfp -4
  add r1 rfp -8
  imw r2 7
  stw r2 r1 0
  imw r1 3
  stw r1 r0 0
  imw r0 5
  ldw r1 rfp -4
  sub r0 r0 r1
  imw r1 2
  sub r0 r0 r1
  leave
  ret
