@_F_main 
imw r0 0 
jz r0 &_Lx0 
imw r0 1 
leave 
ret 
:_Lx0
imw r0 1 
jz r0 &_Lx1 
imw r0 0 
leave 
ret 
:_Lx1
imw r0 1 
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
