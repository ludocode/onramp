#line manual
#line 1 "./function/function-ten-args.i"
#
#
@_F_foo 
  stw r0 rfp -4 
  stw r1 rfp -8 
  stw r2 rfp -12 
  stw r3 rfp -16 
  ldw r9 rfp 8 
  stw r9 rfp -20 
  ldw r9 rfp 12 
  stw r9 rfp -24 
  ldw r9 rfp 16 
  stw r9 rfp -28 
  ldw r9 rfp 20 
  stw r9 rfp -32 
  ldw r9 rfp 24 
  stw r9 rfp -36 
  ldw r9 rfp 28 
  stw r9 rfp -40 
#
  add r0 rfp -4 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
#
:_Lx0 
  add r0 rfp -8 
  push r0 
  imw r0 2 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 2 
  leave 
  ret 
#
:_Lx1 
  add r0 rfp -12 
  push r0 
  imw r0 3 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 3 
  leave 
  ret 
#
:_Lx2 
  add r0 rfp -16 
  push r0 
  imw r0 4 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
  imw r0 4 
  leave 
  ret 
#
:_Lx3 
  add r0 rfp -20 
  push r0 
  imw r0 5 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
  imw r0 5 
  leave 
  ret 
#
:_Lx4 
  add r0 rfp -24 
  push r0 
  imw r0 6 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx5 
  imw r0 6 
  leave 
  ret 
#
:_Lx5 
  add r0 rfp -28 
  push r0 
  imw r0 7 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx6 
  imw r0 7 
  leave 
  ret 
#
:_Lx6 
  add r0 rfp -32 
  push r0 
  imw r0 8 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx7 
  imw r0 8 
  leave 
  ret 
#
:_Lx7 
  add r0 rfp -36 
  push r0 
  imw r0 9 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx8 
  imw r0 9 
  leave 
  ret 
#
:_Lx8 
  add r0 rfp -40 
  push r0 
  imw r0 10 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx9 
  imw r0 10 
  leave 
  ret 
#
:_Lx9 
  imw r0 0 
  leave 
  ret 
#
#
#
  zero r0 
  leave 
  ret 

=foo 
  enter 
  sub rsp rsp 40 
  jmp ^_F_foo 





@_F_main 
#
  imw r0 1 
  push r0 
  imw r0 2 
  push r0 
  imw r0 3 
  push r0 
  imw r0 4 
  push r0 
  imw r0 5 
  push r0 
  imw r0 6 
  push r0 
  imw r0 7 
  push r0 
  imw r0 8 
  push r0 
  imw r0 9 
  push r0 
  imw r0 10 
  push r0 
  ldw r0 rsp 36 
  ldw r1 rsp 32 
  ldw r2 rsp 28 
  ldw r3 rsp 24 
  ldw r9 rsp 0 
  push r9 
  ldw r9 rsp 8 
  push r9 
  ldw r9 rsp 16 
  push r9 
  ldw r9 rsp 24 
  push r9 
  ldw r9 rsp 32 
  push r9 
  ldw r9 rsp 40 
  push r9 
  call ^foo 
  add rsp rsp 64 
  leave 
  ret 
#
#
  zero r0 
  leave 
  ret 

=main 
  enter 
  jmp ^_F_main 






