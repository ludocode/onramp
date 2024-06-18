@_F_main 
  imw r0 0 
  jz r0 &_Lx0 
  imw r0 1 
  jz r0 &_Lx1 
  imw r0 1 
  leave 
  ret 
  jmp &_Lx2 
:_Lx1 
  imw r0 1 
  leave 
  ret 
:_Lx2 
  jmp &_Lx3 
:_Lx0 
  imw r0 0 
  leave 
  ret 
:_Lx3 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
