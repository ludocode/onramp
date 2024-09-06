=main
  enter
  jmp &_Lx0
:_Lx0
  jmp &_Lx1
  imw r0 1
  jmp &_Lx2
  imw r0 1
  leave
  ret
  jmp &_Lx2
:_Lx1
  zero r0
  leave
  ret
:_Lx2
  imw r0 1
  leave
  ret
  imw r0 0
  jz r0 &_Lx1
  jmp &_Lx0
