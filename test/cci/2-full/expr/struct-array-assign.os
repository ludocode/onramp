=main
  enter
  sub rsp rsp 52
  imw r0 10
  imw r1 10
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
  add r0 rfp -50
  imw r1 1
  sts r1 r0 0
  imw r1 2
  sts r1 r0 2
  imw r1 3
  sts r1 r0 4
  imw r1 4
  sts r1 r0 6
  imw r1 5
  sts r1 r0 8
  sub rsp rsp 12
  mov r0 rsp
  add r1 rfp -50
  imw r3 10
  jmp &_Lx2
:_Lx2
  jz r3 &_Lx3
  sub r3 r3 2
  lds r2 r1 r3
  sts r2 r0 r3
  jmp &_Lx2
:_Lx3
  add r1 rfp -40
  imw r2 0
  mul r2 r2 10
  add r1 r1 r2
  imw r3 10
  jmp &_Lx4
:_Lx4
  jz r3 &_Lx5
  sub r3 r3 2
  lds r2 r0 r3
  sts r2 r1 r3
  jmp &_Lx4
:_Lx5
  add rsp rsp 12
  imw r0 0
  push r0
  add r0 rfp -40
  add r1 rfp -50
  imw r2 10
  call ^memcmp
  mov r1 r0
  pop r0
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
  sub rsp rsp 12
  mov r0 rsp
  add r1 rfp -50
  imw r3 10
  jmp &_Lx8
:_Lx8
  jz r3 &_Lx9
  sub r3 r3 2
  lds r2 r1 r3
  sts r2 r0 r3
  jmp &_Lx8
:_Lx9
  add r1 rfp -40
  imw r2 1
  mul r2 r2 10
  add r1 r1 r2
  imw r3 10
  jmp &_LxA
:_LxA
  jz r3 &_LxB
  sub r3 r3 2
  lds r2 r0 r3
  sts r2 r1 r3
  jmp &_LxA
:_LxB
  add rsp rsp 12
  imw r0 0
  push r0
  add r0 rfp -40
  imw r1 1
  mul r1 r1 10
  add r0 r0 r1
  add r1 rfp -50
  imw r2 10
  call ^memcmp
  mov r1 r0
  pop r0
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxC
  jmp &_LxD
:_LxC
  imw r0 3
  leave
  ret
  jmp &_LxD
:_LxD
  sub rsp rsp 12
  mov r0 rsp
  add r1 rfp -50
  imw r3 10
  jmp &_LxE
:_LxE
  jz r3 &_LxF
  sub r3 r3 2
  lds r2 r1 r3
  sts r2 r0 r3
  jmp &_LxE
:_LxF
  add r1 rfp -40
  imw r2 2
  mul r2 r2 10
  add r1 r1 r2
  imw r3 10
  jmp &_Lx10
:_Lx10
  jz r3 &_Lx11
  sub r3 r3 2
  lds r2 r0 r3
  sts r2 r1 r3
  jmp &_Lx10
:_Lx11
  add rsp rsp 12
  imw r0 0
  push r0
  add r0 rfp -40
  imw r1 2
  mul r1 r1 10
  add r0 r0 r1
  add r1 rfp -50
  imw r2 10
  call ^memcmp
  mov r1 r0
  pop r0
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx12
  jmp &_Lx13
:_Lx12
  imw r0 4
  leave
  ret
  jmp &_Lx13
:_Lx13
  sub rsp rsp 12
  mov r0 rsp
  add r1 rfp -50
  imw r3 10
  jmp &_Lx14
:_Lx14
  jz r3 &_Lx15
  sub r3 r3 2
  lds r2 r1 r3
  sts r2 r0 r3
  jmp &_Lx14
:_Lx15
  add r1 rfp -40
  imw r2 3
  mul r2 r2 10
  add r1 r1 r2
  imw r3 10
  jmp &_Lx16
:_Lx16
  jz r3 &_Lx17
  sub r3 r3 2
  lds r2 r0 r3
  sts r2 r1 r3
  jmp &_Lx16
:_Lx17
  add rsp rsp 12
  imw r0 0
  push r0
  add r0 rfp -40
  imw r1 40
  add r0 r0 r1
  imw r1 10
  sub r0 r0 r1
  add r1 rfp -50
  imw r2 10
  call ^memcmp
  mov r1 r0
  pop r0
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx18
  jmp &_Lx19
:_Lx18
  imw r0 5
  leave
  ret
  jmp &_Lx19
:_Lx19
  zero r0
  leave
  ret
