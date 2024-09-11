=main
  enter
  sub rsp rsp 16
  mov r0 rsp
  imw r1 1
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  ltu r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  ltu r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx0
  jmp &_Lx1
:_Lx0
  imw r0 1
  leave
  ret
  jmp &_Lx1
:_Lx1
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  ltu r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  ltu r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx2
  jmp &_Lx3
:_Lx2
  imw r0 2
  leave
  ret
  jmp &_Lx3
:_Lx3
  sub rsp rsp 16
  mov r0 rsp
  imw r1 1
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  ltu r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  ltu r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx4
  jmp &_Lx5
:_Lx4
  imw r0 3
  leave
  ret
  jmp &_Lx5
:_Lx5
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  ltu r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  ltu r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx6
  jmp &_Lx7
:_Lx6
  imw r0 4
  leave
  ret
  jmp &_Lx7
:_Lx7
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  ltu r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  ltu r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx8
  jmp &_Lx9
:_Lx8
  imw r0 5
  leave
  ret
  jmp &_Lx9
:_Lx9
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  ltu r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  ltu r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_LxA
  jmp &_LxB
:_LxA
  imw r0 6
  leave
  ret
  jmp &_LxB
:_LxB
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  ltu r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  ltu r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_LxC
  jmp &_LxD
:_LxC
  imw r0 7
  leave
  ret
  jmp &_LxD
:_LxD
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  ltu r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  ltu r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_LxE
  jmp &_LxF
:_LxE
  imw r0 8
  leave
  ret
  jmp &_LxF
:_LxF
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  ltu r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  ltu r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx10
  jmp &_Lx11
:_Lx10
  imw r0 9
  leave
  ret
  jmp &_Lx11
:_Lx11
  sub rsp rsp 16
  mov r0 rsp
  imw r1 1
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  ltu r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  ltu r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx12
  jmp &_Lx13
:_Lx12
  imw r0 10
  leave
  ret
  jmp &_Lx13
:_Lx13
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  ltu r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  ltu r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx14
  jmp &_Lx15
:_Lx14
  imw r0 11
  leave
  ret
  jmp &_Lx15
:_Lx15
  sub rsp rsp 16
  mov r0 rsp
  imw r1 1
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  ltu r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  ltu r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx16
  jmp &_Lx17
:_Lx16
  imw r0 12
  leave
  ret
  jmp &_Lx17
:_Lx17
  sub rsp rsp 16
  mov r0 rsp
  imw r1 1
  stw r1 r0 0
  imw r1 1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  ltu r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  ltu r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx18
  jmp &_Lx19
:_Lx18
  imw r0 13
  leave
  ret
  jmp &_Lx19
:_Lx19
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  ltu r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  ltu r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx1A
  jmp &_Lx1B
:_Lx1A
  imw r0 14
  leave
  ret
  jmp &_Lx1B
:_Lx1B
  sub rsp rsp 16
  mov r0 rsp
  imw r1 1
  stw r1 r0 0
  imw r1 1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  ltu r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  ltu r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx1C
  jmp &_Lx1D
:_Lx1C
  imw r0 15
  leave
  ret
  jmp &_Lx1D
:_Lx1D
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  ltu r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  ltu r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx1E
  jmp &_Lx1F
:_Lx1E
  imw r0 16
  leave
  ret
  jmp &_Lx1F
:_Lx1F
  sub rsp rsp 16
  mov r0 rsp
  imw r1 1
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx20
  jmp &_Lx21
:_Lx20
  imw r0 17
  leave
  ret
  jmp &_Lx21
:_Lx21
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx22
  jmp &_Lx23
:_Lx22
  imw r0 18
  leave
  ret
  jmp &_Lx23
:_Lx23
  sub rsp rsp 16
  mov r0 rsp
  imw r1 1
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx24
  jmp &_Lx25
:_Lx24
  imw r0 19
  leave
  ret
  jmp &_Lx25
:_Lx25
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx26
  jmp &_Lx27
:_Lx26
  imw r0 20
  leave
  ret
  jmp &_Lx27
