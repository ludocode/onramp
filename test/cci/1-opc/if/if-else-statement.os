#line manual
#line 1 "/tmp/onramp-test.i"
#line 1 "../../../core/libc/0-oo/include/__onramp/__predef.h"
#line 1 "../../../core/libc/0-oo/include/__onramp/__bool.h"
#line 34 "../../../core/libc/0-oo/include/__onramp/__predef.h"
#line 1 "./if/if-else-statement.c"
@_F_main 
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
  imw r0 0 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
  jmp &_Lx1 
:_Lx0 
  imw r0 1 
  jz r0 &_Lx2 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
  jmp &_Lx3 
:_Lx2 
  imw r0 1 
  leave 
  ret 
:_Lx3 
:_Lx1 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
  imw r0 1 
  leave 
  ret 
:_Lx4 
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
  imw r0 1 
  jz r0 &_Lx5 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
  jmp &_Lx6 
:_Lx5 
  imw r0 1 
  jz r0 &_Lx7 
  imw r0 1 
  leave 
  ret 
  jmp &_Lx8 
:_Lx7 
  imw r0 1 
  leave 
  ret 
:_Lx8 
:_Lx6 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx9 
  imw r0 1 
  leave 
  ret 
:_Lx9 
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
  imw r0 0 
  jz r0 &_LxA 
  imw r0 1 
  leave 
  ret 
  jmp &_LxB 
:_LxA 
  imw r0 0 
  jz r0 &_LxC 
  imw r0 1 
  leave 
  ret 
  jmp &_LxD 
:_LxC 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
:_LxD 
:_LxB 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxE 
  imw r0 1 
  leave 
  ret 
:_LxE 
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
