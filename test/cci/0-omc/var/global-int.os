=x 
0 
@_F_foo 
imw r0 ^x 
add r0 rpp r0 
ldw r0 0 r0 
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
@_F_main 
imw r0 ^x 
add r0 rpp r0 
push r0 
imw r0 1 
pop r1 
stw r0 0 r1 
call ^foo 
isz r0 r0 
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
