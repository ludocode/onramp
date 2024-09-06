=main
  enter
  sub rsp rsp 16
  add r0 rfp -16
  imw r1 1
  stw r1 r0 0
  imw r1 2
  stw r1 r0 4
  imw r1 4
  stw r1 r0 8
  imw r1 8
  stw r1 r0 12
  imw r0 15
  add r2 rfp -16
  ldw r1 0 r2
  sub r0 r0 r1
  add r2 rfp -16
  add r2 r2 4
  ldw r1 0 r2
  sub r0 r0 r1
  add r2 rfp -16
  add r2 r2 4
  add r2 r2 4
  ldw r1 0 r2
  sub r0 r0 r1
  add r2 rfp -16
  add r2 r2 12
  ldw r1 0 r2
  sub r0 r0 r1
  leave
  ret
