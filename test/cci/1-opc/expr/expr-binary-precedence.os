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
#line 1 "./expr/expr-binary-precedence.c"
@_F_main 
#
#
#
  imw r0 5 
  push r0 
  imw r0 3 
  pop r1 
  mul r0 r1 r0 
  push r0 
  imw r0 10 
  pop r1 
  add r0 r1 r0 
  push r0 
  imw r0 2 
  pop r1 
  divs r0 r1 r0 
  push r0 
  imw r0 30 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  push r0 
  imw r0 2 
  pop r1 
  mul r0 r1 r0 
  push r0 
  imw r0 3 
  pop r1 
  divs r0 r1 r0 
  jz r0 &_Lx0 
#
  imw r0 1 
  leave 
  ret 
#
#
#
:_Lx0 
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
  jmp ^_F_main 






