#line manual
#line 1 "./global/global-define-char.i"
#
#
=foo 
'00



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






