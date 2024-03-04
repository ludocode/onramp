#line manual
#line 1 "./stdout/stdout-fputc.i"
#
#
#
@_F_main 
#
  imw r0 0x61 
  push r0 
  imw r0 ^stdout 
  add r0 rpp r0 
  ldw r0 0 r0 
  push r0 
  pop r1 
  pop r0 
  call ^fputc 
#
  imw r0 0 
#
  leave 
  ret 
#
  zero r0 
  leave 
  ret 

=main 
  enter 
  jmp ^_F_main 






