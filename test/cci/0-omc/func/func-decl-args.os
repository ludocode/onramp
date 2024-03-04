@_F_foo 
stw r0 rfp 0xFFFFFFFC 
stw r1 rfp 0xFFFFFFF8 
stw r2 rfp 0xFFFFFFF4 
zero r0 
leave 
ret 

=foo 
enter 
imw r9 0xC 
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



