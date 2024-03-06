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
#line 1 "./locals/local-initializer.c"
@_F_main 
#
  add r0 rfp -4 
  push r0 
  imw r0 5 
  pop r1 
  stw r0 0 r1 
#
  add r0 rfp -8 
  push r0 
  imw r0 10 
  pop r1 
  stw r0 0 r1 
#
  add r0 rfp -4 
  push r0 
  imw r0 2 
  pop r1 
  ldw r1 0 r1 
  mul r0 r1 r0 
  push r0 
  add r0 rfp -8 
  pop r1 
  ldw r0 0 r0 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
#
  imw r0 1 
#
  leave 
  ret 
#
#
:_Lx0 
  add r0 rfp -12 
  push r0 
  imw r0 15 
  pop r1 
  stw r0 0 r1 
#
  add r0 rfp -12 
  push r0 
  add r0 rfp -4 
  push r0 
  add r0 rfp -8 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
#
  imw r0 1 
#
  leave 
  ret 
#
#
:_Lx1 
  add r0 rfp -16 
  push r0 
  add r0 rfp -4 
  pop r1 
  stw r0 0 r1 
#
  add r0 rfp -20 
  push r0 
  add r0 rfp -16 
  pop r1 
  stw r0 0 r1 
#
  add r0 rfp -24 
  push r0 
  add r0 rfp -12 
  pop r1 
  stw r0 0 r1 
#
  add r0 rfp -16 
  push r0 
  add r0 rfp -4 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
#
  imw r0 1 
#
  leave 
  ret 
#
:_Lx2 
  add r0 rfp -20 
  ldw r0 0 r0 
  push r0 
  add r0 rfp -4 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
#
  imw r0 1 
#
  leave 
  ret 
#
:_Lx3 
  add r0 rfp -24 
  ldw r0 0 r0 
  push r0 
  imw r0 3 
  push r0 
  add r0 rfp -20 
  ldw r0 0 r0 
  ldw r0 0 r0 
  pop r1 
  ldw r0 0 r0 
  mul r0 r1 r0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
#
  imw r0 1 
#
  leave 
  ret 
#
#
:_Lx4 
  add r0 rfp -28 
  push r0 
  add r0 rfp -16 
  ldw r0 0 r0 
  pop r1 
  ldw r0 0 r0 
  stw r0 0 r1 
#
  add r0 rfp -4 
  push r0 
  add r0 rfp -28 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx5 
#
  imw r0 1 
#
  leave 
  ret 
#
#
:_Lx5 
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
  sub rsp rsp 28 
  jmp ^_F_main 






