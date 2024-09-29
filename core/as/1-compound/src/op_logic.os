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
; This file contains the implementations of the logic opcodes.



; ==========================================================
; void opcode_and(void)
; ==========================================================
; Implements the and opcode.
; ==========================================================

=opcode_and
    add r0 '00 '74   ; and
    ims ra <opcode_standard
    ims ra >opcode_standard
    add rip rpp ra



; ==========================================================
; void opcode_or(void)
; ==========================================================
; Implements the or opcode.
; ==========================================================

=opcode_or
    add r0 '00 '75   ; or
    ims ra <opcode_standard
    ims ra >opcode_standard
    add rip rpp ra



; ==========================================================
; void opcode_xor(void)
; ==========================================================
; Implements the xor opcode.
; ==========================================================

=opcode_xor

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
    ims ra <opcode_xor_template
    ims ra >opcode_xor_template
    add r0 rpp ra
    add r1 '00 '0C

    ; fill it in
    ldb r2 rsp '00    ; load dest
    stb r2 r0 '09
    ldb r2 rsp '01    ; load src1
    stb r2 r0 '02
    stb r2 r0 '06
    ldb r2 rsp '02    ; load src2
    stb r2 r0 '03
    stb r2 r0 '07

    ; pop the arguments
    add rsp rsp '04   ; popd

    ; output it, tail-call emit_bytes_as_hex()
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_xor_template
    or ra '01 '02     ; or ra <src1> <src2>
    and rb '01 '02    ; and rb <src1> <src2>
    sub r0 ra rb      ; sub <dest> ra rb



;; ==========================================================
;; void opcode_ror(void)
;; ==========================================================
;; Implements the ror opcode.
;; ==========================================================
;
;=opcode_ror
;    add r0 '00 '77   ; ror
;    ims ra <opcode_standard
;    ims ra >opcode_standard
;    add rip rpp ra



; ==========================================================
; void opcode_mov(void)
; ==========================================================
; Implements the mov opcode.
; ==========================================================

=opcode_mov

    ; destination is a register. call parse_register
    ims ra <parse_register
    ims ra >parse_register
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put dest in the template
    ims ra <opcode_mov_template
    ims ra >opcode_mov_template
    add r2 rpp ra
    stb r0 r2 '01

    ; single source argument is mix-type
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put src in the template
    ims ra <opcode_mov_template
    ims ra >opcode_mov_template
    add r2 rpp ra
    stb r0 r2 '03

    ; output, tail-call emit_bytes_as_hex()
    add r0 r2 '00
    add r1 '00 '04
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_mov_template
    add r0 '00 r1   ; add <dest> 0 <src>



; ==========================================================
; void opcode_not(void)
; ==========================================================
; Implements the not opcode.
; ==========================================================

=opcode_not
    ; get the data

    ; destination is a register. call parse_register
    ims ra <parse_register
    ims ra >parse_register
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put dest into our template
    ims ra <opcode_not_template
    ims ra >opcode_not_template
    add r2 rpp ra
    stb r0 r2 '01

    ; source is mix-type. call parse_mix
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put source into our template
    ims ra <opcode_not_template
    ims ra >opcode_not_template
    add r2 rpp ra
    stb r0 r2 '03

    ; tail-call emit_bytes_as_hex()
    add r0 r2 '00
    add r1 '00 '04
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

@opcode_not_template
    sub '81 'FF '82



;; ==========================================================
;; void opcode_rol(void)
;; ==========================================================
;; Implements the rol opcode.
;; ==========================================================
;
;=opcode_rol
;
;    ; push space to parse arguments
;    sub rsp rsp '04
;    add r0 rsp '00
;
;    ; call parse_register_mix_mix()
;    ims ra <parse_register_mix_mix
;    ims ra >parse_register_mix_mix
;    sub rsp rsp '04     ; push return address
;    add rb rip '08
;    stw rb '00 rsp
;    add rip rpp ra    ; jump
;    add rsp rsp '04     ; pop return address
;
;    ; get the template
;    ims ra <opcode_rol_template
;    ims ra >opcode_rol_template
;    add r0 rpp ra
;    add r1 '00 '08
;
;    ; fill it in
;    ldb r2 rsp '00    ; load dest
;    stb r2 r0 '05
;    ldb r2 rsp '01    ; load src
;    stb r2 r0 '06
;    ldb r2 rsp '02    ; load bits
;    stb r2 r0 '03
;
;    ; pop the arguments
;    add rsp rsp '04   ; popd
;
;    ; output it, tail-call emit_bytes_as_hex()
;    ims ra <emit_bytes_as_hex
;    ims ra >emit_bytes_as_hex
;    add rip rpp ra    ; jump
;
;=opcode_rol_template
;    sub ra '20 '82   ; sub ra 32 bits
;    ror '80 '81 ra   ; ror dest src ra



