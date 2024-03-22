@_F_open 
  stw r0 rfp -4 
  stw r1 rfp -8 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=open 
  enter 
  sub rsp rsp 8 
  jmp ^_F_open 
@_F_main 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
