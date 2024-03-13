#line manual
#line 1 "./function/function-empty-ten-args.i"
#
#
@_F_foo 
  stw r0 rfp -4 
  stw r1 rfp -8 
  stw r2 rfp -12 
  stw r3 rfp -16 
  ldw r9 rfp 4 
  stw r9 rfp -20 
  ldw r9 rfp 8 
  stw r9 rfp -24 
  ldw r9 rfp 12 
  stw r9 rfp -28 
  ldw r9 rfp 16 
  stw r9 rfp -32 
  ldw r9 rfp 20 
  stw r9 rfp -36 
  ldw r9 rfp 24 
  stw r9 rfp -40 
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
  sub rsp rsp 40 
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






