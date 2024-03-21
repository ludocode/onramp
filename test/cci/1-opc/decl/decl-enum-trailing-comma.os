#line manual
#line 1 "./decl/decl-enum-trailing-comma.i"
@bar 
10 
@_F_main 
  imw r0 ^bar 
  add r0 rpp r0 
  push r0 
  imw r0 10 
  pop r1 
  ldw r1 0 r1 
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
  jmp ^_F_main 
