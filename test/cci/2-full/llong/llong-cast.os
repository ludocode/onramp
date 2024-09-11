=main
  enter
  sub rsp rsp 100
  add r0 rfp -4
  sub rsp rsp 8
  mov r1 rsp
  imw r2 -1412567278
  stw r2 r1 0
  imw r2 305419896
  stw r2 r1 4
  ldw r1 r1 0
  add rsp rsp 8
  stw r1 r0 0
  ldw r0 rfp -4
  imw r1 -1412567278
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  imw r0 1
  leave
  ret
  jmp &_Lx1
:_Lx1
  add r0 rfp -8
  sub rsp rsp 8
  mov r1 rsp
  imw r2 16
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r1 r1 0
  add rsp rsp 8
  stw r1 r0 0
  ldw r0 rfp -8
  imw r1 16
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  imw r0 2
  leave
  ret
  jmp &_Lx3
:_Lx3
  add r0 rfp -12
  sub rsp rsp 8
  mov r1 rsp
  imw r2 -1412567278
  stw r2 r1 0
  imw r2 305419896
  stw r2 r1 4
  ldw r1 r1 0
  add rsp rsp 8
  stw r1 r0 0
  ldw r0 rfp -12
  imw r1 -1412567278
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx4
  jmp &_Lx5
:_Lx4
  imw r0 3
  leave
  ret
  jmp &_Lx5
:_Lx5
  add r0 rfp -16
  sub rsp rsp 8
  mov r1 rsp
  imw r2 16
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r1 r1 0
  add rsp rsp 8
  stw r1 r0 0
  ldw r0 rfp -16
  imw r1 16
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx6
  jmp &_Lx7
:_Lx6
  imw r0 4
  leave
  ret
  jmp &_Lx7
:_Lx7
  add r0 rfp -20
  sub rsp rsp 8
  mov r1 rsp
  imw r2 -1412567278
  stw r2 r1 0
  imw r2 305419896
  stw r2 r1 4
  ldw r1 r1 0
  add rsp rsp 8
  stw r1 r0 0
  ldw r0 rfp -20
  imw r1 -1412567278
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx8
  jmp &_Lx9
:_Lx8
  imw r0 5
  leave
  ret
  jmp &_Lx9
:_Lx9
  add r0 rfp -24
  sub rsp rsp 8
  mov r1 rsp
  imw r2 -5
  stw r2 r1 0
  imw r2 305419896
  stw r2 r1 4
  ldw r1 r1 0
  add rsp rsp 8
  stw r1 r0 0
  ldw r0 rfp -24
  imw r1 5
  sub r1 0 r1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxA
  jmp &_LxB
:_LxA
  imw r0 6
  leave
  ret
  jmp &_LxB
:_LxB
  add r0 rfp -28
  sub rsp rsp 8
  mov r1 rsp
  imw r2 -1412567278
  stw r2 r1 0
  imw r2 305419896
  stw r2 r1 4
  ldw r1 r1 0
  add rsp rsp 8
  stw r1 r0 0
  ldw r0 rfp -28
  imw r1 -1412567278
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxC
  jmp &_LxD
:_LxC
  imw r0 7
  leave
  ret
  jmp &_LxD
:_LxD
  add r0 rfp -32
  sub rsp rsp 8
  mov r1 rsp
  imw r2 -5
  stw r2 r1 0
  imw r2 305419896
  stw r2 r1 4
  ldw r1 r1 0
  add rsp rsp 8
  stw r1 r0 0
  ldw r0 rfp -32
  imw r1 5
  sub r1 0 r1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxE
  jmp &_LxF
:_LxE
  imw r0 8
  leave
  ret
  jmp &_LxF
:_LxF
  add r0 rfp -33
  sub rsp rsp 8
  mov r1 rsp
  imw r2 2
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r2 r1 4
  ldw r1 r1 0
  or r1 r1 r2
  bool r1 r1
  add rsp rsp 8
  stb r1 r0 0
  ldb r0 rfp -33
  trb r0 r0
  imw r1 1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx10
  jmp &_Lx11
:_Lx10
  imw r0 9
  leave
  ret
  jmp &_Lx11
:_Lx11
  add r0 rfp -34
  sub rsp rsp 8
  mov r1 rsp
  imw r2 0
  stw r2 r1 0
  imw r2 -2147483648
  stw r2 r1 4
  ldw r2 r1 4
  ldw r1 r1 0
  or r1 r1 r2
  bool r1 r1
  add rsp rsp 8
  stb r1 r0 0
  ldb r0 rfp -34
  trb r0 r0
  imw r1 1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx12
  jmp &_Lx13
:_Lx12
  imw r0 10
  leave
  ret
  jmp &_Lx13
:_Lx13
  add r0 rfp -35
  sub rsp rsp 8
  mov r1 rsp
  imw r2 4
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r2 r1 4
  ldw r1 r1 0
  or r1 r1 r2
  bool r1 r1
  add rsp rsp 8
  stb r1 r0 0
  ldb r0 rfp -35
  trb r0 r0
  imw r1 1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx14
  jmp &_Lx15
:_Lx14
  imw r0 11
  leave
  ret
  jmp &_Lx15
:_Lx15
  add r0 rfp -36
  sub rsp rsp 8
  mov r1 rsp
  imw r2 0
  stw r2 r1 0
  imw r2 -1
  stw r2 r1 4
  ldw r2 r1 4
  ldw r1 r1 0
  or r1 r1 r2
  bool r1 r1
  add rsp rsp 8
  stb r1 r0 0
  ldb r0 rfp -36
  trb r0 r0
  imw r1 1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx16
  jmp &_Lx17
