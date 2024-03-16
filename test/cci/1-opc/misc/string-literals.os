#line manual
#line 1 "./misc/string-literals.i"
#
#
#
#
#
@_F_main 
#
  imw r0 ^_Sx0 
  add r0 rpp r0 
  push r0 
  imw r0 ^stdout 
  add r0 rpp r0 
  ldw r0 0 r0 
  push r0 
  ldw r0 rsp 4 
  ldw r1 rsp 0 
  call ^fputs 
  add rsp rsp 8 
#
  mov r0 " "
  push r0 
  ldw r0 rsp 0 
  call ^putchar 
  add rsp rsp 4 
#
  imw r0 ^_Sx1 
  add r0 rpp r0 
  push r0 
  ldw r0 rsp 0 
  call ^puts 
  add rsp rsp 4 
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

=main 
  enter 
  jmp ^_F_main 



@_Sx0 
"Hello"'00

@_Sx1 
"world!"'00



@_F_foo 
#
#
  zero r0 
  leave 
  ret 

=foo 
  enter 
  jmp ^_F_foo 






