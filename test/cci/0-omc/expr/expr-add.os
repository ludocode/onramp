@_F_main 
imw r0 1 
sub r0 0 r0 
push r0 
imw r0 10 
sub r0 0 r0 
pop r1 
add r0 r1 r0 
push r0 
imw r0 11 
sub r0 0 r0 
pop r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx0 
imw r0 1 
leave 
ret 
:_Lx0
imw r0 5 
push r0 
imw r0 3 
pop r1 
add r0 r1 r0 
push r0 
imw r0 8 
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



