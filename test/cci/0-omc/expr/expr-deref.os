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
ldw r0 0 r0 
ldb r0 0 r0 
push r0 
pop r0 
call ^putchar 
zero r0 
leave 
ret 

=main 
enter 
imw r9 0x4 
sub rsp rsp r9 
jmp ^_F_main 

@_Sx0
"Hello"'00 



