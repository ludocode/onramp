@_F_foo 
  stw r0 rfp -4 
  stw r1 rfp -8 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  sub r0 0 r0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  add r0 rfp -8 
  push r0 
  imw r0 2 
  sub r0 0 r0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 2 
  leave 
  ret 
:_Lx1 
  add r0 rfp -12 
  mov r1 r0 
  add r0 rfp 8 
  stw r0 0 r1 
  imw r0 ^_Sx0 
  add r0 rpp r0 
  push r0 
  add r0 rfp -12 
  mov r1 r0 
  ldw r0 0 r0 
  ldw r0 0 r0 
  push r0 
  mov r0 r1 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  shl r0 r0 2 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  pop r0 
  push r0 
  ldw r0 rsp 4 
  ldw r1 rsp 0 
  call ^strcmp 
  add rsp rsp 8 
  push r0 
  imw r0 0 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 3 
  leave 
  ret 
:_Lx2 
  imw r0 ^_Sx1 
  add r0 rpp r0 
  push r0 
  add r0 rfp -12 
  mov r1 r0 
  ldw r0 0 r0 
  ldw r0 0 r0 
  push r0 
  mov r0 r1 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  shl r0 r0 2 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  pop r0 
  push r0 
  ldw r0 rsp 4 
  ldw r1 rsp 0 
  call ^strcmp 
  add rsp rsp 8 
  push r0 
  imw r0 0 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
  imw r0 4 
  leave 
  ret 
:_Lx3 
  add r0 rfp -12 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=foo 
  enter 
  sub rsp rsp 12 
  jmp ^_F_foo 
@_Sx0 
"hello"'00
@_Sx1 
"goodbye"'00
@_F_main 
  imw r0 1 
  sub r0 0 r0 
  push r0 
  imw r0 2 
  sub r0 0 r0 
  push r0 
  imw r0 ^_Sx2 
  add r0 rpp r0 
  push r0 
  imw r0 ^_Sx3 
  add r0 rpp r0 
  push r0 
  ldw r0 rsp 12 
  ldw r1 rsp 8 
  ldw r9 rsp 0 
  push r9 
  ldw r9 rsp 8 
  push r9 
  call ^foo 
  add rsp rsp 24 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
@_Sx2 
"hello"'00
@_Sx3 
"goodbye"'00
