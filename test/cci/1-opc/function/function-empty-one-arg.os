#line manual
#line 1 "./function/function-empty-one-arg.i"
@_F_foo 
  stw r0 rfp -4 
#
  imw r0 0 
  leave 
  ret 
#
#
#
  zero r0 
  leave 
  ret 

=foo 
  enter 
  sub rsp rsp 4 
  jmp ^_F_foo 





@_F_main 
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






