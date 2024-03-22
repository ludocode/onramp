@alice 
0 
@bob 
1 
@carol 
2 
@dave 
-10 
@eve 
-2 
@fred 
-1 
@gary 
0 
@helen 
1 
@ida 
0x7FFFFFFF 
@_F_main 
  imw r0 ^alice 
  add r0 rpp r0 
  push r0 
  imw r0 0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx0 
  imw r0 1 
  leave 
  ret 
:_Lx0 
  imw r0 ^bob 
  add r0 rpp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx1 
  imw r0 1 
  leave 
  ret 
:_Lx1 
  imw r0 ^carol 
  add r0 rpp r0 
  push r0 
  imw r0 2 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx2 
  imw r0 1 
  leave 
  ret 
:_Lx2 
  imw r0 ^dave 
  add r0 rpp r0 
  push r0 
  imw r0 10 
  sub r0 0 r0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx3 
  imw r0 1 
  leave 
  ret 
:_Lx3 
  imw r0 ^eve 
  add r0 rpp r0 
  push r0 
  imw r0 2 
  sub r0 0 r0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx4 
  imw r0 1 
  leave 
  ret 
:_Lx4 
  imw r0 ^fred 
  add r0 rpp r0 
  push r0 
  imw r0 1 
  sub r0 0 r0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx5 
  imw r0 1 
  leave 
  ret 
:_Lx5 
  imw r0 ^gary 
  add r0 rpp r0 
  push r0 
  imw r0 0 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx6 
  imw r0 1 
  leave 
  ret 
:_Lx6 
  imw r0 ^helen 
  add r0 rpp r0 
  push r0 
  imw r0 1 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx7 
  imw r0 1 
  leave 
  ret 
:_Lx7 
  imw r0 ^ida 
  add r0 rpp r0 
  push r0 
  imw r0 0x7FFFFFFF 
  pop r1 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx8 
  imw r0 1 
  leave 
  ret 
:_Lx8 
  imw r0 ^gary 
  add r0 rpp r0 
  push r0 
  imw r0 ^alice 
  add r0 rpp r0 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_Lx9 
  imw r0 1 
  leave 
  ret 
:_Lx9 
  imw r0 ^helen 
  add r0 rpp r0 
  push r0 
  imw r0 ^bob 
  add r0 rpp r0 
  pop r1 
  ldw r0 0 r0 
  ldw r1 0 r1 
  cmpu r0 r1 r0 
  and r0 r0 1 
  jz r0 &_LxA 
  imw r0 1 
  leave 
  ret 
:_LxA 
  imw r0 0 
  leave 
  ret 
  zero r0 
  leave 
  ret 
=main 
  enter 
  jmp ^_F_main 
