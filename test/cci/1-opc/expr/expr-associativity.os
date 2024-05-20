@_F_main 
  imw r0 5 
  push r0 
  imw r0 3 
  pop r1 
  sub r0 r1 r0 
  push r0 
  imw r0 2 
  pop r1 
  sub r0 r1 r0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
