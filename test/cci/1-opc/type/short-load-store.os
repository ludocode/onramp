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
#line 1 "./type/short-load-store.c"
=x 
'00'00'00'00 '00'00'00'00



#
#
@_F_main 
#
  imw r0 ^x 
  add r0 rpp r0 
  push r0 
  imw r0 0 
  pop r1 
  shl r0 r0 1 
  add r0 r1 r0 
  push r0 
  imw r0 0x1234 
  pop r1 
  sxs r0 r0 
  sts r0 0 r1 
#
  imw r0 ^x 
  add r0 rpp r0 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 1 
  add r0 r1 r0 
  push r0 
  imw r0 0x5678 
  pop r1 
  sxs r0 r0 
  sts r0 0 r1 
#
  imw r0 ^x 
  add r0 rpp r0 
  push r0 
  imw r0 2 
  pop r1 
  shl r0 r0 1 
  add r0 r1 r0 
  push r0 
  imw r0 0x147A 
  pop r1 
  sxs r0 r0 
  sts r0 0 r1 
#
  imw r0 ^x 
  add r0 rpp r0 
  push r0 
  imw r0 3 
  pop r1 
  shl r0 r0 1 
  add r0 r1 r0 
  push r0 
  imw r0 0x258B 
  pop r1 
  sxs r0 r0 
  sts r0 0 r1 
#
#
  imw r0 ^x 
  add r0 rpp r0 
  push r0 
  imw r0 0 
  pop r1 
  shl r0 r0 1 
  add r0 r1 r0 
  push r0 
  imw r0 0x1234 
  pop r1 
  lds r1 0 r1 
  sxs r1 r1 
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
  imw r0 ^x 
  add r0 rpp r0 
  push r0 
  imw r0 1 
  pop r1 
  shl r0 r0 1 
  add r0 r1 r0 
  push r0 
  imw r0 0x5678 
  pop r1 
  lds r1 0 r1 
  sxs r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
#
  imw r0 2 
  leave 
  ret 
#
#
:_Lx1 
  imw r0 ^x 
  add r0 rpp r0 
  push r0 
  imw r0 2 
  pop r1 
  shl r0 r0 1 
  add r0 r1 r0 
  push r0 
  imw r0 0x147A 
  pop r1 
  lds r1 0 r1 
  sxs r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
#
  imw r0 3 
  leave 
  ret 
#
#
:_Lx2 
  imw r0 ^x 
  add r0 rpp r0 
  push r0 
  imw r0 3 
  pop r1 
  shl r0 r0 1 
  add r0 r1 r0 
  push r0 
  imw r0 0x258B 
  pop r1 
  lds r1 0 r1 
  sxs r1 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
#
  imw r0 4 
  leave 
  ret 
#
#
#
:_Lx3 
  add r0 rfp -4 
  push r0 
  imw r0 ^x 
  add r0 rpp r0 
  pop r1 
  stw r0 0 r1 
#
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  ldw r1 0 r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 0x56781234 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
#
  imw r0 5 
  leave 
  ret 
#
#
:_Lx4 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  shl r0 r0 2 
  add r0 r1 r0 
  push r0 
  imw r0 0x258B147A 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx5 
#
  imw r0 6 
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
  sub rsp rsp 4 
  jmp ^_F_main 






