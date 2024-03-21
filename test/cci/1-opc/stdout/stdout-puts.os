@_F_main 
  imw r0 ^_Sx0 
  add r0 rpp r0 
  push r0 
  ldw r0 rsp 0 
  call ^puts 
  add rsp rsp 4 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
@_Sx0 
"Hello world!"'00
