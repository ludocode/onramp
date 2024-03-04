=word 
0 



=words 
0 



@_F_main 
imw r0 ^words 
add r0 rpp r0 
push r0 
imw r0 ^word 
add r0 rpp r0 
pop r1 
stw r0 0 r1 
imw r0 ^words 
add r0 rpp r0 
ldw r0 0 r0 
push r0 
imw r0 ^_Sx0
add r0 rpp r0 
pop r1 
stw r0 0 r1 
imw r0 ^word 
add r0 rpp r0 
ldw r0 0 r0 
push r0 
pop r0 
call ^puts 
imw r0 ^words 
add r0 rpp r0 
push r0 
imw r0 0 
pop r1 
ldw r1 0 r1 
shl r0 r0 0x2 
add r0 r1 r0 
ldw r0 0 r0 
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
"Hello"'00 



