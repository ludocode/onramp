@_F_foo 
imw r0 ^_Sx0
add r0 rpp r0 
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

@_Sx0
"foo"'00 



@_F_bar 
imw r0 ^_Sx1
add r0 rpp r0 
leave 
ret 
imw r0 ^_Sx2
add r0 rpp r0 
leave 
ret 
zero r0 
leave 
ret 

=bar 
enter 
imw r9 0x0 
sub rsp rsp r9 
jmp ^_F_bar 

@_Sx1
'22"b"'5C"ar"'09'0A'00 

@_Sx2
'0A'09'0B'00 



@_F_baz 
imw r0 ^_Sx3
add r0 rpp r0 
leave 
ret 
zero r0 
leave 
ret 

=baz 
enter 
imw r9 0x0 
sub rsp rsp r9 
jmp ^_F_baz 

@_Sx3
"baz"'00 



@_F_main 
imw r0 0 
push r0 
call ^foo 
push r0 
imw r0 ^_Sx4
add r0 rpp r0 
push r0 
pop r1 
pop r0 
call ^strcmp 
pop r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx0 
imw r0 1 
leave 
ret 
:_Lx0
imw r0 0 
push r0 
call ^baz 
push r0 
imw r0 ^_Sx5
add r0 rpp r0 
push r0 
pop r1 
pop r0 
call ^strcmp 
pop r1 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx1 
imw r0 1 
leave 
ret 
:_Lx1
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
call ^bar 
pop r1 
stw r0 0 r1 
imw r0 0x22 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 0 
pop r1 
ldw r1 0 r1 
add r0 r1 r0 
pop r1 
ldb r0 0 r0 
shl r0 r0 0x18 
shrs r0 r0 0x18 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx2 
imw r0 1 
leave 
ret 
:_Lx2
imw r0 0x62 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 1 
pop r1 
ldw r1 0 r1 
add r0 r1 r0 
pop r1 
ldb r0 0 r0 
shl r0 r0 0x18 
shrs r0 r0 0x18 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx3 
imw r0 1 
leave 
ret 
:_Lx3
imw r0 0x5c 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 2 
pop r1 
ldw r1 0 r1 
add r0 r1 r0 
pop r1 
ldb r0 0 r0 
shl r0 r0 0x18 
shrs r0 r0 0x18 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx4 
imw r0 1 
leave 
ret 
:_Lx4
imw r0 0x61 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 3 
pop r1 
ldw r1 0 r1 
add r0 r1 r0 
pop r1 
ldb r0 0 r0 
shl r0 r0 0x18 
shrs r0 r0 0x18 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx5 
imw r0 1 
leave 
ret 
:_Lx5
imw r0 0x72 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 4 
pop r1 
ldw r1 0 r1 
add r0 r1 r0 
pop r1 
ldb r0 0 r0 
shl r0 r0 0x18 
shrs r0 r0 0x18 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx6 
imw r0 1 
leave 
ret 
:_Lx6
imw r0 0x09 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 5 
pop r1 
ldw r1 0 r1 
add r0 r1 r0 
pop r1 
ldb r0 0 r0 
shl r0 r0 0x18 
shrs r0 r0 0x18 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx7 
imw r0 1 
leave 
ret 
:_Lx7
imw r0 0x0a 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 6 
pop r1 
ldw r1 0 r1 
add r0 r1 r0 
pop r1 
ldb r0 0 r0 
shl r0 r0 0x18 
shrs r0 r0 0x18 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx8 
imw r0 1 
leave 
ret 
:_Lx8
imw r0 0x00 
push r0 
imw r0 0xFFFFFFFC 
add r0 rfp r0 
push r0 
imw r0 7 
pop r1 
ldw r1 0 r1 
add r0 r1 r0 
pop r1 
ldb r0 0 r0 
shl r0 r0 0x18 
shrs r0 r0 0x18 
sub r0 r1 r0 
jz r0 0x1 
mov r0 0x1 
jz r0 &_Lx9 
imw r0 1 
leave 
ret 
:_Lx9
imw r0 0 
leave 
ret 
zero r0 
leave 
ret 

=main 
enter 
imw r9 0x4 
sub rsp rsp r9 
jmp ^_F_main 

@_Sx4
"foo"'00 

@_Sx5
"baz"'00 



