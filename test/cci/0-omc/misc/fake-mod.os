@_F_mod 
stw r0 rfp 0xFFFFFFFC 
stw r1 rfp 0xFFFFFFF8 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
ldw r1 0 r1 
divs r0 r1 r0 
push r0 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
mul r0 r1 r0 
pop r1 
ldw r1 0 r1 
sub r0 r1 r0 
leave 
ret 
zero r0 
leave 
ret 

=mod 
enter 
imw r9 0x8 
sub rsp rsp r9 
jmp ^_F_mod 



@_F_main 
imw r0 7 
push r0 
imw r0 3 
push r0 
pop r1 
pop r0 
call ^mod 
push r0 
imw r0 1 
pop r1 
sub r0 r1 r0 
bool r0 r0 
jz r0 &_Lx0 
imw r0 1 
leave 
ret 
:_Lx0
imw r0 15 
push r0 
imw r0 4 
push r0 
pop r1 
pop r0 
call ^mod 
push r0 
imw r0 3 
pop r1 
sub r0 r1 r0 
bool r0 r0 
jz r0 &_Lx1 
imw r0 1 
leave 
ret 
:_Lx1
imw r0 500 
push r0 
imw r0 10 
push r0 
pop r1 
pop r0 
call ^mod 
push r0 
imw r0 0 
pop r1 
sub r0 r1 r0 
bool r0 r0 
jz r0 &_Lx2 
imw r0 1 
leave 
ret 
:_Lx2
imw r0 0 
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



