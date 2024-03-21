@_F_main 
imw r0 6 
push r0 
imw r0 2 
push r0 
imw r0 3 
pop r1 
mul r0 r1 r0 
pop r1 
sub r0 r1 r0 
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
