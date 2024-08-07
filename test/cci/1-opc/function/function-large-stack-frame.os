@_F_foo 
  stw r0 rfp -4 
  imw r0 -404 
  add r0 rfp r0 
  push r0 
  imw r0 0 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  imw r0 -404 
  add r0 rfp r0 
  push r0 
  imw r0 99 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 -404 
  add r0 rfp r0 
  push r0 
  imw r0 0 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 2 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  imw r0 6 
  push r0 
  imw r0 -404 
  add r0 rfp r0 
  push r0 
  imw r0 99 
  pop r1 
  shl r0 r0 2 
  add r0 r1 r0 
  pop r1 
  ldw r0 0 r0 
  sub r0 r1 r0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=foo 
  enter 
  imw r9 404 
  sub rsp rsp r9 
  jmp ^_F_foo 
@_F_main 
  imw r0 3 
  push r0 
  ldw r0 rsp 0 
  call ^foo 
  add rsp rsp 4 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
