#line manual
#line 1 "/tmp/onramp-test.i"
#line 1 "../../../core/libc/0-oo/include/__onramp/__predef.h"
#line 1 "../../../core/libc/0-oo/include/__onramp/__bool.h"
#line 34 "../../../core/libc/0-oo/include/__onramp/__predef.h"
#line 1 "./function/function-variadic.c"
@_F_bar 
  stw r0 rfp -4 
  add r0 rfp -4 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  shl r0 r0 2 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  ldw r0 0 r0 
  push r0 
  imw r0 2 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  add r0 rfp -4 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  shl r0 r0 2 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  ldw r0 0 r0 
  push r0 
  imw r0 3 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 1 
  leave 
  ret 
:_Lx1 
  add r0 rfp -4 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  shl r0 r0 2 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  ldw r0 0 r0 
  push r0 
  imw r0 4 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 1 
  leave 
  ret 
:_Lx2 
  add r0 rfp -4 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  shl r0 r0 2 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  ldw r0 0 r0 
  push r0 
  imw r0 5 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
  imw r0 1 
  leave 
  ret 
:_Lx3 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=bar 
  enter 
  sub rsp rsp 4 
  jmp ^_F_bar 
@_F_foo 
  stw r0 rfp -4 
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
  add r0 rfp -8 
  mov r1 r0 
  add r0 rfp 4 
  stw r0 0 r1 
  add r0 rfp -12 
  push r0 
  add r0 rfp -8 
  pop r1 
  ldw r0 0 r0 
  stw r0 0 r1 
  add r0 rfp -12 
  ldw r0 0 r0 
  push r0 
  ldw r0 rsp 0 
  call ^bar 
  add rsp rsp 4 
  jz r0 &_Lx5 
  imw r0 1 
  leave 
  ret 
:_Lx5 
  add r0 rfp -8 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  shl r0 r0 2 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  ldw r0 0 r0 
  push r0 
  imw r0 2 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx6 
  imw r0 1 
  leave 
  ret 
:_Lx6 
  add r0 rfp -8 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  shl r0 r0 2 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  ldw r0 0 r0 
  push r0 
  imw r0 3 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx7 
  imw r0 1 
  leave 
  ret 
:_Lx7 
  add r0 rfp -8 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  shl r0 r0 2 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  ldw r0 0 r0 
  push r0 
  imw r0 4 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx8 
  imw r0 1 
  leave 
  ret 
:_Lx8 
  add r0 rfp -8 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  shl r0 r0 2 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  ldw r0 0 r0 
  push r0 
  imw r0 5 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx9 
  imw r0 1 
  leave 
  ret 
:_Lx9 
  add r0 rfp -8 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=foo 
  enter 
  sub rsp rsp 12 
  jmp ^_F_foo 
@_F_main 
  imw r0 1 
  push r0 
  imw r0 2 
  push r0 
  imw r0 3 
  push r0 
  imw r0 4 
  push r0 
  imw r0 5 
  push r0 
  ldw r0 rsp 16 
  ldw r9 rsp 0 
  push r9 
  ldw r9 rsp 8 
  push r9 
  ldw r9 rsp 16 
  push r9 
  ldw r9 rsp 24 
  push r9 
  call ^foo 
  add rsp rsp 36 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