; ==========================================================
; void opcode_shrs(void)
; ==========================================================
; Implements the shrs opcode.
; ==========================================================

=opcode_shrs

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
    ims ra <opcode_shrs_template
    ims ra >opcode_shrs_template
    add r0 rpp ra
    add r1 '00 '28

    ; fill it in
    ldb r2 rsp '00    ; load dest
    stb r2 r0 '1D
    stb r2 r0 '25
    ldb r2 rsp '01    ; load src
    stb r2 r0 '02
    stb r2 r0 '1A
    stb r2 r0 '26
    ldb r2 rsp '02    ; load bits
    stb r2 r0 '09
    stb r2 r0 '0F
    stb r2 r0 '1B
    stb r2 r0 '27

    ; pop the arguments
    add rsp rsp '04   ; popd

    ; output it, tail-call emit_bytes_as_hex()
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_shrs_template

    ; test the sign bit
    shru ra r1 '1F
    jz ra &opcode_shrs_template_nonnegative

    ; negative. if bits is zero, jump to unsigned shift (to copy to dest)
    jz r2 &opcode_shrs_template_nonnegative
    ; generate a mask
    sub rb '20 r2
    shl rb '01 rb
    sub rb '00 rb
    ; shift and apply inverted mask
    shru ra r1 r2
    or r0 ra rb
    jz '00 &opcode_shrs_template_done

:opcode_shrs_template_nonnegative
    shru r0 r1 r2

:opcode_shrs_template_done



; ==========================================================
; void opcode_shru(void)
; ==========================================================
; Implements the shru opcode.
; ==========================================================

=opcode_shru
    add r0 '00 '77   ; shru
    ims ra <opcode_standard
    ims ra >opcode_standard
    add rip rpp ra



; ==========================================================
; void opcode_shl(void)
; ==========================================================
; Implements the shl opcode.
; ==========================================================

=opcode_shl
    add r0 '00 '76   ; shl
    ims ra <opcode_standard
    ims ra >opcode_standard
    add rip rpp ra



; ==========================================================
; void opcode_bool(void)
; ==========================================================
; Implements the bool opcode.
; ==========================================================

=opcode_bool

    ; destination is a register. call parse_register
    ims ra <parse_register
    ims ra >parse_register
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put dest in the template
    ims ra <opcode_bool_template
    ims ra >opcode_bool_template
    add r2 rpp ra
    stb r0 r2 '05
    stb r0 r2 '0D

    ; value is mix-type
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put value in the template
    ims ra <opcode_bool_template
    ims ra >opcode_bool_template
    add r2 rpp ra
    stb r0 r2 '01

    ; output, tail-call emit_bytes_as_hex()
    add r0 r2 '00
    add r1 '00 '10
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_bool_template
    jz r1 &opcode_bool_template_zero
    add r0 '00 '01
    jz '00 &opcode_bool_template_end
:opcode_bool_template_zero
    add r0 '00 '00
:opcode_bool_template_end



; ==========================================================
; void opcode_isz(void)
; ==========================================================
; Implements the isz opcode.
; ==========================================================

=opcode_isz

    ; destination is a register. call parse_register
    ims ra <parse_register
    ims ra >parse_register
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put dest in the template
    ims ra <opcode_isz_template
    ims ra >opcode_isz_template
    add r2 rpp ra
    stb r0 r2 '05
    stb r0 r2 '0D

    ; value is mix-type
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put value in the template
    ims ra <opcode_isz_template
    ims ra >opcode_isz_template
    add r2 rpp ra
    stb r0 r2 '01

    ; output, tail-call emit_bytes_as_hex()
    add r0 r2 '00
    add r1 '00 '10
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_isz_template
    jz r1 &opcode_isz_template_zero
    add r0 '00 '00
    jz '00 &opcode_isz_template_end
:opcode_isz_template_zero
    add r0 '00 '01
:opcode_isz_template_end
