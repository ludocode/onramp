@_F_foo 
stw r0 rfp 0xFFFFFFFC 
imw r0 2 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
mul r0 r1 r0 
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
stw r0 rfp 0xFFFFFFFC 
stw r1 rfp 0xFFFFFFF8 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
ldw r1 0 r1 
add r0 r1 r0 
leave 
ret 
zero r0 
leave 
ret 
=bar 
enter 
imw r9 0x8 
sub rsp rsp r9 
jmp ^_F_bar 
@_F_main 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 0 
pop r1 
stw r0 0 r1 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 0 
pop r1 
stw r0 0 r1 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 4 
pop r1 
stw r0 0 r1 
push r0 
pop r0 
call ^foo 
push r0 
imw r0 8 
pop r1 
sub r0 r1 r0 
bool r0 r0 
jz r0 &_Lx0 
imw r0 1 
leave 
ret 
:_Lx0
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 4 
pop r1 
ldw r1 0 r1 
sub r0 r1 r0 
bool r0 r0 
jz r0 &_Lx1 
imw r0 1 
leave 
ret 
:_Lx1
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 3 
pop r1 
stw r0 0 r1 
push r0 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 7 
pop r1 
stw r0 0 r1 
push r0 
pop r1 
pop r0 
call ^bar 
push r0 
imw r0 10 
pop r1 
sub r0 r1 r0 
bool r0 r0 
jz r0 &_Lx2 
imw r0 1 
leave 
ret 
:_Lx2
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 3 
pop r1 
ldw r1 0 r1 
sub r0 r1 r0 
bool r0 r0 
jz r0 &_Lx3 
imw r0 1 
leave 
ret 
:_Lx3
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 7 
pop r1 
ldw r1 0 r1 
sub r0 r1 r0 
bool r0 r0 
jz r0 &_Lx4 
imw r0 1 
leave 
ret 
:_Lx4
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
