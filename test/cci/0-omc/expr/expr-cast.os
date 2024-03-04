@_F_main 
imw r0 0x123 
shl r0 r0 0x18 
shrs r0 r0 0x18 
push r0 
imw r0 0x23 
pop r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx0 
imw r0 1 
leave 
ret 
:_Lx0
imw r0 0x2aa 
shl r0 r0 0x18 
shrs r0 r0 0x18 
push r0 
imw r0 0xffffffaa 
pop r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx1 
imw r0 1 
leave 
ret 
:_Lx1
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 2 
pop r1 
shl r0 r0 0x18 
shrs r0 r0 0x18 
stb r0 0 r1 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
ldb r0 0 r0 
shl r0 r0 0x18 
shrs r0 r0 0x18 
push r0 
imw r0 3 
pop r1 
sub r0 r1 r0 
push r0 
imw r0 1 
sub r0 0 r0 
pop r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx2 
imw r0 1 
leave 
ret 
:_Lx2
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 0x123 
pop r1 
stw r0 0 r1 
imw r0 0x23 
push r0 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
ldw r0 0 r0 
shl r0 r0 0x18 
shrs r0 r0 0x18 
pop r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx3 
imw r0 1 
leave 
ret 
:_Lx3
imw r0 0 
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



