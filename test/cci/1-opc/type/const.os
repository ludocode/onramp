@_F_main 
  add r0 rfp -4 
  push r0 
  imw r0 ^_Sx0 
  add r0 rpp r0 
  pop r1 
  stw r0 0 r1 
  imw r0 0 
  push r0 
  add r0 rfp -4 
  ldw r0 0 r0 
  push r0 
  imw r0 ^_Sx1 
  add r0 rpp r0 
  push r0 
  ldw r0 rsp 4 
  ldw r1 rsp 0 
  call ^strcmp 
  add rsp rsp 8 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  zero r0 
  leave 
  ret 
=main 
  enter 
  sub rsp rsp 4 
  jmp ^_F_main 
@_Sx0 
"Hello!"'00
@_Sx1 
"Hello!"'00
