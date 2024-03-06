=queens 
0 



@_F_main 
imw r0 ^queens 
add r0 rpp r0 
push r0 
imw r0 8 
push r0 
pop r0 
call ^malloc 
pop r1 
stw r0 0 r1 
imw r0 0 
push r0 
pop r0 
call ^place_queen 
imw r0 ^queens 
add r0 rpp r0 
ldw r0 0 r0 
push r0 
pop r0 
call ^free 
zero r0 
leave 
ret 

=main 
enter 
imw r9 0x0 
sub rsp rsp r9 
jmp ^_F_main 



@_F_is_queen_valid 
stw r0 rfp 0xFFFFFFFC 
stw r1 rfp 0xFFFFFFF8 
imw r0 0xFFFFFFF4 
add r0 rfp r0 
push r0 
imw r0 0 
pop r1 
stw r0 0 r1 
:_Lx0
imw r0 0xFFFFFFF4 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
ldw r1 0 r1 
cmps r0 r1 r0 
sub r0 r0 0xFFFFFFFF 
jz r0 0x1 
mov r0 0x1 
sub r0 0x1 r0 
jz r0 &_Lx1 
imw r0 0xFFFFFFF0 
add r0 rfp r0 
push r0 
imw r0 ^queens 
add r0 rpp r0 
push r0 
imw r0 0xFFFFFFF4 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
ldw r1 0 r1 
add r0 r1 r0 
pop r1 
ldb r0 0 r0 
shl r0 r0 0x18 
shrs r0 r0 0x18 
stw r0 0 r1 
imw r0 0xFFFFFFEC 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFF4 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
ldw r1 0 r1 
sub r0 r1 r0 
pop r1 
stw r0 0 r1 
imw r0 0xFFFFFFF0 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
ldw r1 0 r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
sub r0 0x1 r0 
push r0 
imw r0 0xFFFFFFF0 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFEC 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
ldw r1 0 r1 
sub r0 r1 r0 
pop r1 
ldw r1 0 r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
sub r0 0x1 r0 
pop r1 
or r0 r1 r0 
push r0 
imw r0 0xFFFFFFF0 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFEC 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
ldw r1 0 r1 
add r0 r1 r0 
pop r1 
ldw r1 0 r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
sub r0 0x1 r0 
pop r1 
or r0 r1 r0 
jz r0 &_Lx2 
imw r0 0 
leave 
ret 
:_Lx2
imw r0 0xFFFFFFF4 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFF4 
add r0 rfp r0 
push r0 
imw r0 1 
pop r1 
ldw r1 0 r1 
add r0 r1 r0 
pop r1 
stw r0 0 r1 
jmp &_Lx0 
:_Lx1
imw r0 1 
leave 
ret 
zero r0 
leave 
ret 

=is_queen_valid 
enter 
imw r9 0x14 
sub rsp rsp r9 
jmp ^_F_is_queen_valid 



@_F_place_queen 
stw r0 rfp 0xFFFFFFFC 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 8 
pop r1 
ldw r1 0 r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
sub r0 0x1 r0 
jz r0 &_Lx3 
call ^print_board 
zero r0 
leave 
ret 
:_Lx3
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 0 
pop r1 
stw r0 0 r1 
:_Lx4
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 8 
pop r1 
ldw r1 0 r1 
cmps r0 r1 r0 
sub r0 r0 0xFFFFFFFF 
jz r0 0x1 
mov r0 0x1 
sub r0 0x1 r0 
jz r0 &_Lx5 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
ldw r0 0 r0 
push r0 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
ldw r0 0 r0 
push r0 
pop r1 
pop r0 
call ^is_queen_valid 
jz r0 &_Lx6 
imw r0 ^queens 
add r0 rpp r0 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
ldw r1 0 r1 
add r0 r1 r0 
push r0 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
shl r0 r0 0x18 
shrs r0 r0 0x18 
stb r0 0 r1 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 1 
pop r1 
ldw r1 0 r1 
add r0 r1 r0 
push r0 
pop r0 
call ^place_queen 
:_Lx6
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 1 
pop r1 
ldw r1 0 r1 
add r0 r1 r0 
pop r1 
stw r0 0 r1 
jmp &_Lx4 
:_Lx5
zero r0 
leave 
ret 

=place_queen 
enter 
imw r9 0x8 
sub rsp rsp r9 
jmp ^_F_place_queen 



@_F_print_board 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 0 
pop r1 
stw r0 0 r1 
:_Lx7
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 8 
pop r1 
ldw r1 0 r1 
cmps r0 r1 r0 
sub r0 r0 0xFFFFFFFF 
jz r0 0x1 
mov r0 0x1 
sub r0 0x1 r0 
jz r0 &_Lx8 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 0 
pop r1 
stw r0 0 r1 
:_Lx9
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 8 
pop r1 
ldw r1 0 r1 
cmps r0 r1 r0 
sub r0 r0 0xFFFFFFFF 
jz r0 0x1 
mov r0 0x1 
sub r0 0x1 r0 
jz r0 &_LxA 
imw r0 0xFFFFFFF4 
add r0 rfp r0 
push r0 
imw r0 ^queens 
add r0 rpp r0 
push r0 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
ldw r1 0 r1 
add r0 r1 r0 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
pop r1 
ldw r0 0 r0 
ldb r1 0 r1 
shl r1 r1 0x18 
shrs r1 r1 0x18 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
sub r0 0x1 r0 
pop r1 
stw r0 0 r1 
imw r0 0xFFFFFFF4 
add r0 rfp r0 
ldw r0 0 r0 
jz r0 &_LxB 
mov r0 "Q" 
push r0 
pop r0 
call ^putchar 
:_LxB
imw r0 0xFFFFFFF4 
add r0 rfp r0 
ldw r0 0 r0 
jz r0 0x1 
mov r0 0x1 
sub r0 0x1 r0 
jz r0 &_LxC 
mov r0 "." 
push r0 
pop r0 
call ^putchar 
:_LxC
mov r0 " " 
push r0 
pop r0 
call ^putchar 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFF8 
add r0 rfp r0 
push r0 
imw r0 1 
pop r1 
ldw r1 0 r1 
add r0 r1 r0 
pop r1 
stw r0 0 r1 
jmp &_Lx9 
:_LxA
mov r0 '0A
push r0 
pop r0 
call ^putchar 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 1 
pop r1 
ldw r1 0 r1 
add r0 r1 r0 
pop r1 
stw r0 0 r1 
jmp &_Lx7 
:_Lx8
mov r0 '0A
push r0 
pop r0 
call ^putchar 
mov r0 '0A
push r0 
pop r0 
call ^putchar 
zero r0 
leave 
ret 

=print_board 
enter 
imw r9 0xC 
sub rsp rsp r9 
jmp ^_F_print_board 



