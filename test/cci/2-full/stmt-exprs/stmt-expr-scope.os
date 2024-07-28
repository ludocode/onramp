=main
  enter
  sub rsp rsp 8
  mov r2 3
  add r3 rfp -8
  stw r2 0 r3
  ldw r1 rfp -8
  add r2 rfp -4
  stw r1 0 r2
  mov r0 5
  ldw r1 rfp -4
  sub r0 r0 r1
  mov r1 2
  sub r0 r0 r1
  leave
  ret
