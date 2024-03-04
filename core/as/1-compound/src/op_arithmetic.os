; The MIT License (MIT)
;
; Copyright (c) 2023-2024 Fraser Heavy Software
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.



; This is the implementation of the compound assembler.
;
; This file contains the implementations of the arithmetic opcodes.



; ==========================================================
; void opcode_add(void)
; ==========================================================
; Implements the add opcode.
; ==========================================================

=opcode_add
    add r0 '00 '70   ; add
    ims ra <opcode_standard
    ims ra >opcode_standard
    add rip rpp ra



; ==========================================================
; void opcode_sub(void)
; ==========================================================
; Implements the sub opcode.
; ==========================================================

=opcode_sub
    add r0 '00 '71   ; sub
    ims ra <opcode_standard
    ims ra >opcode_standard
    add rip rpp ra



; ==========================================================
; void opcode_mul(void)
; ==========================================================
; Implements the mul opcode.
; ==========================================================

=opcode_mul
    add r0 '00 '72   ; mul
    ims ra <opcode_standard
    ims ra >opcode_standard
    add rip rpp ra



; ==========================================================
; void opcode_divu(void)
; ==========================================================
; Implements the divu opcode.
; ==========================================================

=opcode_divu
    add r0 '00 '73   ; div
    ims ra <opcode_standard
    ims ra >opcode_standard
    add rip rpp ra



; ==========================================================
; void opcode_divs(void)
; ==========================================================
; Implements the divs opcode.
; ==========================================================

=opcode_divs

    ; push space to parse arguments
    sub rsp rsp '04
    add r0 rsp '00

    ; call parse_register_mix_mix()
    ims ra <parse_register_mix_mix
    ims ra >parse_register_mix_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the template
    ims ra <opcode_divs_template
    ims ra >opcode_divs_template
    add r0 rpp ra
    add r1 '00 '5C

    ; fill it in
    ldb r2 rsp '00    ; load dest
    stb r2 r0 '3D
    stb r2 r0 '59
    stb r2 r0 '5B
    ldb r2 rsp '01    ; load src1
    stb r2 r0 '06
    stb r2 r0 '17
    stb r2 r0 '1E
    ldb r2 rsp '02    ; load src2
    stb r2 r0 '22
    stb r2 r0 '33
    stb r2 r0 '3A

    ; pop the arguments
    add rsp rsp '04   ; popd

    ; output it, tail-call emit_bytes_as_hex()
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_divs_template

    ; make stack space
    sub rsp rsp '08

    ; collect sign of src1 in ra, store it on the stack
    ror ra '82 '1F
    and ra ra '01
    stw ra rsp '00

    ; place absolute value of src1 in ra
    jz ra '02 '00
    sub ra '00 '82
    jz '00 '01 '00
    add ra '82 '00

    ; collect sign of src2 in rb, store it on the stack
    ror rb '83 '1F
    and rb rb '01
    stw rb rsp '04

    ; place absolute value of src2 in rb
    jz rb '02 '00
    sub rb '00 '83
    jz '00 '01 '00
    add rb '83 '00

    ; do the unsigned division
    ; (we can write to dest now since we're done reading srcs)
    divu '81 ra rb

    ; pop and xor signs
    ldw ra rsp '00
    ldw rb rsp '04
    add rsp rsp '08
    add ra ra rb
    and ra ra '01

    ; flip sign of dest if exactly one of src1 and src2 was negative
    jz ra '01 '00
    sub '81 '00 '81



; ==========================================================
; void opcode_modu(void)
; ==========================================================
; Implements the modu opcode.
; ==========================================================

=opcode_modu
    'FF ;TODO



; ==========================================================
; void opcode_mods(void)
; ==========================================================
; Implements the mods opcode.
; ==========================================================

=opcode_mods
    'FF ;TODO



; ==========================================================
; void opcode_sxb(void)
; ==========================================================
; Implements the sxb opcode.
; ==========================================================

=opcode_sxb
    'FF ;TODO



; ==========================================================
; void opcode_sxs(void)
; ==========================================================
; Implements the sxs opcode.
; ==========================================================

=opcode_sxs
    'FF ;TODO



; ==========================================================
; void opcode_zero(void)
; ==========================================================
; Implements the zero opcode.
; ==========================================================

=opcode_zero

    ; destination is a register. call parse_register
    ims ra <parse_register
    ims ra >parse_register
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put dest into the template
    ims ra <opcode_zero_template
    ims ra >opcode_zero_template
    add r2 rpp ra
    stb r0 r2 '01

    ; output, tail-call emit_bytes_as_hex()
    add r0 r2 '00
    add r1 '00 '04
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_zero_template
    add r0 '00 '00   ; add <dest> 0 0



; ==========================================================
; void opcode_inc(void)
; ==========================================================
; Implements the inc opcode.
; ==========================================================

=opcode_inc

    ; destination is a register. call parse_register
    ims ra <parse_register
    ims ra >parse_register
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put dest into the template
    ims ra <opcode_inc_template
    ims ra >opcode_inc_template
    add r2 rpp ra
    stb r0 r2 '01
    stb r0 r2 '02

    ; output, tail-call emit_bytes_as_hex()
    add r0 r2 '00
    add r1 '00 '04
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_inc_template
    add r0 r0 '01   ; add <dest> <dest> 1



; ==========================================================
; void opcode_dec(void)
; ==========================================================
; Implements the dec opcode.
; ==========================================================

=opcode_dec

    ; destination is a register. call parse_register
    ims ra <parse_register
    ims ra >parse_register
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put dest into the template
    ims ra <opcode_dec_template
    ims ra >opcode_dec_template
    add r2 rpp ra
    stb r0 r2 '01
    stb r0 r2 '02

    ; output, tail-call emit_bytes_as_hex()
    add r0 r2 '00
    add r1 '00 '04
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_dec_template
    sub r0 r0 '01   ; sub <dest> <dest> 1
