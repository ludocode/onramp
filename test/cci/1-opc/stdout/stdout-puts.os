#line manual
#line 1 "./stdout/stdout-puts.i"
#
#
@_F_main 
#
  imw r0 ^_Sx0 
  add r0 rpp r0 
  push r0 
  pop r0 
  call ^puts 
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



@_Sx0 
"Hello world!"'00




