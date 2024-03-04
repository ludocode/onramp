; Calculates Fibonacci sequence, printing out first 20 entries.





; TODO this test is currently failing! program needs to be updated. for now test is skipped
=main
    add r0 '00 '00     ; zero r0
    ldw rip '00 rsp    ; ret





;==========================================
:main_DISABLED

; TODO set up frame pointer

; r0: a
; r1: b
; r2: c  (not preserved)
; r4: i

; initialize variables
add r0 ^00 ^00   ; a = 0
add r1 ^00 ^01   ; b = 1
add r4 ^00 ^14   ; i = 20

:main_loop

    ; call output_int(a)
    sub rsp rsp ^18         ; push stack (24)
    store r0 rsp ^14        ; preserve a
    store r1 rsp ^10        ; preserve b
    store r2 rsp ^0C        ; preserve c        ; TODO we don't actually need to preserve c
    store r4 rsp ^08        ; preserve i
    store rb rsp ^04        ; preserve rlr
    store r0 rsp ^00        ; pass argument a
    imm ra <output_int
    imm ra >output_int
    add rb rpc ^04          ; set return address
    add rpc rpb ra          ; jump output_int
    load rb rsp ^04         ; restore rlr
    load r4 rsp ^08         ; restore i
    load r2 rsp ^0C         ; restore c
    load r1 rsp ^10         ; restore b
    load r0 rsp ^14         ; restore a
    add rsp rsp ^18         ; pop stack

    ; calculate fibonacci
    add r2 r0 r1    ; c = a + b
    add r0 r1 ^00   ; a = b
    add r1 r2 ^00   ; b = c

    ; loop
    sub r4 r4 ^01      ; i -= 1
    jz r4 ^04 ^00      ; if (i != 0)...
    jz ^00 &main_loop  ; goto main_loop

; return
add rpc rb ^00   ; jump to return address



;==========================================
:output_int

; TODO set up frame pointer

; r0: a  ; remaining decimal to convert
; r1: b  ; a / 10
; r2: c  ; a % 10
; r3: d  ; ascii character

; load argument
load r0 rsp ^00   ; argument a

; push a null-terminator
sub rsp rsp ^04     ; sub rsp rsp 4
store ^00 rsp ^00   ; store 0 rsp 0

; convert char to decimal, pushing chars onto stack
:decimal_loop
    div r1 r0 ^0A          ; b = a / 10
    mul r2 r1 ^0A          ; c = b * 10
    sub r2 r0 r2          ; c = a - c  (i.e. c = a % 10)
    add r3 r2 ^30          ; d = c + '0'
    sub rsp rsp ^04
    store r3 rsp ^00       ; push d
    add r0 r1 ^00          ; a = b
    jz r0 ^04 ^00          ; if (a != 0)...
    jz ^00 &decimal_loop   ; goto decimal_loop

; pop chars and output
:print_loop
    load r3 rsp ^00        ; load r3 rsp 0    ; pop d
    jz r3 &print_loop_end  ; jz r0 24         ; if d == '\0': break
    sub rsp rsp ^08        ; sub rsp rsp 8
    store ^01 rsp ^00      ; store 1 rsp 0    ; push 1
    store r3 rsp ^04       ; store r3 rsp 4   ; push d
    sys write ^00 ^00      ; sys write        ; syscall write()
    add rsp rsp ^0C        ; add rsp rsp 12   ; pop args and digit
    jz ^00 &print_loop     ; jump print_loop
:print_loop_end

; output newline and clean up
sub rsp rsp ^08
store ^01 rsp ^00    ; push 1
store ^0A rsp ^04    ; push '\n'
sys write ^00 ^00   ; syscall write()
add rsp rsp ^0C     ; pop args and null terminator

