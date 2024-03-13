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
#line 1 "./if/if-else-block.c"
@_F_main 
#
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
#
  add r0 rfp -8 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
#
  imw r0 1 
  jz r0 &_Lx0 
#
  add r0 rfp -4 
  push r0 
  imw r0 5 
  pop r1 
  stw r0 0 r1 
#
  jmp &_Lx1 
:_Lx0 
#
  add r0 rfp -8 
  push r0 
  imw r0 7 
  pop r1 
  stw r0 0 r1 
#
#
:_Lx1 
  add r0 rfp -4 
  push r0 
  imw r0 5 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  push r0 
  add r0 rfp -8 
  push r0 
  imw r0 0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  pop r1 
  or r0 r1 r0 
  jz r0 &_Lx2 
#
  imw r0 1 
  leave 
  ret 
#
#
#
:_Lx2 
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
#
  add r0 rfp -8 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
#
  imw r0 0 
  jz r0 &_Lx3 
#
  add r0 rfp -4 
  push r0 
  imw r0 5 
  pop r1 
  stw r0 0 r1 
#
  jmp &_Lx4 
:_Lx3 
#
  add r0 rfp -8 
  push r0 
  imw r0 7 
  pop r1 
  stw r0 0 r1 
#
#
:_Lx4 
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  push r0 
  add r0 rfp -8 
  push r0 
  imw r0 7 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  pop r1 
  or r0 r1 r0 
  jz r0 &_Lx5 
#
  imw r0 1 
  leave 
  ret 
#
#
#
:_Lx5 
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
  sub rsp rsp 8 
  jmp ^_F_main 






