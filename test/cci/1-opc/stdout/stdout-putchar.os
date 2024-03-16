#line manual
#line 1 "./stdout/stdout-putchar.i"
#
#
@_F_main 
#
  imw r0 0x61 
  push r0 
  ldw r0 rsp 0 
  call ^putchar 
  add rsp rsp 4 
#
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






