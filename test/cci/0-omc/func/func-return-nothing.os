@_F_foo 
stw r0 rfp 0xFFFFFFFC 
zero r0 
leave 
ret 
zero r0 
leave 
ret 

=foo 
enter 
imw r9 0x4 
sub rsp rsp r9 
jmp ^_F_foo 



@_F_bar 
zero r0 
leave 
ret 
zero r0 
leave 
ret 

@bar 
enter 
imw r9 0x0 
sub rsp rsp r9 
jmp ^_F_bar 



@_F_main 
stw r0 rfp 0xFFFFFFFC 
stw r1 rfp 0xFFFFFFF8 
zero r0 
leave 
ret 
zero r0 
leave 
ret 
zero r0 
leave 
ret 
zero r0 
leave 
ret 

=main 
enter 
imw r9 0x8 
sub rsp rsp r9 
jmp ^_F_main 



