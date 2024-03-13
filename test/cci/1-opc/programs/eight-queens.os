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
#line 1 "./programs/eight-queens.c"
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#line 1 "../../../core/libc/0-oo/include/stdio.h"
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
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
#line 33 "../../../core/libc/0-oo/include/stdio.h"
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
#line 34 "../../../core/libc/0-oo/include/stdio.h"
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#line 33 "./programs/eight-queens.c"
#
#
#line 1 "../../../core/libc/0-oo/include/stdlib.h"
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
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
#line 33 "../../../core/libc/0-oo/include/stdlib.h"
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
#line 34 "../../../core/libc/0-oo/include/stdlib.h"
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#line 34 "./programs/eight-queens.c"
#
=queens 
'00'00'00'00



#
#
#
#
@_F_main 
#
  imw r0 ^queens 
  add r0 rpp r0 
  push r0 
  imw r0 8 
  push r0 
  pop r0 
  call ^malloc 
  pop r1 
  stw r0 0 r1 
#
  imw r0 0 
  push r0 
  pop r0 
  call ^place_queen 
#
  imw r0 ^queens 
  add r0 rpp r0 
  ldw r0 0 r0 
  push r0 
  pop r0 
  call ^free 
#
#
#
  zero r0 
  leave 
  ret 

=main 
  enter 
  jmp ^_F_main 





@_F_is_queen_valid 
  stw r0 rfp -4 
  stw r1 rfp -8 
#
#
  add r0 rfp -12 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
#
:_Lx0 
  add r0 rfp -12 
  push r0 
  add r0 rfp -4 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  cmps r0 r1 r0 
  cmpu r0 r0 -1 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_Lx1 
#
#
#
  add r0 rfp -16 
  push r0 
  imw r0 ^queens 
  add r0 rpp r0 
  push r0 
  add r0 rfp -12 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  ldb r0 0 r0 
  shl r0 r0 24 
  shrs r0 r0 24 
  stw r0 0 r1 
#
  add r0 rfp -20 
  push r0 
  add r0 rfp -4 
  push r0 
  add r0 rfp -12 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  sub r0 r1 r0 
  pop r1 
  stw r0 0 r1 
#
  add r0 rfp -16 
  push r0 
  add r0 rfp -8 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  xor r0 r0 1 
  push r0 
  add r0 rfp -16 
  push r0 
  add r0 rfp -8 
  push r0 
  add r0 rfp -20 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  sub r0 r1 r0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  xor r0 r0 1 
  pop r1 
  or r0 r1 r0 
  push r0 
  add r0 rfp -16 
  push r0 
  add r0 rfp -8 
  push r0 
  add r0 rfp -20 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  xor r0 r0 1 
  pop r1 
  or r0 r1 r0 
  jz r0 &_Lx2 
#
  imw r0 0 
  leave 
  ret 
#
#
:_Lx2 
  add r0 rfp -12 
  push r0 
  add r0 rfp -12 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
#
#
  jmp &_Lx0 
:_Lx1 
  imw r0 1 
  leave 
  ret 
#
#
#
  zero r0 
  leave 
  ret 

=is_queen_valid 
  enter 
  sub rsp rsp 20 
  jmp ^_F_is_queen_valid 





@_F_place_queen 
  stw r0 rfp -4 
#
  add r0 rfp -4 
  push r0 
  imw r0 8 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_Lx3 
#
  call ^print_board 
#
  leave 
  ret 
#
#
#
:_Lx3 
#
  add r0 rfp -8 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
#
:_Lx4 
  add r0 rfp -8 
  push r0 
  imw r0 8 
  pop r1 
  ldw r1 0 r1 
  cmps r0 r1 r0 
  cmpu r0 r0 -1 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_Lx5 
#
  add r0 rfp -4 
  ldw r0 0 r0 
  push r0 
  add r0 rfp -8 
  ldw r0 0 r0 
  push r0 
  pop r1 
  pop r0 
  call ^is_queen_valid 
  jz r0 &_Lx6 
#
  imw r0 ^queens 
  add r0 rpp r0 
  push r0 
  add r0 rfp -4 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -8 
  pop r1 
  ldw r0 0 r0 
  shl r0 r0 24 
  shrs r0 r0 24 
  stb r0 0 r1 
#
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  push r0 
  pop r0 
  call ^place_queen 
#
#
:_Lx6 
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
#
#
  jmp &_Lx4 
:_Lx5 
#
#
  zero r0 
  leave 
  ret 

=place_queen 
  enter 
  sub rsp rsp 8 
  jmp ^_F_place_queen 





@_F_print_board 
#
#
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
#
:_Lx7 
  add r0 rfp -4 
  push r0 
  imw r0 8 
  pop r1 
  ldw r1 0 r1 
  cmps r0 r1 r0 
  cmpu r0 r0 -1 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_Lx8 
#
#
  add r0 rfp -8 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
#
:_Lx9 
  add r0 rfp -8 
  push r0 
  imw r0 8 
  pop r1 
  ldw r1 0 r1 
  cmps r0 r1 r0 
  cmpu r0 r0 -1 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_LxA 
#
#
  add r0 rfp -12 
  push r0 
  imw r0 ^queens 
  add r0 rpp r0 
  push r0 
  add r0 rfp -8 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  add r0 r1 r0 
  push r0 
  add r0 rfp -4 
  pop r1 
  ldw r0 0 r0 
  ldb r1 0 r1 
  shl r1 r1 24 
  shrs r1 r1 24 
  cmpu r0 r1 r0 
  and r0 r0 1 
  xor r0 r0 1 
  pop r1 
  stw r0 0 r1 
#
  add r0 rfp -12 
  ldw r0 0 r0 
  jz r0 &_LxB 
#
  mov r0 "Q"
  push r0 
  pop r0 
  call ^putchar 
#
#
:_LxB 
  add r0 rfp -12 
  ldw r0 0 r0 
  cmpu r0 r0 0 
  and r0 r0 1 
  xor r0 r0 1 
  jz r0 &_LxC 
#
  mov r0 "."
  push r0 
  pop r0 
  call ^putchar 
#
#
:_LxC 
  mov r0 " "
  push r0 
  pop r0 
  call ^putchar 
#
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
#
#
  jmp &_Lx9 
:_LxA 
  mov r0 '0A
  push r0 
  pop r0 
  call ^putchar 
#
  add r0 rfp -4 
  push r0 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
#
#
  jmp &_Lx7 
:_Lx8 
  mov r0 '0A
  push r0 
  pop r0 
  call ^putchar 
#
  mov r0 '0A
  push r0 
  pop r0 
  call ^putchar 
#
#
  zero r0 
  leave 
  ret 

=print_board 
  enter 
  sub rsp rsp 12 
  jmp ^_F_print_board 






