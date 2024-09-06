=foo
  enter
  sub rsp rsp 4
  stw r0 rfp -4
  imw r0 0
  leave
  ret
=main
  enter
  imw r0 0
  call ^foo
  mov r0 r0
  leave
  ret