:_Lx27
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx28
  jmp &_Lx29
:_Lx28
  imw r0 21
  leave
  ret
  jmp &_Lx29
:_Lx29
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx2A
  jmp &_Lx2B
:_Lx2A
  imw r0 22
  leave
  ret
  jmp &_Lx2B
:_Lx2B
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx2C
  jmp &_Lx2D
:_Lx2C
  imw r0 23
  leave
  ret
  jmp &_Lx2D
:_Lx2D
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx2E
  jmp &_Lx2F
:_Lx2E
  imw r0 24
  leave
  ret
  jmp &_Lx2F
:_Lx2F
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx30
  jmp &_Lx31
:_Lx30
  imw r0 25
  leave
  ret
  jmp &_Lx31
:_Lx31
  sub rsp rsp 16
  mov r0 rsp
  imw r1 1
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx32
  jmp &_Lx33
:_Lx32
  imw r0 26
  leave
  ret
  jmp &_Lx33
:_Lx33
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx34
  jmp &_Lx35
:_Lx34
  imw r0 27
  leave
  ret
  jmp &_Lx35
:_Lx35
  sub rsp rsp 16
  mov r0 rsp
  imw r1 1
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx36
  jmp &_Lx37
:_Lx36
  imw r0 28
  leave
  ret
  jmp &_Lx37
:_Lx37
  sub rsp rsp 16
  mov r0 rsp
  imw r1 1
  stw r1 r0 0
  imw r1 1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx38
  jmp &_Lx39
:_Lx38
  imw r0 29
  leave
  ret
  jmp &_Lx39
:_Lx39
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx3A
  jmp &_Lx3B
:_Lx3A
  imw r0 30
  leave
  ret
  jmp &_Lx3B
:_Lx3B
  sub rsp rsp 16
  mov r0 rsp
  imw r1 1
  stw r1 r0 0
  imw r1 1
  stw r1 r0 4
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx3C
  jmp &_Lx3D
:_Lx3C
  imw r0 31
  leave
  ret
  jmp &_Lx3D
:_Lx3D
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx3E
  jmp &_Lx3F
:_Lx3E
  imw r0 32
  leave
  ret
  jmp &_Lx3F
:_Lx3F
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  push r0
  mov r0 r1
  sub rsp rsp 8
  mov r1 rsp
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  call ^__llong_negate
  mov r1 r0
  add rsp rsp 8
  pop r0
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx40
  jmp &_Lx41
:_Lx40
  imw r0 33
  leave
  ret
  jmp &_Lx41
:_Lx41
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  call ^__llong_negate
  mov r0 r0
  add rsp rsp 8
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx42
  jmp &_Lx43
:_Lx42
  imw r0 34
  leave
  ret
  jmp &_Lx43
:_Lx43
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  push r0
  mov r0 r1
  sub rsp rsp 8
  mov r1 rsp
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  call ^__llong_negate
  mov r1 r0
  add rsp rsp 8
  pop r0
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx44
  jmp &_Lx45
:_Lx44
  imw r0 35
  leave
  ret
  jmp &_Lx45
:_Lx45
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  call ^__llong_negate
  mov r0 r0
  add rsp rsp 8
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx46
  jmp &_Lx47
:_Lx46
  imw r0 36
  leave
  ret
  jmp &_Lx47
:_Lx47
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  push r0
  mov r0 r1
  sub rsp rsp 8
  mov r1 rsp
  imw r2 0
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  call ^__llong_negate
  mov r1 r0
  add rsp rsp 8
  pop r0
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx48
  jmp &_Lx49
:_Lx48
  imw r0 37
  leave
  ret
  jmp &_Lx49
:_Lx49
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 0
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  call ^__llong_negate
  mov r0 r0
  add rsp rsp 8
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx4A
  jmp &_Lx4B
:_Lx4A
  imw r0 38
  leave
  ret
  jmp &_Lx4B
