@_F_foo 
zero r0 
leave 
ret 

@foo 
enter 
imw r9 0x0 
sub rsp rsp r9 
jmp ^_F_foo 



@_F_main 
zero r0 
leave 
ret 

=main 
enter 
imw r9 0x0 
sub rsp rsp r9 
jmp ^_F_main 



