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
#line 1 "./expr/expr-array-dereference.c"
#
#
#line 1 "../../../core/libc/0-oo/include/string.h"
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#line 1 "../../../core/libc/0-oo/include/__onramp/__size_t.h"
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#line 33 "../../../core/libc/0-oo/include/string.h"
#
#
#line 1 "../../../core/libc/0-oo/include/__onramp/__null.h"
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#line 34 "../../../core/libc/0-oo/include/string.h"
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#line 2 "./expr/expr-array-dereference.c"
#
@_F_main 
#
#
  add r0 rfp -8 
  push r0 
  imw r0 0 
  pop r1 
  add r0 r1 r0 
  push r0 
  mov r0 "H"
  pop r1 
  sxb r0 r0 
  stb r0 0 r1 
#
  add r0 rfp -8 
  push r0 
  imw r0 1 
  pop r1 
  add r0 r1 r0 
  push r0 
  mov r0 "e"
  pop r1 
  sxb r0 r0 
  stb r0 0 r1 
#
  add r0 rfp -8 
  push r0 
  imw r0 2 
  pop r1 
  add r0 r1 r0 
  push r0 
  mov r0 "l"
  pop r1 
  sxb r0 r0 
  stb r0 0 r1 
#
  add r0 rfp -8 
  push r0 
  imw r0 3 
  pop r1 
  add r0 r1 r0 
  push r0 
  mov r0 "l"
  pop r1 
  sxb r0 r0 
  stb r0 0 r1 
#
  add r0 rfp -8 
  push r0 
  imw r0 4 
  pop r1 
  add r0 r1 r0 
  push r0 
  mov r0 "o"
  pop r1 
  sxb r0 r0 
  stb r0 0 r1 
#
  add r0 rfp -8 
  push r0 
  imw r0 5 
  pop r1 
  add r0 r1 r0 
  push r0 
  imw r0 0 
  pop r1 
  sxb r0 r0 
  stb r0 0 r1 
#
  imw r0 0 
  push r0 
  add r0 rfp -8 
  push r0 
  imw r0 ^_Sx0 
  add r0 rpp r0 
  push r0 
  ldw r0 rsp 4 
  ldw r1 rsp 0 
  call ^strcmp 
  add rsp rsp 8 
  pop r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
#
  imw r0 1 
  leave 
  ret 
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
  sub rsp rsp 8 
  jmp ^_F_main 



@_Sx0 
"Hello"'00




