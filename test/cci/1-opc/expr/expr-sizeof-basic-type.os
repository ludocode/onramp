#line manual
#line 1 "./expr/expr-sizeof-basic-type.i"
@_F_main 
#
  imw r0 4 
  push r0 
  imw r0 4 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
#
  imw r0 1 
#
  leave 
  ret 
#
:_Lx0 
  imw r0 1 
  push r0 
  imw r0 1 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
#
  imw r0 2 
#
  leave 
  ret 
#
:_Lx1 
  imw r0 1 
  push r0 
  imw r0 1 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
#
  imw r0 3 
#
  leave 
  ret 
#
:_Lx2 
  imw r0 4 
  push r0 
  imw r0 4 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
#
  imw r0 4 
#
  leave 
  ret 
#
:_Lx3 
  imw r0 4 
  push r0 
  imw r0 4 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
#
  imw r0 5 
#
  leave 
  ret 
#
:_Lx4 
  imw r0 0 
#
  leave 
  ret 
#
  zero r0 
  leave 
  ret 

=main 
  enter 
  jmp ^_F_main 






