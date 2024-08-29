=foo
  enter
  imw r0 ^x
  ldb r0 rpp r0
  leave
  ret
=main
  enter
  mov r0 1
  bool r0 r0
  imw r1 ^x
  add r1 rpp r1
  stb r0 r1 0
  call ^foo
  mov r0 r0
  isz r0 r0
  leave
  ret
@x
  0
