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
; This file contains the implementations of the memory opcodes.



; ==========================================================
; void opcode_store(uint8_t opcode);
; ==========================================================
; A helper to implement the two store opcodes.
; ==========================================================

=opcode_store
    ; we don't use a separate template symbol since we need to write all four
    ; bytes. instead we just form it on the stack.

    ; make space on the stack
    ;    <opcode> <value> <base> <offset>
    sub rsp rsp '04

    ; put opcode in the template
    stb r0 rsp '00

    ; value is mix-type
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put value in the template
    stb r0 rsp '01

    ; base is mix-type
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put base in the template
    stb r0 rsp '02

    ; offset is mix-type
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put offset in the template
    stb r0 rsp '03

    ; output
    add r0 rsp '00
    add r1 '00 '04
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; done
    add rsp rsp '04   ; popd (template)
    ldw rip '00 rsp        ; ret



; ==========================================================
; void opcode_ldw(void)
; ==========================================================
; Implements the ldw opcode.
; ==========================================================

=opcode_ldw
    add r0 '00 '78   ; ldw
    ims ra <opcode_standard
    ims ra >opcode_standard
    add rip rpp ra



; ==========================================================
; void opcode_stw(void)
; ==========================================================
; Implements the stw opcode.
; ==========================================================

=opcode_stw
    add r0 '00 '79   ; stw
    ims ra <opcode_store
    ims ra >opcode_store
    add rip rpp ra



; ==========================================================
; void opcode_ldb(void)
; ==========================================================
; Implements the ldb opcode.
; ==========================================================

=opcode_ldb
    add r0 '00 '7A   ; ldb
    ims ra <opcode_standard
    ims ra >opcode_standard
    add rip rpp ra



; ==========================================================
; void opcode_stb(void)
; ==========================================================
; Implements the stb opcode.
; ==========================================================

=opcode_stb
    add r0 '00 '7B   ; stb
    ims ra <opcode_store
    ims ra >opcode_store
    add rip rpp ra



; ==========================================================
; void opcode_lds(void)
; ==========================================================
; Implements the lds opcode.
; ==========================================================

=opcode_lds

    ; destination is a register. call parse_register
    ims ra <parse_register
    ims ra >parse_register
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put dest in the template
    ims ra <opcode_lds_template
    ims ra >opcode_lds_template
    add r2 rpp ra
    stb r0 r2 '11

    ; base argument is mix-type
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put base in the template
    ims ra <opcode_lds_template
    ims ra >opcode_lds_template
    add r2 rpp ra
    stb r0 r2 '02

    ; offset argument is mix-type
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put offset in the template
    ims ra <opcode_lds_template
    ims ra >opcode_lds_template
    add r2 rpp ra
    stb r0 r2 '03

    ; output, tail-call emit_bytes_as_hex()
    add r0 r2 '00
    add r1 '00 '14
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_lds_template
    add rb r1 r2     ; add rb <base> <offset>
    ldb ra rb '00    ; ldb ra rb 0
    ldb rb rb '01    ; ldb rb rb 1
    shl rb rb '08    ; shl rb rb 8
    or r0 ra rb      ; or <dest> ra rb



; ==========================================================
; void opcode_sts(void)
; ==========================================================
; Implements the sts opcode.
; ==========================================================

=opcode_sts

    ; value is mix-type
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put value in the template
    ims ra <opcode_sts_template
    ims ra >opcode_sts_template
    add r2 rpp ra
    stb r0 r2 '05
    stb r0 r2 '0A

    ; base argument is mix-type
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put base in the template
    ims ra <opcode_sts_template
    ims ra >opcode_sts_template
    add r2 rpp ra
    stb r0 r2 '02

    ; offset argument is mix-type
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put offset in the template
    ims ra <opcode_sts_template
    ims ra >opcode_sts_template
    add r2 rpp ra
    stb r0 r2 '03

    ; output, tail-call emit_bytes_as_hex()
    add r0 r2 '00
    add r1 '00 '10
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_sts_template
    add rb r1 r2     ; add rb <base> <offset>
    stb r0 rb '00    ; stb <dest> rb 0
    shru ra r0 '08   ; shru ra <dest> 8
    stb ra rb '01    ; stb ra rb 1



; ==========================================================
; void opcode_push(void)
; ==========================================================
; Implements the push opcode.
; ==========================================================

=opcode_push

    ; argument is mix-type, call parse_mix()
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put it in the template
    ims ra <opcode_push_template
    ims ra >opcode_push_template
    add r2 rpp ra
    stb r0 r2 '05

    ; output, tail-call emit_bytes_as_hex()
    add r0 r2 '00
    add r1 '00 '08
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_push_template
    sub rsp rsp '04   ; sub rsp rsp '04
    stw r0 rsp '00    ; stw <value> rsp '00



; ==========================================================
; void opcode_pop(void)
; ==========================================================
; Implements the pop opcode.
; ==========================================================

=opcode_pop

    ; argument is a register, call parse_register()
    ims ra <parse_register
    ims ra >parse_register
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put it in the template
    ims ra <opcode_pop_template
    ims ra >opcode_pop_template
    add r2 rpp ra
    stb r0 r2 '01

    ; output, tail-call emit_bytes_as_hex()
    add r0 r2 '00
    add r1 '00 '08
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_pop_template
    ldw r0 rsp '00    ; ldw <dest> rsp '00
    add rsp rsp '04   ; add rsp rsp '04



;;;; ==========================================================
;;;; void opcode_pushn(void)
;;;; ==========================================================
;;;; Implements the pushn opcode.
;;;;
;;;; TODO not going to implement this for now, might not need it
;;;; ==========================================================
;;;
;;;
;;;=opcode_pushn
;;;    ims r0 <error_not_implemented
;;;    ims r0 >error_not_implemented
;;;    add r0 rpp r0
;;;    ims ra <fatal
;;;    ims ra >fatal
;;;    add rip rpp ra
;;;
;;;=error_not_implemented
;;;    "Not implemented." '00
;;;
;;;
;;;
;;;; ==========================================================
;;;; void opcode_popn(void)
;;;; ==========================================================
;;;; Implements the popn opcode.
;;;;
;;;; TODO not going to implement this for now, might not need it
;;;; ==========================================================
;;;
;;;=opcode_popn
;;;    ims r0 <error_not_implemented
;;;    ims r0 >error_not_implemented
;;;    add r0 rpp r0
;;;    ims ra <fatal
;;;    ims ra >fatal
;;;    add rip rpp ra



; ==========================================================
; void opcode_popd(void)
; ==========================================================
; Implements the popd opcode.
; ==========================================================

=opcode_popd

    ; the popd opcode takes no arguments.

    ; get the template
    ims ra <opcode_popd_template
    ims ra >opcode_popd_template
    add r0 rpp ra
    add r1 '00 '04

    ; tail-call emit_bytes_as_hex()
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

@opcode_popd_template
    add rsp rsp '04     ; popd
