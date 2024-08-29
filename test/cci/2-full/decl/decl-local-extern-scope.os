@x
  0
@{50_Ix0_x
  enter
  imw r0 ^x
  add r0 rpp r0
  mov r1 3
  stw r1 r0 0
  leave
  ret
=main
  enter
  sub rsp rsp 4
  add r0 rfp -4
  mov r1 7
  stw r1 r0 0
  mov r0 3
  imw r1 ^x
  ldw r1 rpp r1
  sub r0 r0 r1
  leave
  ret
