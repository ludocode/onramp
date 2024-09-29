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
    add r1 '00 '54

    ; fill it in
    ldb r2 rsp '00    ; load dest
    stb r2 r0 '35
    stb r2 r0 '51
    stb r2 r0 '53
    ldb r2 rsp '01    ; load src1
    stb r2 r0 '06
    stb r2 r0 '13
    stb r2 r0 '1A
    ldb r2 rsp '02    ; load src2
    stb r2 r0 '1E
    stb r2 r0 '2B
    stb r2 r0 '32

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
    shru ra r1 '1F
    stw ra rsp '00

    ; place absolute value of src1 in ra
    jz ra '02 '00
    sub ra '00 r1
    jz '00 '01 '00
    add ra r1 '00

    ; collect sign of src2 in rb, store it on the stack
    shru rb r2 '1F
    stw rb rsp '04

    ; place absolute value of src2 in rb
    jz rb '02 '00
    sub rb '00 r2
    jz '00 '01 '00
    add rb r2 '00

    ; do the unsigned division
    ; (we can write to dest now since we're done reading srcs)
    divu r0 ra rb

    ; pop and xor signs
    ldw ra rsp '00
    ldw rb rsp '04
    add rsp rsp '08
    add ra ra rb
    and ra ra '01

    ; flip sign of dest if exactly one of src1 and src2 was negative
    jz ra '01 '00
    sub r0 '00 r0



; ==========================================================
; void opcode_modu(void)
; ==========================================================
; Implements the modu opcode.
; ==========================================================

=opcode_modu

    ; destination is a register. call parse_register
    ims ra <parse_register
    ims ra >parse_register
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put dest in the template
    ims ra <opcode_modu_template
    ims ra >opcode_modu_template
    add r2 rpp ra
    stb r0 r2 '09

    ; dividend is mix-type
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put dividend in the template
    ims ra <opcode_modu_template
    ims ra >opcode_modu_template
    add r2 rpp ra
    stb r0 r2 '02
    stb r0 r2 '0A

    ; divisor is mix-type
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put divisor in the template
    ims ra <opcode_modu_template
    ims ra >opcode_modu_template
    add r2 rpp ra
    stb r0 r2 '03
    stb r0 r2 '07

    ; output, tail-call emit_bytes_as_hex()
    add r0 r2 '00
    add r1 '00 '0C
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_modu_template
    divu ra r1 r2     ; divu ra <dividend> <divisor>
    mul rb ra r2      ; mul rb ra <divisor>
    sub r0 r1 rb      ; sub <dest> <dividend> rb



; ==========================================================
; void opcode_mods(void)
; ==========================================================
; Implements the mods opcode.
; ==========================================================

=opcode_mods

    ; TODO why isn't this using parse_register_mix_mix()?

    ; destination is a register. call parse_register
    ims ra <parse_register
    ims ra >parse_register
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put dest in the template
    ims ra <opcode_mods_template
    ims ra >opcode_mods_template
    add r2 rpp ra
    stb r0 r2 '31
    stb r0 r2 '36
    stb r0 r2 '39
    stb r0 r2 '49
    stb r0 r2 '4B

    ; dividend is mix-type
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put dividend in the template
    ims ra <opcode_mods_template
    ims ra >opcode_mods_template
    add r2 rpp ra
    stb r0 r2 '06
    stb r0 r2 '13
    stb r0 r2 '1A

    ; divisor is mix-type
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put divisor in the template
    ims ra <opcode_mods_template
    ims ra >opcode_mods_template
    add r2 rpp ra
    stb r0 r2 '1E
    stb r0 r2 '27
    stb r0 r2 '2E

    ; output, tail-call emit_bytes_as_hex()
    add r0 r2 '00
    add r1 '00 '4C
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_mods_template

    ; make stack space
    sub rsp rsp '04

    ; store sign of src1 on the stack
    shru ra r1 '1F
    stw ra rsp '00

    ; place absolute value of src1 in ra
    jz ra '02 '00
    sub ra '00 r1
    jz '00 '01 '00
    add ra r1 '00

    ; place absolute value of src2 in rb
    shru rb r2 '1F
    jz rb '02 '00
    sub rb '00 r2
    jz '00 '01 '00
    add rb r2 '00

    ; do the unsigned modulus
    ; (we can write to dest now since we're done reading srcs)
    divu r0 ra rb
    mul rb r0 rb
    sub r0 ra rb

    ; pop and flip sign of dest if src1 was negative
    ldw ra rsp '00
    add rsp rsp '04
    jz ra '01 '00
    sub r0 '00 r0



; ==========================================================
; void opcode_sxb(void)
; ==========================================================
; Implements the sxb opcode.
; ==========================================================

=opcode_sxb
    add r0 '00 '18
    ims ra <opcode_sx_helper
    ims ra >opcode_sx_helper
    add rip rpp ra    ; jump



; ==========================================================
; void opcode_sxs(void)
; ==========================================================
; Implements the sxs opcode.
; ==========================================================

=opcode_sxs
    add r0 '00 '10
    ims ra <opcode_sx_helper
    ims ra >opcode_sx_helper
    add rip rpp ra    ; jump



; ==========================================================
; void opcode_sx_helper(int high_bits)
; ==========================================================
; Implements the sxs and sxb opcodes.
; ==========================================================

=opcode_sx_helper

    ; put bits in the template
    ims ra <opcode_sx_template
    ims ra >opcode_sx_template
    add r2 rpp ra
    ; high_bits
    stb r0 r2 '0f
    ; low_bits
    sub r0 '20 r0
    stb r0 r2 '17
    stb r0 r2 '23
    ; low_bits - 1
    sub r0 r0 '01
    stb r0 r2 '03

    ; destination is a register. call parse_register
    ims ra <parse_register
    ims ra >parse_register
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put dest in the template
    ims ra <opcode_sx_template
    ims ra >opcode_sx_template
    add r2 rpp ra
    stb r0 r2 '19
    stb r0 r2 '29

    ; single source argument is mix-type
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put src in the template
    ims ra <opcode_sx_template
    ims ra >opcode_sx_template
    add r2 rpp ra
    stb r0 r2 '02
    stb r0 r2 '1B
    stb r0 r2 '2A

    ; output, tail-call emit_bytes_as_hex()
    add r0 r2 '00
    add r1 '00 '2C
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_sx_template

    ; branch on the high bit
    shru ra r1 '07   ; shru ra <src> <low_bits-1>
    and ra ra '01    ; and ra ra 1
    jz ra &opcode_sx_template_positive

    ; negative, set high bits
    shl ra '01 '18    ; shru ra 1 <high_bits>
    sub ra ra '01     ; sub ra ra 1
    shl ra ra '08     ; shru ra ra <low_bits>
    or r0 ra r1       ; or r0 ra r1
    jz '00 &opcode_sx_template_end

:opcode_sx_template_positive
    ; positive, clear high bits
    shl ra '01 '08   ; shru ra 1 <low_bits>
    sub ra ra '01    ; sub ra ra 1
    and r0 r1 ra     ; and <dest> <src> ra

:opcode_sx_template_end



; ==========================================================
; void opcode_trb(void)
; ==========================================================
; Implements the trb opcode.
; ==========================================================

=opcode_trb
    add r0 '00 '08
    ims ra <opcode_tr_helper
    ims ra >opcode_tr_helper
    add rip rpp ra    ; jump




; ==========================================================
; void opcode_trs(void)
; ==========================================================
; Implements the trs opcode.
; ==========================================================

=opcode_trs
    add r0 '00 '10
    ims ra <opcode_tr_helper
    ims ra >opcode_tr_helper
    add rip rpp ra    ; jump



; ==========================================================
; void opcode_tr_helper(int low_bits)
; ==========================================================
; Implements the trs and trb opcodes.
; ==========================================================

=opcode_tr_helper

    ; put bits in the template
    ims ra <opcode_tr_template
    ims ra >opcode_tr_template
    add r2 rpp ra
    stb r0 r2 '03

    ; destination is a register. call parse_register
    ims ra <parse_register
    ims ra >parse_register
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put dest in the template
    ims ra <opcode_tr_template
    ims ra >opcode_tr_template
    add r2 rpp ra
    stb r0 r2 '09

    ; single source argument is mix-type
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put src in the template
    ims ra <opcode_tr_template
    ims ra >opcode_tr_template
    add r2 rpp ra
    stb r0 r2 '0a

    ; output, tail-call emit_bytes_as_hex()
    add r0 r2 '00
    add r1 '00 '0C
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_tr_template
    shl ra '01 '08   ; shl ra 1 <low_bits>
    sub ra ra '01    ; sub ra ra 1
    and r0 r1 ra     ; and <dest> <src> ra



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
