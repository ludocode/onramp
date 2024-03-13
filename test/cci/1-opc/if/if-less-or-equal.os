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
#line 1 "./if/if-less-or-equal.c"
#
#
#line 1 "../../../core/libc/0-oo/include/limits.h"
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
#
#
#
#line 2 "./if/if-less-or-equal.c"
#
@_F_main 
#
  imw r0 5 
  push r0 
  imw r0 1 
  pop r1 
  cmps r0 r1 r0 
  cmpu r0 r0 -1 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_Lx0 
#
  imw r0 1 
  leave 
  ret 
#
#
:_Lx0 
  imw r0 5 
  push r0 
  imw r0 1 
  sub r0 0 r0 
  pop r1 
  cmps r0 r1 r0 
  cmpu r0 r0 -1 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_Lx1 
#
  imw r0 1 
  leave 
  ret 
#
#
:_Lx1 
  imw r0 1 
  push r0 
  imw r0 5 
  sub r0 0 r0 
  pop r1 
  cmps r0 r1 r0 
  cmpu r0 r0 -1 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_Lx2 
#
  imw r0 1 
  leave 
  ret 
#
#
:_Lx2 
  imw r0 5 
  sub r0 0 r0 
  push r0 
  imw r0 10 
  sub r0 0 r0 
  pop r1 
  cmps r0 r1 r0 
  cmpu r0 r0 -1 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_Lx3 
#
  imw r0 1 
  leave 
  ret 
#
#
:_Lx3 
  imw r0 2147483647 
  sub r0 0 r0 
  push r0 
  imw r0 1 
  pop r1 
  sub r0 r1 r0 
  push r0 
  imw r0 1 
  pop r1 
  add r0 r1 r0 
  push r0 
  imw r0 2147483647 
  sub r0 0 r0 
  push r0 
  imw r0 1 
  pop r1 
  sub r0 r1 r0 
  pop r1 
  cmps r0 r1 r0 
  cmpu r0 r0 -1 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_Lx4 
#
  imw r0 1 
  leave 
  ret 
#
#
:_Lx4 
  imw r0 2147483647 
  push r0 
  imw r0 2147483647 
  push r0 
  imw r0 1 
  pop r1 
  sub r0 r1 r0 
  pop r1 
  cmps r0 r1 r0 
  cmpu r0 r0 -1 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_Lx5 
#
  imw r0 1 
  leave 
  ret 
#
#
:_Lx5 
  imw r0 2147483647 
  push r0 
  imw r0 2147483647 
  sub r0 0 r0 
  push r0 
  imw r0 1 
  pop r1 
  sub r0 r1 r0 
  pop r1 
  cmps r0 r1 r0 
  cmpu r0 r0 -1 
  and r0 r0 1 
  xor r0 r0 1 
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
  push r0 
  imw r0 1 
  pop r1 
  cmps r0 r1 r0 
  cmpu r0 r0 1 
  and r0 r0 1 
  cmpu r0 r0 0 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_Lx7 
#
  imw r0 1 
  leave 
  ret 
#
#
:_Lx7 
  imw r0 2147483647 
  sub r0 0 r0 
  push r0 
  imw r0 1 
  pop r1 
  sub r0 r1 r0 
  push r0 
  imw r0 2147483647 
  sub r0 0 r0 
  push r0 
  imw r0 1 
  pop r1 
  sub r0 r1 r0 
  pop r1 
  cmps r0 r1 r0 
  cmpu r0 r0 1 
  and r0 r0 1 
  cmpu r0 r0 0 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_Lx8 
#
  imw r0 1 
  leave 
  ret 
#
#
:_Lx8 
  imw r0 2147483647 
  push r0 
  imw r0 2147483647 
  pop r1 
  cmps r0 r1 r0 
  cmpu r0 r0 1 
  and r0 r0 1 
  cmpu r0 r0 0 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_Lx9 
#
  imw r0 1 
  leave 
  ret 
#
#
:_Lx9 
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






