=main
  enter
  sub rsp rsp 4
  imw r0 60
  imw r1 15
  imw r2 4
  mul r1 r1 r2
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  imw r0 16
  leave
  ret
  jmp &_Lx1
:_Lx1
  imw r0 20
  imw r1 5
  imw r2 4
  mul r1 r1 r2
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  imw r0 17
  leave
  ret
  jmp &_Lx3
:_Lx3
  imw r0 1
  imw r2 ^x
  add r2 rpp r2
  imw r3 0
  mul r3 r3 20
  add r2 r2 r3
  mov r1 r2
  imw r2 0
  shl r2 r2 2
  add r1 r1 r2
  stw r0 r1 0
  imw r0 2
  imw r2 ^x
  add r2 rpp r2
  imw r3 0
  mul r3 r3 20
  add r2 r2 r3
  mov r1 r2
  imw r2 1
  shl r2 r2 2
  add r1 r1 r2
  stw r0 r1 0
  imw r0 3
  imw r2 ^x
  add r2 rpp r2
  imw r3 0
  mul r3 r3 20
  add r2 r2 r3
  mov r1 r2
  imw r2 2
  shl r2 r2 2
  add r1 r1 r2
  stw r0 r1 0
  imw r0 4
  imw r2 ^x
  add r2 rpp r2
  imw r3 0
  mul r3 r3 20
  add r2 r2 r3
  mov r1 r2
  imw r2 3
  shl r2 r2 2
  add r1 r1 r2
  stw r0 r1 0
  imw r0 5
  imw r2 ^x
  add r2 rpp r2
  imw r3 0
  mul r3 r3 20
  add r2 r2 r3
  mov r1 r2
  imw r2 4
  shl r2 r2 2
  add r1 r1 r2
  stw r0 r1 0
  imw r0 6
  imw r2 ^x
  add r2 rpp r2
  imw r3 1
  mul r3 r3 20
  add r2 r2 r3
  mov r1 r2
  imw r2 0
  shl r2 r2 2
  add r1 r1 r2
  stw r0 r1 0
  imw r0 7
  imw r2 ^x
  add r2 rpp r2
  imw r3 1
  mul r3 r3 20
  add r2 r2 r3
  mov r1 r2
  imw r2 1
  shl r2 r2 2
  add r1 r1 r2
  stw r0 r1 0
  imw r0 8
  imw r2 ^x
  add r2 rpp r2
  imw r3 1
  mul r3 r3 20
  add r2 r2 r3
  mov r1 r2
  imw r2 2
  shl r2 r2 2
  add r1 r1 r2
  stw r0 r1 0
  imw r0 9
  imw r2 ^x
  add r2 rpp r2
  imw r3 1
  mul r3 r3 20
  add r2 r2 r3
  mov r1 r2
  imw r2 3
  shl r2 r2 2
  add r1 r1 r2
  stw r0 r1 0
  imw r0 10
  imw r2 ^x
  add r2 rpp r2
  imw r3 1
  mul r3 r3 20
  add r2 r2 r3
  mov r1 r2
  imw r2 4
  shl r2 r2 2
  add r1 r1 r2
  stw r0 r1 0
  imw r0 11
  imw r2 ^x
  add r2 rpp r2
  imw r3 2
  mul r3 r3 20
  add r2 r2 r3
  mov r1 r2
  imw r2 0
  shl r2 r2 2
  add r1 r1 r2
  stw r0 r1 0
  imw r0 12
  imw r2 ^x
  add r2 rpp r2
  imw r3 2
  mul r3 r3 20
  add r2 r2 r3
  mov r1 r2
  imw r2 1
  shl r2 r2 2
  add r1 r1 r2
  stw r0 r1 0
  imw r0 13
  imw r2 ^x
  add r2 rpp r2
  imw r3 2
  mul r3 r3 20
  add r2 r2 r3
  mov r1 r2
  imw r2 2
  shl r2 r2 2
  add r1 r1 r2
  stw r0 r1 0
  imw r0 14
  imw r2 ^x
  add r2 rpp r2
  imw r3 2
  mul r3 r3 20
  add r2 r2 r3
  mov r1 r2
  imw r2 3
  shl r2 r2 2
  add r1 r1 r2
  stw r0 r1 0
  imw r0 15
  imw r2 ^x
  add r2 rpp r2
  imw r3 2
  mul r3 r3 20
  add r2 r2 r3
  mov r1 r2
  imw r2 4
  shl r2 r2 2
  add r1 r1 r2
  stw r0 r1 0
  add r0 rfp -4
  imw r1 ^x
  add r1 rpp r1
  stw r1 r0 0
  ldw r1 rfp -4
  imw r2 0
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx4
  jmp &_Lx5
