@_F_main 
  add r0 rfp -4 
  push r0 
  imw r0 0xFFF2345 
  pop r1 
  sxs r0 r0 
  sts r0 0 r1 
  add r0 rfp -4 
  push r0 
  imw r0 9029 
  pop r1 
  lds r1 0 r1 
  sxs r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  sub rsp rsp 4 
  jmp ^_F_main 
