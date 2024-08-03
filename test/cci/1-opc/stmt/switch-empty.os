@_F_main 
  imw r0 1 
  add r1 rfp -4 
  stw r0 0 r1 
  jmp &_Lx1 
  imw r0 1 
  leave 
  ret 
  jmp &_Lx0 
:_Lx1 
:_Lx0 
  imw r0 1 
  add r1 rfp -8 
  stw r0 0 r1 
  jmp &_Lx3 
:_U_4_main_foo
  imw r0 0 
  leave 
  ret 
  jmp &_Lx2 
:_Lx3 
:_Lx2 
  jmp &_U_4_main_foo
  imw r0 2 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  sub rsp rsp 8 
  jmp ^_F_main 
