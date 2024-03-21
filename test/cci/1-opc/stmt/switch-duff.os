#line manual
#line 1 "/tmp/onramp-test.i"
#line 1 "../../../core/libc/0-oo/include/__onramp/__predef.h"
#line 1 "../../../core/libc/0-oo/include/__onramp/__bool.h"
#line 34 "../../../core/libc/0-oo/include/__onramp/__predef.h"
#line 1 "./stmt/switch-duff.c"
#line 1 "../../../core/libc/0-oo/include/string.h"
#line 1 "../../../core/libc/0-oo/include/__onramp/__size_t.h"
#line 33 "../../../core/libc/0-oo/include/string.h"
#line 1 "../../../core/libc/0-oo/include/__onramp/__null.h"
#line 34 "../../../core/libc/0-oo/include/string.h"
#line 2 "./stmt/switch-duff.c"
@_F_main 
  imw r0 -132 
  add r0 rfp r0 
  push r0 
  imw r0 ^_Sx0 
  add r0 rpp r0 
  pop r1 
  stw r0 0 r1 
  imw r0 -136 
  add r0 rfp r0 
  push r0 
  imw r0 -128 
  add r0 rfp r0 
  pop r1 
  stw r0 0 r1 
  imw r0 -140 
  add r0 rfp r0 
  push r0 
  imw r0 -132 
  add r0 rfp r0 
  pop r1 
  ldw r0 0 r0 
  stw r0 0 r1 
  imw r0 -144 
  add r0 rfp r0 
  push r0 
  imw r0 -132 
  add r0 rfp r0 
  ldw r0 0 r0 
  push r0 
  ldw r0 rsp 0 
  call ^strlen 
  add rsp rsp 4 
  push r0 
  imw r0 1 
  pop r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  imw r0 -148 
  add r0 rfp r0 
  push r0 
  imw r0 -144 
  add r0 rfp r0 
  push r0 
  imw r0 7 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  push r0 
  imw r0 8 
  pop r1 
  divs r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  imw r0 -144 
  add r0 rfp r0 
  push r0 
  imw r0 7 
  pop r1 
  ldw r1 0 r1 
  and r0 r1 r0 
  imw r1 -152 
  add r1 rfp r1 
  stw r0 0 r1 
  jmp &_Lx1 
  jmp &_Lx2 
:_Lx1 
  imw r0 0 
  imw r1 -152 
  ldw r1 rfp r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  jmp &_Lx3 
:_Lx2 
:_Lx4 
  imw r0 -136 
  add r0 rfp r0 
  ldw r0 0 r0 
  push r0 
  imw r0 -140 
  add r0 rfp r0 
  ldw r0 0 r0 
  pop r1 
  ldb r0 0 r0 
  stb r0 0 r1 
  imw r0 -136 
  add r0 rfp r0 
  push r0 
  imw r0 -136 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  imw r0 -140 
  add r0 rfp r0 
  push r0 
  imw r0 -140 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  jmp &_Lx6 
:_Lx3 
  imw r0 7 
  imw r1 -152 
  ldw r1 rfp r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx6 
  jmp &_Lx7 
:_Lx6 
  imw r0 -136 
  add r0 rfp r0 
  ldw r0 0 r0 
  push r0 
  imw r0 -140 
  add r0 rfp r0 
  ldw r0 0 r0 
  pop r1 
  ldb r0 0 r0 
  stb r0 0 r1 
  imw r0 -136 
  add r0 rfp r0 
  push r0 
  imw r0 -136 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  imw r0 -140 
  add r0 rfp r0 
  push r0 
  imw r0 -140 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  jmp &_Lx8 
:_Lx7 
  imw r0 6 
  imw r1 -152 
  ldw r1 rfp r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx8 
  jmp &_Lx9 
:_Lx8 
  imw r0 -136 
  add r0 rfp r0 
  ldw r0 0 r0 
  push r0 
  imw r0 -140 
  add r0 rfp r0 
  ldw r0 0 r0 
  pop r1 
  ldb r0 0 r0 
  stb r0 0 r1 
  imw r0 -136 
  add r0 rfp r0 
  push r0 
  imw r0 -136 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  imw r0 -140 
  add r0 rfp r0 
  push r0 
  imw r0 -140 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  jmp &_LxA 
