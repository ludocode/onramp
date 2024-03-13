#line manual
#line 1 "./function/function-empty-two-args.i"
@_F_foo 
  stw r0 rfp -4 
  stw r1 rfp -8 
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
  sub rsp rsp 8 
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






