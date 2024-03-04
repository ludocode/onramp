@_F_main 
zero r0 
leave 
ret 

=main 
enter 
imw r9 0x4 
sub rsp rsp r9 
jmp ^_F_main 



@_F_foo 
stw r0 rfp 0xFFFFFFFC 
stw r1 rfp 0xFFFFFFF8 
stw r2 rfp 0xFFFFFFF4 
stw r3 rfp 0xFFFFFFF0 
zero r0 
leave 
ret 

=foo 
enter 
imw r9 0x30 
sub rsp rsp r9 
jmp ^_F_foo 



