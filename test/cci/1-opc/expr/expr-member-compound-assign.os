=f 
'00'00'00'00 '00'00'00'00 '00'00'00'00
@_F_main 
  imw r0 ^f 
  add r0 rpp r0 
  add r0 r0 8 
  push r0 
  imw r0 1 
  ldw r1 rsp 0 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  imw r0 ^f 
  add r0 rpp r0 
  add r0 r0 8 
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
:_Lx0 
  add r0 rfp -4 
  push r0 
  imw r0 ^f 
  add r0 rpp r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -4 
  ldw r0 0 r0 
  add r0 r0 8 
  push r0 
  imw r0 1 
  ldw r1 rsp 0 
  ldw r1 0 r1 
  add r0 r1 r0 
  pop r1 
  stw r0 0 r1 
  add r0 rfp -4 
  ldw r0 0 r0 
  add r0 r0 8 
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
:_Lx1 
  zero r0 
  leave 
  ret 
=main 
  enter 
  sub rsp rsp 4 
  jmp ^_F_main 
