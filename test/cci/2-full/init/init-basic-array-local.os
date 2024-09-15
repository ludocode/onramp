=main
  enter
  sub rsp rsp 12
  add r0 rfp -12
  imw r1 1
  stw r1 r0 0
  imw r1 2
  stw r1 r0 4
  imw r1 3
  stw r1 r0 8
  imw r0 6
  add r1 rfp -12
  imw r2 0
  shl r2 r2 2
  add r1 r1 r2
  ldw r1 0 r1
  sub r0 r0 r1
  add r1 rfp -12
  imw r2 1
  shl r2 r2 2
  add r1 r1 r2
  ldw r1 0 r1
  sub r0 r0 r1
  add r1 rfp -12
  imw r2 2
  shl r2 r2 2
  add r1 r1 r2
  ldw r1 0 r1
  sub r0 r0 r1
  leave
  ret
