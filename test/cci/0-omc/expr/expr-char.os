@_F_foo 
mov r0 "f" 
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
mov r0 '0A
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
call ^foo 
push r0 
imw r0 102 
pop r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx0 
imw r0 1 
leave 
ret 
:_Lx0
call ^bar 
push r0 
imw r0 0x0A 
pop r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
sub r0 0x1 r0 
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



