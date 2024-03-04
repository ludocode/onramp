#line manual
#line 1 "./function/function-empty-noargs.i"
@_F_foo 
#
  imw r0 0 
#
  leave 
  ret 
#
#
  zero r0 
  leave 
  ret 

=foo 
  enter 
  jmp ^_F_foo 





@_F_main 
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






