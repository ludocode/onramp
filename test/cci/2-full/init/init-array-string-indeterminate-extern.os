@_Sx0
  "Alice"
  '00
=alice 0
  0
@{50_Ix0_alice
  enter
  imw r0 ^alice
  add r0 rpp r0
  imw r1 ^_Sx0
  add r1 rpp r1
  imw r2 0
  add r2 r2 r0
  imw r4 6
  jmp &_Lx1
:_Lx1
  jz r4 &_Lx2
  sub r4 r4 1
  ldb r3 r1 r4
  stb r3 r2 r4
  jmp &_Lx1
:_Lx2
  leave
  ret
@_Sx1
  "Bob"'00"!!!!!!"
  '00
=bob
  0
@{50_Ix3_bob
  enter
  imw r0 ^bob
  add r0 rpp r0
  imw r1 ^_Sx1
  add r1 rpp r1
  imw r2 0
  add r2 r2 r0
  ldb r3 r1 0
  stb r3 r2 0
  ldb r3 r1 1
  stb r3 r2 1
  ldb r3 r1 2
  stb r3 r2 2
  ldb r3 r1 3
  stb r3 r2 3
  leave
  ret
@_Sx2
  "Carl"
  '00
=carl 0 0
  0
@{50_Ix4_carl
  enter
  imw r0 ^carl
  add r0 rpp r0
  imw r1 ^_Sx2
  add r1 rpp r1
  imw r2 0
  add r2 r2 r0
  imw r4 5
  jmp &_Lx5
:_Lx5
  jz r4 &_Lx6
  sub r4 r4 1
  ldb r3 r1 r4
  stb r3 r2 r4
  jmp &_Lx5
:_Lx6
  add r2 r2 5
  stb 0 r2 0
  stb 0 r2 1
  stb 0 r2 2
  stb 0 r2 3
  leave
  ret
@_Sx3
  "Alice"
  '00
@_Sx4
  "Bob"
  '00
@_Sx5
  "Carl"
  '00
=main
  enter
  imw r0 6
  imw r1 6
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx7
  jmp &_Lx8
:_Lx7
  imw r0 1
  leave
  ret
  jmp &_Lx8
:_Lx8
  imw r0 4
  imw r1 4
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx9
  jmp &_LxA
:_Lx9
  imw r0 2
  leave
  ret
  jmp &_LxA
:_LxA
  imw r0 9
  imw r1 9
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxB
  jmp &_LxC
:_LxB
  imw r0 3
  leave
  ret
  jmp &_LxC
:_LxC
  imw r0 0
  push r0
  imw r0 ^alice
  add r0 rpp r0
  imw r1 ^_Sx3
  add r1 rpp r1
  call ^strcmp
  mov r1 r0
  pop r0
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxD
  jmp &_LxE
:_LxD
  imw r0 4
  leave
  ret
  jmp &_LxE
:_LxE
  imw r0 0
  push r0
  imw r0 ^bob
  add r0 rpp r0
  imw r1 ^_Sx4
  add r1 rpp r1
  call ^strcmp
  mov r1 r0
  pop r0
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_LxF
  jmp &_Lx10
:_LxF
  imw r0 5
  leave
  ret
  jmp &_Lx10
:_Lx10
  imw r0 0
  push r0
  imw r0 ^carl
  add r0 rpp r0
  imw r1 ^_Sx5
  add r1 rpp r1
  call ^strcmp
  mov r1 r0
  pop r0
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx11
  jmp &_Lx12
:_Lx11
  imw r0 6
  leave
  ret
  jmp &_Lx12
:_Lx12
  imw r0 0
  imw r2 ^carl
  add r2 rpp r2
  imw r3 6
  add r2 r2 r3
  ldb r1 0 r2
  sxb r1 r1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx13
  jmp &_Lx14
:_Lx13
  imw r0 7
  leave
  ret
  jmp &_Lx14
:_Lx14
  imw r0 0
  imw r2 ^carl
  add r2 rpp r2
  imw r3 7
  add r2 r2 r3
  ldb r1 0 r2
  sxb r1 r1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx15
  jmp &_Lx16
:_Lx15
  imw r0 8
  leave
  ret
  jmp &_Lx16
:_Lx16
  imw r0 0
  imw r2 ^carl
  add r2 rpp r2
  imw r3 8
  add r2 r2 r3
  ldb r1 0 r2
  sxb r1 r1
  sub r0 r0 r1
  bool r0 r0
  jnz r0 &_Lx17
  jmp &_Lx18
:_Lx17
  imw r0 9
  leave
  ret
  jmp &_Lx18
:_Lx18
  zero r0
  leave
  ret
