; The MIT License (MIT)
; Copyright (c) 2025 Fraser Heavy Software
; This test case is part of the Onramp compiler project.

; returns x * 3 + 9 / 4
=foo
  enter            ; create stack frame
  imw r9 ^_F_foo   ; get stack frame size of foo
  ldw r9 rpp r9    ; ^^^
  sub rsp rsp r9   ; reserve space for x
  stw r0 rfp -4    ; save x
  ldw r0 rfp -4    ; load x
  push r0
  imw r0 3         ; 3
  pop r1
  mul r0 r1 r0     ; *
  push r0
  imw r0 9         ; 9
  push r0
  imw r0 4         ; 4
  pop r1
  divu r0 r1 r0     ; /
  pop r1
  add r0 r1 r0     ; +
  leave
  ret

@_F_foo
  0

=main
  mov r0 3
  call ^foo
  sub r0 r0 11
  jnz r0 &fail

  mov r0 12
  call ^foo
  sub r0 r0 38
  jnz r0 &fail

  mov r0 0xffffffff
  call ^foo
  sub r0 r0 0xffffffff
  jnz r0 &fail

  zero r0
  ret

:fail
  mov r0 1
  ret
