@_F_main 
imw r0 19 
push r0 
imw r0 7 
pop r1 
mods r0 r1 r0 
push r0 
imw r0 5 
pop r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx0 
imw r0 1 
leave 
ret 
:_Lx0
imw r0 19 
sub r0 0 r0 
push r0 
imw r0 7 
pop r1 
mods r0 r1 r0 
push r0 
imw r0 5 
sub r0 0 r0 
pop r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx1 
imw r0 1 
leave 
ret 
:_Lx1
imw r0 19 
push r0 
imw r0 7 
sub r0 0 r0 
pop r1 
mods r0 r1 r0 
push r0 
imw r0 5 
pop r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx2 
imw r0 1 
leave 
ret 
:_Lx2
imw r0 19 
sub r0 0 r0 
push r0 
imw r0 7 
sub r0 0 r0 
pop r1 
mods r0 r1 r0 
push r0 
imw r0 5 
sub r0 0 r0 
pop r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx3 
imw r0 1 
leave 
ret 
:_Lx3
imw r0 2147483647 
sub r0 0 r0 
push r0 
imw r0 1 
pop r1 
sub r0 r1 r0 
push r0 
imw r0 7919 
pop r1 
mods r0 r1 r0 
push r0 
imw r0 1309 
sub r0 0 r0 
pop r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
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
imw r9 0x0 
sub rsp rsp r9 
jmp ^_F_main 



