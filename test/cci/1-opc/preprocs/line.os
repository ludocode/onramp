#line manual
#line 1 "./preprocs/line.i"
#line 25 "stdio.h"
#
#line 8 "foo.c"
@_F_main 
#
#
#line 12 "bar.c"
  imw r0 ^_Sx0 
  add r0 rpp r0 
  push r0 
  pop r0 
  call ^puts 
#
#
#line 15 "foo.c"
#
  zero r0 
  leave 
  ret 

=main 
  enter 
  jmp ^_F_main 



@_Sx0 
"Hello world!"'00




