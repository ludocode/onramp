=write_number_buffer 
0 



=write_number_buffer_capacity 
0 



@_F_write_number 
stw r0 rfp 0xFFFFFFFC 
stw r1 rfp 0xFFFFFFF8 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 0 
pop r1 
ldw r1 0 r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
sub r0 0x1 r0 
jz r0 &_Lx0 
imw r0 ^_Sx0
add r0 rpp r0 
push r0 
imw r0 1 
push r0 
imw r0 1 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
ldw r0 0 r0 
push r0 
pop r3 
pop r2 
pop r1 
pop r0 
call ^fwrite 
zero r0 
leave 
ret 
:_Lx0
imw r0 0xFFFFFFF4 
add r0 rfp r0 
push r0 
imw r0 ^write_number_buffer 
add r0 rpp r0 
pop r1 
ldw r0 0 r0 
stw r0 0 r1 
imw r0 0xFFFFFFF0 
add r0 rfp r0 
push r0 
imw r0 ^write_number_buffer_capacity 
add r0 rpp r0 
push r0 
imw r0 1 
pop r1 
ldw r1 0 r1 
sub r0 r1 r0 
pop r1 
stw r0 0 r1 
:_Lx1
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 0 
pop r1 
ldw r1 0 r1 
cmps r0 r1 r0 
sub r0 r0 0x1 
jz r0 0x1 
mov r0 0x1 
sub r0 0x1 r0 
jz r0 &_Lx2 
imw r0 0xFFFFFFF0 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFF0 
add r0 rfp r0 
push r0 
imw r0 1 
pop r1 
ldw r1 0 r1 
sub r0 r1 r0 
pop r1 
stw r0 0 r1 
imw r0 0xFFFFFFEC 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 10 
pop r1 
ldw r1 0 r1 
divs r0 r1 r0 
push r0 
imw r0 10 
pop r1 
mul r0 r1 r0 
pop r1 
ldw r1 0 r1 
sub r0 r1 r0 
pop r1 
stw r0 0 r1 
imw r0 0xFFFFFFF4 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFF0 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
ldw r1 0 r1 
add r0 r1 r0 
push r0 
mov r0 "0" 
push r0 
imw r0 0xFFFFFFEC 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
add r0 r1 r0 
pop r1 
shl r0 r0 0x18 
shrs r0 r0 0x18 
stb r0 0 r1 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 10 
pop r1 
ldw r1 0 r1 
divs r0 r1 r0 
pop r1 
stw r0 0 r1 
jmp &_Lx1 
:_Lx2
imw r0 0xFFFFFFEC 
add r0 rfp r0 
push r0 
imw r0 ^write_number_buffer_capacity 
add r0 rpp r0 
push r0 
imw r0 0xFFFFFFF0 
add r0 rfp r0 
push r0 
imw r0 1 
pop r1 
ldw r1 0 r1 
add r0 r1 r0 
pop r1 
ldw r1 0 r1 
sub r0 r1 r0 
pop r1 
stw r0 0 r1 
imw r0 0xFFFFFFF4 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFF0 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
ldw r1 0 r1 
add r0 r1 r0 
push r0 
imw r0 1 
push r0 
imw r0 0xFFFFFFEC 
add r0 rfp r0 
ldw r0 0 r0 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
ldw r0 0 r0 
push r0 
pop r3 
pop r2 
pop r1 
pop r0 
call ^fwrite 
zero r0 
leave 
ret 

@write_number 
enter 
imw r9 0x14 
sub rsp rsp r9 
jmp ^_F_write_number 

@_Sx0
"0"'00 



@_F_main 
imw r0 ^write_number_buffer_capacity 
add r0 rpp r0 
push r0 
imw r0 32 
pop r1 
stw r0 0 r1 
imw r0 ^write_number_buffer 
add r0 rpp r0 
push r0 
imw r0 ^write_number_buffer_capacity 
add r0 rpp r0 
ldw r0 0 r0 
push r0 
pop r0 
call ^malloc 
pop r1 
stw r0 0 r1 
imw r0 ^stdout 
add r0 rpp r0 
ldw r0 0 r0 
push r0 
imw r0 54081 
push r0 
pop r1 
pop r0 
call ^write_number 
mov r0 '0A
push r0 
pop r0 
call ^putchar 
zero r0 
leave 
ret 

=main 
enter 
imw r9 0x0 
sub rsp rsp r9 
jmp ^_F_main 



