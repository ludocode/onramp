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
#line 1 "./stmt/for-loop.c"
@_F_main 
#
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
#
#
#
  add r0 rfp -8 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
:_Lx0 
  add r0 rfp -8 
  push r0 
  imw r0 5 
  pop r1 
  ldw r1 0 r1 
  cmps r0 r1 r0 
  cmpu r0 r0 -1 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_Lx1 
  jmp &_Lx2 
:_Lx3 
  add r0 rfp -8 
  push r0 
  add r0 rfp -8 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  jmp &_Lx0 
:_Lx2 
#
  add r0 rfp -4 
  push r0 
  add r0 rfp -4 
  push r0 
  add r0 rfp -8 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
#
#
  jmp &_Lx3 
:_Lx1 
  add r0 rfp -4 
  push r0 
  imw r0 10 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
#
  imw r0 1 
  leave 
  ret 
#
#
#
#
:_Lx4 
#
  add r0 rfp -8 
  push r0 
  imw r0 5 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -8 
  push r0 
  imw r0 3 
  pop r1 
  stw r0 0 r1 
:_Lx5 
  add r0 rfp -8 
  push r0 
  imw r0 2 
  sub r0 0 r0 
  pop r1 
  ldw r1 0 r1 
  cmps r0 r1 r0 
  cmpu r0 r0 1 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_Lx6 
  jmp &_Lx7 
:_Lx8 
  add r0 rfp -8 
  push r0 
  add r0 rfp -8 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  sub r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  jmp &_Lx5 
:_Lx7 
#
  add r0 rfp -4 
  push r0 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  sub r0 r1 r0 
  pop r1 
  stw r0 0 r1 
#
#
  jmp &_Lx8 
:_Lx6 
  add r0 rfp -4 
  push r0 
  imw r0 5 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx9 
#
  imw r0 1 
  leave 
  ret 
#
#
#
#
:_Lx9 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
:_LxA 
  imw r0 0 
  jz r0 &_LxB 
  jmp &_LxC 
:_LxD 
  add r0 rfp -4 
  push r0 
  imw r0 10 
  pop r1 
  stw r0 0 r1 
  jmp &_LxA 
:_LxC 
#
  imw r0 1 
  leave 
  ret 
#
#
  jmp &_LxD 
:_LxB 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxE 
#
  imw r0 1 
  leave 
  ret 
#
#
#
#
:_LxE 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
#
:_LxF 
  jmp &_Lx11 
:_Lx12 
  jmp &_LxF 
:_Lx11 
#
  add r0 rfp -4 
  push r0 
  imw r0 2 
  pop r1 
  stw r0 0 r1 
#
#
  jmp &_Lx10 
#
  jmp &_Lx12 
:_Lx10 
  add r0 rfp -4 
  push r0 
  imw r0 2 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx13 
#
  imw r0 1 
  leave 
  ret 
#
#
#
#
:_Lx13 
  add r0 rfp -4 
  push r0 
  imw r0 5 
  pop r1 
  stw r0 0 r1 
#
:_Lx14 
  add r0 rfp -4 
  ldw r0 0 r0 
  jz r0 &_Lx15 
  jmp &_Lx16 
:_Lx17 
  jmp &_Lx14 
:_Lx16 
  add r0 rfp -4 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  sub r0 r1 r0 
  pop r1 
  stw r0 0 r1 
#
  jmp &_Lx17 
:_Lx15 
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx18 
#
  imw r0 1 
  leave 
  ret 
#
#
#
:_Lx18 
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





