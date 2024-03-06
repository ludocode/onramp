@_F_main 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 ^_Sx0
add r0 rpp r0 
pop r1 
stw r0 0 r1 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 4 
pop r1 
ldw r1 0 r1 
add r0 r1 r0 
push r0 
mov r0 "o" 
pop r1 
ldb r1 0 r1 
shl r1 r1 0x18 
shrs r1 r1 0x18 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx0 
imw r0 1 
leave 
ret 
:_Lx0
imw r0 8 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
add r0 r1 r0 
push r0 
mov r0 "r" 
pop r1 
ldb r1 0 r1 
shl r1 r1 0x18 
shrs r1 r1 0x18 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx1 
imw r0 2 
leave 
ret 
:_Lx1
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 4 
pop r1 
ldw r1 0 r1 
add r0 r1 r0 
pop r1 
stw r0 0 r1 
imw r0 0xFFFFFFF4 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 8 
pop r1 
ldw r1 0 r1 
add r0 r1 r0 
pop r1 
stw r0 0 r1 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
ldw r0 0 r0 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
ldw r0 0 r0 
pop r1 
sub r0 r1 r0 
shrs r0 r0 0x2 
push r0 
imw r0 1 
pop r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx2 
imw r0 3 
leave 
ret 
:_Lx2
imw r0 0xFFFFFFFC 
add r0 rfp r0 
ldw r0 0 r0 
push r0 
imw r0 0xFFFFFFF4 
add r0 rfp r0 
ldw r0 0 r0 
pop r1 
sub r0 r1 r0 
shrs r0 r0 0x2 
push r0 
imw r0 2 
sub r0 0 r0 
pop r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx3 
imw r0 4 
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
imw r9 0xC 
sub rsp rsp r9 
jmp ^_F_main 

@_Sx0
"Hello world!"'00 



