#line manual
#line 5 "./stmt-exprs/stmt-expr-while.c"
=main
  enter
  sub rsp rsp 8
#line 10
  mov r0 3
  mov r2 1
  add r3 rfp -8
  stw r2 0 r3
  jmp &_Lx0
:_Lx0
  ldw r1 rfp -8
  jz r1 &_Lx1
  mov r1 0
  add r2 rfp -8
  stw r1 0 r2
#line 12
  jmp &_Lx0
:_Lx1
#line 15
  mov r1 5
#line 10
  add r0 r0 r1
  add r1 rfp -4
  stw r0 0 r1
#line 18
  ldw r0 rfp -4
  mov r1 8
  sub r0 r0 r1
  cmpu r0 r0 0
  and r0 r0 1
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  mov r0 1
  leave
  ret
#line 18
  jmp &_Lx3
:_Lx3
#line 20
  mov r0 0
  leave
  ret
