@queens 
'00'00'00'00 '00'00'00'00
@_F_main 
  imw r0 0 
  push r0 
  ldw r0 rsp 0 
  call ^place_queen 
  add rsp rsp 4 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
@_F_is_queen_valid 
  stw r0 rfp -4 
  stw r1 rfp -8 
  add r0 rfp -12 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
:_Lx2 
  add r0 rfp -12 
  push r0 
  add r0 rfp -4 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  cmps r0 r1 r0 
  cmpu r0 r0 -1 
  add r0 r0 1 
  and r0 r0 1 
  jz r0 &_Lx1 
  jmp &_Lx3 
:_Lx0 
  add r0 rfp -12 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  jmp &_Lx2 
:_Lx3 
  add r0 rfp -16 
  push r0 
  imw r0 ^queens 
  add r0 rpp r0 
  push r0 
  add r0 rfp -12 
  pop r1 
  ldw r0 0 r0 
  add r0 r1 r0 
  pop r1 
  ldb r0 0 r0 
  sxb r0 r0 
  stw r0 0 r1 
  add r0 rfp -20 
  push r0 
  add r0 rfp -4 
  push r0 
  add r0 rfp -12 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  sub r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -16 
  push r0 
  add r0 rfp -8 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  add r0 r0 1 
  and r0 r0 1 
  jnz r0 &_Lx5 
  add r0 rfp -16 
  push r0 
  add r0 rfp -8 
  push r0 
  add r0 rfp -20 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  sub r0 r1 r0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  add r0 r0 1 
  and r0 r0 1 
  jnz r0 &_Lx5 
  add r0 rfp -16 
  push r0 
  add r0 rfp -8 
  push r0 
  add r0 rfp -20 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  add r0 r0 1 
  and r0 r0 1 
:_Lx5 
  bool r0 r0 
  jz r0 &_Lx4 
  imw r0 0 
  leave 
  ret 
:_Lx4 
  jmp &_Lx0 
:_Lx1 
  imw r0 1 
  leave 
  ret 
  zero r0 
  leave 
  ret 
@is_queen_valid 
  enter 
  sub rsp rsp 20 
  jmp ^_F_is_queen_valid 
@_F_place_queen 
  stw r0 rfp -4 
  add r0 rfp -4 
  push r0 
  imw r0 8 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  add r0 r0 1 
  and r0 r0 1 
  jz r0 &_Lx6 
  call ^print_board 
  add rsp rsp 0 
  leave 
  ret 
:_Lx6 
  add r0 rfp -8 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
:_Lx9 
  add r0 rfp -8 
  push r0 
  imw r0 8 
  pop r1 
  ldw r1 0 r1 
  cmps r0 r1 r0 
  cmpu r0 r0 -1 
  add r0 r0 1 
  and r0 r0 1 
  jz r0 &_Lx8 
  jmp &_LxA 
:_Lx7 
  add r0 rfp -8 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  jmp &_Lx9 
:_LxA 
  add r0 rfp -4 
  ldw r0 0 r0 
  push r0 
  add r0 rfp -8 
  ldw r0 0 r0 
  push r0 
  ldw r0 rsp 4 
  ldw r1 rsp 0 
  call ^is_queen_valid 
  add rsp rsp 8 
  jz r0 &_LxB 
  imw r0 ^queens 
  add r0 rpp r0 
  push r0 
  add r0 rfp -4 
  pop r1 
  ldw r0 0 r0 
  add r0 r1 r0 
  push r0 
  add r0 rfp -8 
  ldw r0 0 r0 
  sxb r0 r0 
  pop r1 
  stb r0 0 r1 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  add r0 r1 r0 
  push r0 
  ldw r0 rsp 0 
  call ^place_queen 
  add rsp rsp 4 
:_LxB 
  jmp &_Lx7 
:_Lx8 
  zero r0 
  leave 
  ret 
@place_queen 
  enter 
  sub rsp rsp 8 
  jmp ^_F_place_queen 
@_F_print_board 
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
:_LxE 
  add r0 rfp -4 
  push r0 
  imw r0 8 
  pop r1 
  ldw r1 0 r1 
  cmps r0 r1 r0 
  cmpu r0 r0 -1 
  add r0 r0 1 
  and r0 r0 1 
  jz r0 &_LxD 
  jmp &_LxF 
:_LxC 
  add r0 rfp -4 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  jmp &_LxE 
:_LxF 
  add r0 rfp -8 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
:_Lx12 
  add r0 rfp -8 
  push r0 
  imw r0 8 
  pop r1 
  ldw r1 0 r1 
  cmps r0 r1 r0 
  cmpu r0 r0 -1 
  add r0 r0 1 
  and r0 r0 1 
  jz r0 &_Lx11 
  jmp &_Lx13 
:_Lx10 
  add r0 rfp -8 
  push r0 
  mov r1 r0 
  imw r0 1 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  jmp &_Lx12 
:_Lx13 
  imw r0 ^queens 
  add r0 rpp r0 
  push r0 
  add r0 rfp -8 
  pop r1 
  ldw r0 0 r0 
  add r0 r1 r0 
  push r0 
  add r0 rfp -4 
  pop r1 
  ldw r0 0 r0 
  ldb r1 0 r1 
  sxb r1 r1 
  cmpu r0 r1 r0 
  add r0 r0 1 
  and r0 r0 1 
  jz r0 &_Lx14 
  mov r0 "Q"
  jmp &_Lx15 
:_Lx14 
  mov r0 "."
:_Lx15 
  push r0 
  ldw r0 rsp 0 
  call ^putchar 
  add rsp rsp 4 
  mov r0 " "
  push r0 
  ldw r0 rsp 0 
  call ^putchar 
  add rsp rsp 4 
  jmp &_Lx10 
:_Lx11 
  mov r0 '0A
  push r0 
  ldw r0 rsp 0 
  call ^putchar 
  add rsp rsp 4 
  jmp &_LxC 
:_LxD 
  imw r0 ^_Sx0 
  add r0 rpp r0 
  push r0 
  ldw r0 rsp 0 
  call ^puts 
  add rsp rsp 4 
  zero r0 
  leave 
  ret 
@print_board 
  enter 
  sub rsp rsp 8 
  jmp ^_F_print_board 
@_Sx0 
'0A'00