:_Lx16
  imw r0 12
  leave
  ret
  jmp &_Lx17
:_Lx17
  add r0 rfp -44
  imw r1 0
  bool r1 r1
  trb r1 r1
  stw r1 r0 0
  stw 0 r0 4
  sub rsp rsp 16
  mov r0 rsp
  add r1 rfp -44
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  sub r0 r0 r2
  ldw r1 rsp 4
  ldw r2 rsp 12
  sub r1 r1 r2
  or r0 r0 r1
  add rsp rsp 16
  bool r0 r0
  jnz r0 &_Lx18
  jmp &_Lx19
:_Lx18
  imw r0 13
  leave
  ret
  jmp &_Lx19
:_Lx19
  add r0 rfp -52
  imw r1 2
  bool r1 r1
  trb r1 r1
  stw r1 r0 0
  stw 0 r0 4
  sub rsp rsp 16
  mov r0 rsp
  add r1 rfp -52
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  sub r0 r0 r2
  ldw r1 rsp 4
  ldw r2 rsp 12
  sub r1 r1 r2
  or r0 r0 r1
  add rsp rsp 16
  bool r0 r0
  jnz r0 &_Lx1A
  jmp &_Lx1B
:_Lx1A
  imw r0 14
  leave
  ret
  jmp &_Lx1B
:_Lx1B
  add r0 rfp -60
  imw r1 0
  bool r1 r1
  trb r1 r1
  stw r1 r0 0
  stw 0 r0 4
  sub rsp rsp 16
  mov r0 rsp
  add r1 rfp -60
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  sub r0 r0 r2
  ldw r1 rsp 4
  ldw r2 rsp 12
  sub r1 r1 r2
  or r0 r0 r1
  add rsp rsp 16
  bool r0 r0
  jnz r0 &_Lx1C
  jmp &_Lx1D
:_Lx1C
  imw r0 15
  leave
  ret
  jmp &_Lx1D
:_Lx1D
  add r0 rfp -68
  imw r1 2
  bool r1 r1
  trb r1 r1
  stw r1 r0 0
  stw 0 r0 4
  sub rsp rsp 16
  mov r0 rsp
  add r1 rfp -68
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  sub r0 r0 r2
  ldw r1 rsp 4
  ldw r2 rsp 12
  sub r1 r1 r2
  or r0 r0 r1
  add rsp rsp 16
  bool r0 r0
  jnz r0 &_Lx1E
  jmp &_Lx1F
:_Lx1E
  imw r0 16
  leave
  ret
  jmp &_Lx1F
:_Lx1F
  add r0 rfp -76
  imw r1 -231451016
  stw r1 r0 0
  stw 0 r0 4
  sub rsp rsp 16
  mov r0 rsp
  add r1 rfp -76
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  add r1 rsp 8
  imw r2 -231451016
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  sub r0 r0 r2
  ldw r1 rsp 4
  ldw r2 rsp 12
  sub r1 r1 r2
  or r0 r0 r1
  add rsp rsp 16
  bool r0 r0
  jnz r0 &_Lx20
  jmp &_Lx21
:_Lx20
  imw r0 17
  leave
  ret
  jmp &_Lx21
:_Lx21
  add r0 rfp -84
  imw r1 -231451016
  stw r1 r0 0
  shru r1 r1 31
  sub r1 0 r1
  stw r1 r0 4
  sub rsp rsp 16
  mov r0 rsp
  add r1 rfp -84
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  add r1 rsp 8
  imw r2 -231451016
  stw r2 r1 0
  imw r2 -1
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  sub r0 r0 r2
  ldw r1 rsp 4
  ldw r2 rsp 12
  sub r1 r1 r2
  or r0 r0 r1
  add rsp rsp 16
  bool r0 r0
  jnz r0 &_Lx22
  jmp &_Lx23
:_Lx22
  imw r0 18
  leave
  ret
  jmp &_Lx23
:_Lx23
  add r0 rfp -92
  imw r1 -231451016
  stw r1 r0 0
  stw 0 r0 4
  sub rsp rsp 16
  mov r0 rsp
  add r1 rfp -92
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  add r1 rsp 8
  imw r2 -231451016
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  sub r0 r0 r2
  ldw r1 rsp 4
  ldw r2 rsp 12
  sub r1 r1 r2
  or r0 r0 r1
  add rsp rsp 16
  bool r0 r0
  jnz r0 &_Lx24
  jmp &_Lx25
:_Lx24
  imw r0 19
  leave
  ret
  jmp &_Lx25
:_Lx25
  add r0 rfp -100
  imw r1 -231451016
  stw r1 r0 0
  shru r1 r1 31
  sub r1 0 r1
  stw r1 r0 4
  sub rsp rsp 16
  mov r0 rsp
  add r1 rfp -100
  ldw r2 r1 0
  stw r2 r0 0
  ldw r2 r1 4
  stw r2 r0 4
  add r1 rsp 8
  imw r2 -231451016
  stw r2 r1 0
  imw r2 -1
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  sub r0 r0 r2
  ldw r1 rsp 4
  ldw r2 rsp 12
  sub r1 r1 r2
  or r0 r0 r1
  add rsp rsp 16
  bool r0 r0
  jnz r0 &_Lx26
  jmp &_Lx27
:_Lx26
  imw r0 20
  leave
  ret
  jmp &_Lx27
:_Lx27
  zero r0
  leave
  ret
