#line manual
#line 1 "/tmp/onramp-test.i"
#line 1 "../../../core/libc/0-oo/include/__onramp/__predef.h"
#line 1 "../../../core/libc/0-oo/include/__onramp/__bool.h"
#line 34 "../../../core/libc/0-oo/include/__onramp/__predef.h"
#line 1 "./type/short-store.c"
@_F_main 
  add r0 rfp -4 
  push r0 
  imw r0 0xfff2345 
  pop r1 
  sxs r0 r0 
  sts r0 0 r1 
  add r0 rfp -4 
  push r0 
  imw r0 0x2345 
  pop r1 
  lds r1 0 r1 
  sxs r1 r1 
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