:_Lx9 
  imw r0 5 
  imw r1 -152 
  ldw r1 rfp r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxA 
  jmp &_LxB 
:_LxA 
  imw r0 -136 
  add r0 rfp r0 
  ldw r0 0 r0 
  push r0 
  imw r0 -140 
  add r0 rfp r0 
  ldw r0 0 r0 
  pop r1 
  ldb r0 0 r0 
  stb r0 0 r1 
  imw r0 -136 
  add r0 rfp r0 
  push r0 
  imw r0 -136 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  imw r0 -140 
  add r0 rfp r0 
  push r0 
  imw r0 -140 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  jmp &_LxC 
:_LxB 
  imw r0 4 
  imw r1 -152 
  ldw r1 rfp r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxC 
  jmp &_LxD 
:_LxC 
  imw r0 -136 
  add r0 rfp r0 
  ldw r0 0 r0 
  push r0 
  imw r0 -140 
  add r0 rfp r0 
  ldw r0 0 r0 
  pop r1 
  ldb r0 0 r0 
  stb r0 0 r1 
  imw r0 -136 
  add r0 rfp r0 
  push r0 
  imw r0 -136 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  imw r0 -140 
  add r0 rfp r0 
  push r0 
  imw r0 -140 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  jmp &_LxE 
:_LxD 
  imw r0 3 
  imw r1 -152 
  ldw r1 rfp r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxE 
  jmp &_LxF 
:_LxE 
  imw r0 -136 
  add r0 rfp r0 
  ldw r0 0 r0 
  push r0 
  imw r0 -140 
  add r0 rfp r0 
  ldw r0 0 r0 
  pop r1 
  ldb r0 0 r0 
  stb r0 0 r1 
  imw r0 -136 
  add r0 rfp r0 
  push r0 
  imw r0 -136 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  imw r0 -140 
  add r0 rfp r0 
  push r0 
  imw r0 -140 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  jmp &_Lx10 
:_LxF 
  imw r0 2 
  imw r1 -152 
  ldw r1 rfp r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx10 
  jmp &_Lx11 
:_Lx10 
  imw r0 -136 
  add r0 rfp r0 
  ldw r0 0 r0 
  push r0 
  imw r0 -140 
  add r0 rfp r0 
  ldw r0 0 r0 
  pop r1 
  ldb r0 0 r0 
  stb r0 0 r1 
  imw r0 -136 
  add r0 rfp r0 
  push r0 
  imw r0 -136 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  imw r0 -140 
  add r0 rfp r0 
  push r0 
  imw r0 -140 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  jmp &_Lx12 
:_Lx11 
  imw r0 1 
  imw r1 -152 
  ldw r1 rfp r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx12 
  jmp &_Lx13 
:_Lx12 
  imw r0 -136 
  add r0 rfp r0 
  ldw r0 0 r0 
  push r0 
  imw r0 -140 
  add r0 rfp r0 
  ldw r0 0 r0 
  pop r1 
  ldb r0 0 r0 
  stb r0 0 r1 
  imw r0 -136 
  add r0 rfp r0 
  push r0 
  imw r0 -136 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  imw r0 -140 
  add r0 rfp r0 
  push r0 
  imw r0 -140 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  imw r0 -148 
  add r0 rfp r0 
  push r0 
  imw r0 -148 
  add r0 rfp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  sub r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  push r0 
  imw r0 0 
  pop r1 
  cmps r0 r1 r0 
  cmpu r0 r0 1 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_Lx5 
  jmp &_Lx4 
:_Lx5 
  jmp &_Lx0 
:_Lx13 
:_Lx0 
  imw r0 0 
  push r0 
  imw r0 -128 
  add r0 rfp r0 
  push r0 
  imw r0 -132 
  add r0 rfp r0 
  ldw r0 0 r0 
  push r0 
  ldw r0 rsp 4 
  ldw r1 rsp 0 
  call ^strcmp 
  add rsp rsp 8 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx14 
  imw r0 1 
  leave 
  ret 
:_Lx14 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  imw r9 152 
  sub rsp rsp r9 
  jmp ^_F_main 
@_Sx0 
"The quick brown fox jumps over the lazy dog."'00
