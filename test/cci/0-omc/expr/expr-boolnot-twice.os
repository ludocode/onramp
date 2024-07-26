@_F_main 
imw r0 0 
isz r0 r0 
isz r0 r0 
jz r0 &_Lx0 
imw r0 1 
leave 
ret 
:_Lx0
imw r0 0x100 
sxb r0 r0 
isz r0 r0 
isz r0 r0 
jz r0 &_Lx1 
imw r0 2 
leave 
ret 
:_Lx1
imw r0 3 
sxb r0 r0 
isz r0 r0 
isz r0 r0 
jz r0 &_Lx2 
imw r0 1 
isz r0 r0 
isz r0 r0 
jz r0 &_Lx3 
imw r0 0 
leave 
ret 
:_Lx3
:_Lx2
imw r0 3 
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
