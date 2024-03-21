#line manual
#line 1 "./expr/expr-conditional-types.i"
@_F_main 
  add r0 rfp -4 
  push r0 
  imw r0 5 
  sub r0 0 r0 
  pop r1 
  sxb r0 r0 
  stb r0 0 r1 
  imw r0 0 
  jz r0 &_Lx1 
  imw r0 3 
  jmp &_Lx2 
:_Lx1 
  add r0 rfp -4 
  ldb r0 0 r0 
  sxb r0 r0 
:_Lx2 
  push r0 
  imw r0 5 
  sub r0 0 r0 
  pop r1 
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
