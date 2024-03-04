@_F_main 
imw r0 ^_Sx0
add r0 rpp r0 
push r0 
pop r0 
call ^puts 
zero r0 
leave 
ret 

=main 
enter 
imw r9 0x0 
sub rsp rsp r9 
jmp ^_F_main 

@_Sx0
"Hello world!"'00 



