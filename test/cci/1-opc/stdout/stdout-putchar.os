#line manual
#line 1 "./stdout/stdout-putchar.i"
#
#
@_F_main 
#
  imw r0 0x61 
  push r0 
  pop r0 
  call ^putchar 
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






