@_F_foo 
imw r0 0x12345678 
leave 
ret 
zero r0 
leave 
ret 

=foo 
enter 
imw r9 0x0 
sub rsp rsp r9 
jmp ^_F_foo 



@_F_bar 
imw r0 4567 
leave 
ret 
zero r0 
leave 
ret 

=bar 
enter 
imw r9 0x0 
sub rsp rsp r9 
jmp ^_F_bar 



@_F_main 
imw r0 0 
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



