@_F_foo 
stw r0 rfp 0xFFFFFFFC 
zero r0 
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
@_F_baz 
stw r0 rfp 0xFFFFFFFC 
imw r0 1 
leave 
ret 
zero r0 
leave 
ret 
=baz 
enter 
imw r9 0x4 
sub rsp rsp r9 
jmp ^_F_baz 
@_F_bar 
zero r0 
leave 
ret 
zero r0 
leave 
ret 
@bar 
enter 
imw r9 0x0 
sub rsp rsp r9 
jmp ^_F_bar 
@_F_main 
stw r0 rfp 0xFFFFFFFC 
stw r1 rfp 0xFFFFFFF8 
imw r0 0 
leave 
ret 
zero r0 
leave 
ret 
imw r0 0xFFFFFFF4 
add r0 rfp r0 
push r0 
imw r0 2 
pop r1 
sxb r0 r0 
stb r0 0 r1 
imw r0 0xFFFFFFF4 
add r0 rfp r0 
ldb r0 0 r0 
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
