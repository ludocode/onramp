@_F_main 
  jmp &_U_4_main_foo
  imw r0 1 
  leave 
  ret 
:_U_4_main_foo
  add r0 rfp -4 
  push r0 
  imw r0 0 
  pop r1 
  stw r0 0 r1 
  jmp &_U_4_main_bar
  imw r0 2 
  leave 
  ret 
:_Lx0 
  imw r0 1 
  jz r0 &_Lx1 
  jmp &_Lx1 
:_U_4_main_bar
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  stw r0 0 r1 
  jmp &_Lx0 
:_Lx1 
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 3 
  leave 
  ret 
:_Lx2 
  imw r0 0 
  jz r0 &_Lx3 
:_U_4_main_baz
  imw r0 4 
  leave 
  ret 
:_Lx3 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  sub rsp rsp 4 
  jmp ^_F_main 
