=main
  enter
  jmp &_Lx0
:_Lx1
  jmp &_Lx0
:_Lx0
  jmp &_Lx2
  jmp &_Lx3
:_Lx2
  zero r0
  leave
  ret
:_Lx4
  jmp &_Lx3
:_Lx3
  mov r0 1
  leave
  ret
  jmp &_Lx4
:_Lx5
  mov r0 1
  leave
  ret
  jmp &_Lx1