:_Lx4B
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  push r0
  mov r0 r1
  sub rsp rsp 8
  mov r1 rsp
  imw r2 0
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  call ^__llong_negate
  mov r1 r0
  add rsp rsp 8
  pop r0
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx4C
  jmp &_Lx4D
:_Lx4C
  imw r0 39
  leave
  ret
  jmp &_Lx4D
:_Lx4D
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 0
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  call ^__llong_negate
  mov r0 r0
  add rsp rsp 8
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx4E
  jmp &_Lx4F
:_Lx4E
  imw r0 40
  leave
  ret
  jmp &_Lx4F
:_Lx4F
  sub rsp rsp 16
  mov r0 rsp
  imw r1 1
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  push r0
  mov r0 r1
  sub rsp rsp 8
  mov r1 rsp
  imw r2 0
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  call ^__llong_negate
  mov r1 r0
  add rsp rsp 8
  pop r0
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx50
  jmp &_Lx51
:_Lx50
  imw r0 41
  leave
  ret
  jmp &_Lx51
:_Lx51
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 0
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  call ^__llong_negate
  mov r0 r0
  add rsp rsp 8
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx52
  jmp &_Lx53
:_Lx52
  imw r0 42
  leave
  ret
  jmp &_Lx53
:_Lx53
  sub rsp rsp 16
  mov r0 rsp
  imw r1 1
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  push r0
  mov r0 r1
  sub rsp rsp 8
  mov r1 rsp
  imw r2 0
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  call ^__llong_negate
  mov r1 r0
  add rsp rsp 8
  pop r0
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx54
  jmp &_Lx55
:_Lx54
  imw r0 43
  leave
  ret
  jmp &_Lx55
:_Lx55
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 0
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  call ^__llong_negate
  mov r0 r0
  add rsp rsp 8
  add r1 rsp 8
  imw r2 1
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx56
  jmp &_Lx57
:_Lx56
  imw r0 44
  leave
  ret
  jmp &_Lx57
:_Lx57
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  push r0
  mov r0 r1
  sub rsp rsp 8
  mov r1 rsp
  imw r2 1
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  call ^__llong_negate
  mov r1 r0
  add rsp rsp 8
  pop r0
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx58
  jmp &_Lx59
:_Lx58
  imw r0 45
  leave
  ret
  jmp &_Lx59
:_Lx59
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 1
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  call ^__llong_negate
  mov r0 r0
  add rsp rsp 8
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx5A
  jmp &_Lx5B
:_Lx5A
  imw r0 46
  leave
  ret
  jmp &_Lx5B
:_Lx5B
  sub rsp rsp 16
  mov r0 rsp
  imw r1 0
  stw r1 r0 0
  imw r1 0
  stw r1 r0 4
  add r1 rsp 8
  push r0
  mov r0 r1
  sub rsp rsp 8
  mov r1 rsp
  imw r2 1
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  call ^__llong_negate
  mov r1 r0
  add rsp rsp 8
  pop r0
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  jnz r0 &_Lx5C
  jmp &_Lx5D
:_Lx5C
  imw r0 47
  leave
  ret
  jmp &_Lx5D
:_Lx5D
  sub rsp rsp 16
  mov r0 rsp
  mov r0 r0
  sub rsp rsp 8
  mov r1 rsp
  imw r2 1
  stw r2 r1 0
  imw r2 1
  stw r2 r1 4
  call ^__llong_negate
  mov r0 r0
  add rsp rsp 8
  add r1 rsp 8
  imw r2 0
  stw r2 r1 0
  imw r2 0
  stw r2 r1 4
  ldw r0 rsp 0
  ldw r2 rsp 8
  lts r0 r0 r2
  ldw r1 rsp 4
  ldw r3 rsp 12
  sub r2 r1 r3
  isz r2 r2
  and r0 r0 r2
  lts r1 r1 r3
  or r0 r0 r1
  add rsp rsp 16
  sub r0 1 r0
  jnz r0 &_Lx5E
  jmp &_Lx5F
:_Lx5E
  imw r0 48
  leave
  ret
  jmp &_Lx5F
:_Lx5F
  zero r0
  leave
  ret
