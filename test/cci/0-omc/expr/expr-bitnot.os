@_F_main 
imw r0 0xFFFFFFFF 
not r0 r0 
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
