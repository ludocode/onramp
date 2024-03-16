#line manual
#line 1 "/tmp/onramp-test.i"
#
#line 1 "../../../core/libc/0-oo/include/__onramp/__predef.h"
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#line 1 "../../../core/libc/0-oo/include/__onramp/__bool.h"
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#line 34 "../../../core/libc/0-oo/include/__onramp/__predef.h"
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#line 1 "./expr/expr-comma.c"
@_F_foo 
  stw r0 rfp -4 
#
  add r0 rfp -4 
  ldw r0 0 r0 
  push r0 
  imw r0 5 
  pop r1 
  stw r0 0 r1 
#
  imw r0 0 
  leave 
  ret 
#
#
#
  zero r0 
  leave 
  ret 

=foo 
  enter 
  sub rsp rsp 4 
  jmp ^_F_foo 





@_F_main 
#
#
#
#
  imw r0 1 
  imw r0 0 
  imw r0 1 
  imw r0 0 
  jz r0 &_Lx0 
#
  imw r0 1 
  leave 
  ret 
#
#
:_Lx0 
  imw r0 0 
  imw r0 1 
  imw r0 0 
  imw r0 1 
  isz r0 r0 
  jz r0 &_Lx1 
#
  imw r0 1 
  leave 
  ret 
#
#
#
#
:_Lx1 
  add r0 rfp -4 
  push r0 
  ldw r0 rsp 0 
  call ^foo 
  add rsp rsp 4 
  add r0 rfp -4 
  push r0 
  imw r0 5 
  pop r1 
  ldw r1 0 r1 
  sub r0 r1 r0 
  jz r0 &_Lx2 
#
  imw r0 1 
  leave 
  ret 
#
#
#
#
:_Lx2 
  add r0 rfp -4 
  push r0 
  imw r0 4 
  pop r1 
  stw r0 0 r1 
  imw r0 7 
#
  add r0 rfp -4 
  push r0 
  imw r0 4 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
#
  imw r0 1 
  leave 
  ret 
#
#
#
#
:_Lx3 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  jz r0 &_Lx4 
  imw r0 5 
  imw r0 3 
  jmp &_Lx5 
:_Lx4 
  imw r0 1 
  sub r0 0 r0 
:_Lx5 
  pop r1 
  stw r0 0 r1 
#
  add r0 rfp -4 
  push r0 
  imw r0 3 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx6 
#
  imw r0 1 
  leave 
  ret 
#
#
#
#
:_Lx6 
  imw r0 1 
  imw r0 1 
  jz r0 &_Lx8 
  imw r0 1 
  jmp &_Lx9 
:_Lx8 
  imw r0 1 
:_Lx9 
  imw r0 0 
  jz r0 &_Lx7 
#
  imw r0 1 
  leave 
  ret 
#
#
#
:_Lx7 
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






