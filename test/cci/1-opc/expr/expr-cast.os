#line manual
#line 1 "/tmp/onramp-test.i"
#line 1 "../../../core/libc/0-oo/include/__onramp/__predef.h"
#line 1 "../../../core/libc/0-oo/include/__onramp/__bool.h"
#line 34 "../../../core/libc/0-oo/include/__onramp/__predef.h"
#line 1 "./expr/expr-cast.c"
@_F_main 
  imw r0 0x123 
  sxb r0 r0 
  push r0 
  imw r0 0x23 
  pop r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  imw r0 0x123 
  sxb r0 r0 
  push r0 
  imw r0 0x23 
  pop r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 1 
  leave 
  ret 
:_Lx1 
  imw r0 0x123 
  sxb r0 r0 
  push r0 
  imw r0 0x23 
  pop r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 1 
  leave 
  ret 
:_Lx2 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
