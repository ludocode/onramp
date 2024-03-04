@_F_foo 
stw r0 rfp 0xFFFFFFFC 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
ldw r0 0 r0 
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
imw r0 0 
push r0 
pop r0 
call ^foo 
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
call ^bar 
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



