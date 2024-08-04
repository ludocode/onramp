=foo
  enter
  sub rsp rsp 4
  stw r0 rfp -4
  mov r0 0
  leave
  ret
=main
  enter
  mov r0 0
  call ^foo
  mov r0 r0
  leave
  ret
