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



@_F_main 
call ^foo 
push r0 
imw r0 ^_Sx3
add r0 rpp r0 
push r0 
pop r1 
pop r0 
call ^strcmp 
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

@_Sx3
"foo"'00 



