=foo 
0 



@_F_main 
imw r0 ^foo 
add r0 rpp r0 
ldw r0 0 r0 
leave 
ret 
zero r0 
leave 
ret 

=main 
enter 
imw r9 0x0 
sub rsp rsp r9 
jmp ^_F_main 