:_Lx4
  imw r0 1
  leave
  ret
  jmp &_Lx5
:_Lx5
  ldw r1 rfp -4
  imw r2 1
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 2
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx6
  jmp &_Lx7
:_Lx6
  imw r0 2
  leave
  ret
  jmp &_Lx7
:_Lx7
  ldw r1 rfp -4
  imw r2 2
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 3
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx8
  jmp &_Lx9
:_Lx8
  imw r0 3
  leave
  ret
  jmp &_Lx9
:_Lx9
  ldw r1 rfp -4
  imw r2 3
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 4
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxA
  jmp &_LxB
:_LxA
  imw r0 4
  leave
  ret
  jmp &_LxB
:_LxB
  ldw r1 rfp -4
  imw r2 4
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 5
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxC
  jmp &_LxD
:_LxC
  imw r0 5
  leave
  ret
  jmp &_LxD
:_LxD
  ldw r1 rfp -4
  imw r2 5
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 6
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxE
  jmp &_LxF
:_LxE
  imw r0 6
  leave
  ret
  jmp &_LxF
:_LxF
  ldw r1 rfp -4
  imw r2 6
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 7
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx10
  jmp &_Lx11
:_Lx10
  imw r0 7
  leave
  ret
  jmp &_Lx11
:_Lx11
  ldw r1 rfp -4
  imw r2 7
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 8
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx12
  jmp &_Lx13
:_Lx12
  imw r0 8
  leave
  ret
  jmp &_Lx13
:_Lx13
  ldw r1 rfp -4
  imw r2 8
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 9
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx14
  jmp &_Lx15
:_Lx14
  imw r0 9
  leave
  ret
  jmp &_Lx15
:_Lx15
  ldw r1 rfp -4
  imw r2 9
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 10
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx16
  jmp &_Lx17
:_Lx16
  imw r0 10
  leave
  ret
  jmp &_Lx17
:_Lx17
  ldw r1 rfp -4
  imw r2 10
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 11
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx18
  jmp &_Lx19
:_Lx18
  imw r0 11
  leave
  ret
  jmp &_Lx19
:_Lx19
  ldw r1 rfp -4
  imw r2 11
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 12
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx1A
  jmp &_Lx1B
:_Lx1A
  imw r0 12
  leave
  ret
  jmp &_Lx1B
:_Lx1B
  ldw r1 rfp -4
  imw r2 12
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 13
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx1C
  jmp &_Lx1D
:_Lx1C
  imw r0 13
  leave
  ret
  jmp &_Lx1D
:_Lx1D
  ldw r1 rfp -4
  imw r2 13
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 14
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx1E
  jmp &_Lx1F
:_Lx1E
  imw r0 14
  leave
  ret
  jmp &_Lx1F
:_Lx1F
  ldw r1 rfp -4
  imw r2 14
  shl r2 r2 2
  add r1 r1 r2
  ldw r0 0 r1
  imw r1 15
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx20
  jmp &_Lx21
:_Lx20
  imw r0 15
  leave
  ret
  jmp &_Lx21
:_Lx21
  zero r0
  leave
  ret
=x 0 0 0 0 0 0 0 0 0 0 0 0 0 0
  0
