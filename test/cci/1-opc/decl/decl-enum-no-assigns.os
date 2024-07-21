@x 
0 
@y 
1 
@z 
2 
@_F_main 
  imw r0 ^x 
  add r0 rpp r0 
  ldw r0 0 r0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
