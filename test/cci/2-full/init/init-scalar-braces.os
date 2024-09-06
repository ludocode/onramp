=main
  enter
  sub rsp rsp 4
  add r0 rfp -4
  imw r1 4
  stw r1 r0 0
  zero r0
  leave
  ret
