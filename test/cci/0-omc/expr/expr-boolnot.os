@_F_main 
imw r0 1 
jz r0 0x1 
mov r0 0x1 
sub r0 0x1 r0 
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



