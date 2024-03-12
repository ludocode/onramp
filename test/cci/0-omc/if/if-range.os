@_F_main 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 12 
pop r1 
stw r0 0 r1 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 0x8000 
sub r0 0 r0 
pop r1 
ldw r1 0 r1 
cmps r0 r1 r0 
sub r0 r0 0xFFFFFFFF 
isz r0 r0 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 0xFFFF 
pop r1 
ldw r1 0 r1 
cmps r0 r1 r0 
sub r0 r0 0x1 
isz r0 r0 
pop r1 
or r0 r1 r0 
jz r0 &_Lx0 
imw r0 1 
leave 
ret 
:_Lx0
imw r0 0 
leave 
ret 
zero r0 
leave 
ret 

=main 
enter 
imw r9 0x4 
sub rsp rsp r9 
jmp ^_F_main 



