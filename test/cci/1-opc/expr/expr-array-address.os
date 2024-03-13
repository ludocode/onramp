#line manual
#line 1 "./expr/expr-array-address.i"
@_F_main 
#
#
  add r0 rfp -4 
  push r0 
  imw r0 4 
  pop r1 
  shl r0 r0 24 
  shrs r0 r0 24 
  stb r0 0 r1 
#
  add r0 rfp -4 
  push r0 
  add r0 rfp -4 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
#
  imw r0 1 
  leave 
  ret 
#
#
:_Lx0 
  add r0 rfp -4 
  push r0 
  imw r0 4 
  pop r1 
  ldb r1 0 r1 
  shl r1 r1 24 
  shrs r1 r1 24 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
#
  imw r0 1 
  leave 
  ret 
#
#
:_Lx1 
  add r0 rfp -4 
  push r0 
  imw r0 4 
  pop r1 
  ldb r1 0 r1 
  shl r1 r1 24 
  shrs r1 r1 24 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
#
  imw r0 1 
  leave 
  ret 
#
#
:_Lx2 
  imw r0 0 
  leave 
  ret 
#
#
  zero r0 
  leave 
  ret 

=main 
  enter 
  sub rsp rsp 4 
  jmp ^_F_main 






