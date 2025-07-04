; The MIT License (MIT)
;
; Copyright (c) 2023-2025 Fraser Heavy Software
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
; This file contains the implementations of the control opcodes.
;
; The control opcodes are by far the most complicated because immediate loads
; and relative jumps take multi-byte arguments in many forms.



; ==========================================================
; void opcode_short(void);
; ==========================================================
; A helper to parse and emit a short value argument.
;
; This is used by the ims instruction and all of the jump instructions.
;
; Note that this allows 16-bit absolute invocations for jumps which doesn't
; make sense. This isn't allowed; we just don't bother to check. The final
; assembler checks.
;
; TODO the assembly language no longer supports jumps by value; only jumps to
; relative labels are allowed. We need to use this only for the ims instruction
; and have jump instructions parse only a relative label. as/2 has the proper
; restrictions so for now we don't bother with as/1.
; ==========================================================

=opcode_short

    ; call consume_whitespace_and_comments()
    ims ra <consume_whitespace_and_comments
    ims ra >consume_whitespace_and_comments
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; check for a linker directive, call try_parse_linker()
    ims ra <try_parse_linker
    ims ra >try_parse_linker
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if we found one, tail-call opcode_short_linker()
    ; (the control byte is already in r0)
    jz r0 &opcode_short_not_linker
    ims ra <opcode_short_linker
    ims ra >opcode_short_linker
    add rip rpp ra    ; jump
:opcode_short_not_linker

    ; make space for a number
    sub rsp rsp '04
    add r0 rsp '00

    ; check for a number, call try_parse_number()
    ims ra <try_parse_number
    ims ra >try_parse_number
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if we found one, tail-call opcode_short_number()
    jz r0 &opcode_short_not_number
    ldw r0 rsp '00      ; pop r0
    add rsp rsp '04
    ims ra <opcode_short_number
    ims ra >opcode_short_number
    add rip rpp ra    ; jump
:opcode_short_not_number
    add rsp rsp '04     ; popd

    ; otherwise it's individual_bytes. tail-call opcode_short_bytes()
    ims ra <opcode_short_bytes
    ims ra >opcode_short_bytes
    add rip rpp ra    ; jump



; ==========================================================
; static void opcode_short_linker(char control_byte)
; ==========================================================
; Implements the rest of the ims opcode after reading a linker directive.
; ==========================================================

@opcode_short_linker

    ; a linker directive was found. make sure it's a short invocation
    sub ra r0 "<"
    jz ra &opcode_short_linker_ok
    sub ra r0 ">"
    jz ra &opcode_short_linker_ok
    sub ra r0 "&"
    jz ra &opcode_short_linker_ok
    jz '00 &opcode_short_linker_error
:opcode_short_linker_ok

    ; emit it, tail-call emit_linker()
    ims ra <emit_linker
    ims ra >emit_linker
    add rip rpp ra    ; jump

:opcode_short_linker_error
    ims r0 <error_short_value
    ims r0 >error_short_value
    add r0 rpp r0
    ims ra <fatal
    ims ra >fatal
    add rip rpp ra



; ==========================================================
; static void opcode_short_number(int number)
; ==========================================================
; Implements the rest of the ims opcode after reading a number.
;
; Note that we don't check that the number is in range. We just truncate it to
; 16 bits. (The final assembler includes this error check.)
; ==========================================================

@opcode_short_number

    ; stash the number
    sub rsp rsp '04   ; push r0
    stw r0 rsp '00

    ; emit the low byte, call emit_byte_as_hex()
    ims ra <emit_byte_as_hex
    ims ra >emit_byte_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; pop the number, shift it down
    ldw r0 rsp '00    ; pop r0
    add rsp rsp '04
    shru r0 r0 '08

    ; emit the high byte, call emit_byte_as_hex()
    ims ra <emit_byte_as_hex
    ims ra >emit_byte_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; done
    ldw rip '00 rsp   ; ret



; ==========================================================
; static void opcode_short_bytes(void)
; ==========================================================
; Implements the rest of an ims opcode that contains individual bytes. The
; bytes can be characters (i.e. single-character strings) or quoted bytes.
; ==========================================================

@opcode_short_bytes

    ; push an iterator
    sub rsp rsp '04
    stw '02 rsp '00

:opcode_short_bytes_loop

    ; call try_parse_character()
    ims ra <try_parse_character
    ims ra >try_parse_character
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; check whether we got a character
    sub ra r0 'FF
    jz ra &opcode_short_bytes_not_char
    jz '00 &opcode_short_bytes_found
:opcode_short_bytes_not_char

    ; call try_parse_quoted_byte()
    ims ra <try_parse_quoted_byte
    ims ra >try_parse_quoted_byte
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; check whether we got a quoted byte
    sub ra r0 'FF
    jz ra &opcode_short_bytes_error

:opcode_short_bytes_found

    ; we got a byte. emit it, call emit_byte_as_hex()
    ims ra <emit_byte_as_hex
    ims ra >emit_byte_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; decrement iterator. if it's zero, we're done
    ldw r0 rsp '00
    sub r0 r0 '01
    jz r0 &opcode_short_bytes_done
    stw r0 rsp '00

    ; call consume_whitespace_and_comments()
    ims ra <consume_whitespace_and_comments
    ims ra >consume_whitespace_and_comments
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; next
    jz '00 &opcode_short_bytes_loop

:opcode_short_bytes_done
    add rsp rsp '04
    ldw rip '00 rsp   ; ret

:opcode_short_bytes_error
    ims r0 <error_short_value
    ims r0 >error_short_value
    add r0 rpp r0
    ims ra <fatal
    ims ra >fatal
    add rip rpp ra



; ==========================================================
; void opcode_ims(void)
; ==========================================================
; Implements the ims opcode.
; ==========================================================

=opcode_ims

    ; We emit the first two bytes (`ims` and the destination register), then
    ; dispatch to helper functions depending on whether we were given a linker
    ; directive, a number, or individual bytes.

    ; emit ims, call emit_byte_as_hex(0x7C)
    add r0 '00 '7C   ; ims
    ims ra <emit_byte_as_hex
    ims ra >emit_byte_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; destination is a register. call parse_register
    ims ra <parse_register
    ims ra >parse_register
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; emit it, call emit_byte_as_hex()
    ims ra <emit_byte_as_hex
    ims ra >emit_byte_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; tail-call opcode_short()
    ims ra <opcode_short
    ims ra >opcode_short
    add rip rpp ra    ; jump



; ==========================================================
; void opcode_imw(void)
; ==========================================================
; Implements the imw opcode.
;
; The implementation is different depending on whether we have a linker
; directive, a number, or individual bytes. We set up the destination register
; here but otherwise just dispatch to different helper functions.
; ==========================================================

=opcode_imw

    ; destination is a register. call parse_register
    ims ra <parse_register
    ims ra >parse_register
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; put dest into our template
    ims ra <opcode_imw_template
    ims ra >opcode_imw_template
    add ra rpp ra
    stb r0 ra '01
    stb r0 ra '05

    ; call consume_whitespace_and_comments()
    ims ra <consume_whitespace_and_comments
    ims ra >consume_whitespace_and_comments
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; check for a linker directive, call try_parse_linker()
    ims ra <try_parse_linker
    ims ra >try_parse_linker
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if we found one, call opcode_imw_linker(control_byte)
    jz r0 &opcode_imw_not_linker
    ims ra <opcode_imw_linker
    ims ra >opcode_imw_linker
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address
    jz '00 &opcode_imw_ret
:opcode_imw_not_linker

    ; make space for a number
    sub rsp rsp '04
    add r0 rsp '00

    ; check for a number, call try_parse_number()
    ims ra <try_parse_number
    ims ra >try_parse_number
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if we found one, call opcode_imw_number(number)
    jz r0 &opcode_imw_not_number
    ldw r0 rsp '00      ; pop r0
    add rsp rsp '04
    ims ra <opcode_imw_number
    ims ra >opcode_imw_number
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address
    jz '00 &opcode_imw_ret
:opcode_imw_not_number
    add rsp rsp '04     ; popd

    ; otherwise it's individual bytes. call opcode_imw_bytes()
    ims ra <opcode_imw_bytes
    ims ra >opcode_imw_bytes
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

:opcode_imw_ret
    ldw rip '00 rsp   ; ret



; ==========================================================
; static char opcode_imw_template[8];
; ==========================================================
; The data for an immediate 32-bit load used by various opcode functions.
; ==========================================================

@opcode_imw_template
    ;    ims <dest> <value_2> <value_3>
    ;    ims <dest> <value_0> <value_1>
    ims '8F '00 '00
    ims '8F '00 '00



; ==========================================================
; static void opcode_imw_linker(char control_byte)
; ==========================================================
; Implements the rest of the imw opcode after reading a linker directive.
; ==========================================================

@opcode_imw_linker

    ; a linker directive was found. make sure it's a full invocation
    sub ra r0 "^"
    jz ra &opcode_imw_linker_ok
    jz '00 &opcode_imw_linker_error
:opcode_imw_linker_ok

    ; tail-call opcode_32bit_load()
    ims ra <opcode_32bit_load
    ims ra >opcode_32bit_load
    add rip rpp ra    ; jump

:opcode_imw_linker_error
    ims r0 <error_imw_value
    ims r0 >error_imw_value
    add r0 rpp r0
    ims ra <fatal
    ims ra >fatal
    add rip rpp ra



; ==========================================================
; static void opcode_32bit_load(void)
; ==========================================================
; A helper to load an absolute label (in `identifier`) into a register.
;
; The register must already be stored in opcode_imw_template.
;
; TODO don't use the data for this, just take the register as argument
; ==========================================================

=opcode_32bit_load

    ; get the first part of the data
    ims ra <opcode_imw_template
    ims ra >opcode_imw_template
    add r0 rpp ra
    add r1 '00 '02

    ; emit it
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; emit a high invocation, call emit_linker('<')
    add r0 '00 "<"
    ims ra <emit_linker
    ims ra >emit_linker
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the second part of the data (same as the first)
    ims ra <opcode_imw_template
    ims ra >opcode_imw_template
    add r0 rpp ra
    add r1 '00 '02

    ; emit it
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; emit a low invocation, call emit_linker('>')
    add r0 '00 ">"
    ims ra <emit_linker
    ims ra >emit_linker
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; done
    ldw rip '00 rsp   ; ret




; ==========================================================
; static void opcode_imw_number(int number)
; ==========================================================
; Implements the rest of the imw opcode after reading a number.
; ==========================================================

@opcode_imw_number

    ; get the data
    ims ra <opcode_imw_template
    ims ra >opcode_imw_template
    add r1 rpp ra

    ; rotate and place the bytes into the data
    stb r0 r1 '06   ; address of <value_0>
    shru r0 r0 '08
    stb r0 r1 '07   ; address of <value_1>
    shru r0 r0 '08
    stb r0 r1 '02   ; address of <value_2>
    shru r0 r0 '08
    stb r0 r1 '03   ; address of <value_3>

    ; output the data, tail-call emit_bytes_as_hex()
    add r0 r1 '00
    add r1 '00 '08
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump



; ==========================================================
; static void opcode_imw_bytes(void)
; ==========================================================
; Implements the rest of an imw opcode that contains individual bytes. The
; bytes can be characters (i.e. single-character strings) or quoted bytes.
;
; vars:
; - byte iterator: rfp-4
; ==========================================================

@opcode_imw_bytes

    ; set up a stack frame
    sub rsp rsp '04     ; push rfp       ; enter
    stw rfp '00 rsp     ; ...
    add rfp rsp '00     ; mov rfp rsp

    ; set up our variables
    sub rsp rsp '04
    stw '00 rfp 'FC

:opcode_imw_loop

    ; call try_parse_character()
    ims ra <try_parse_character
    ims ra >try_parse_character
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; check whether we got a character
    sub ra r0 'FF
    jz ra &opcode_imw_not_char
    jz '00 &opcode_imw_byte
:opcode_imw_not_char

    ; call try_parse_quoted_byte()
    ims ra <try_parse_quoted_byte
    ims ra >try_parse_quoted_byte
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; check whether we got a quoted byte
    sub ra r0 'FF
    jz ra &opcode_imw_bytes_error

:opcode_imw_byte

    ; we have a byte in r0. get our data
    ims ra <opcode_imw_template
    ims ra >opcode_imw_template
    add r1 rpp ra

    ; figure out where the byte goes
    ldw r2 rfp 'FC
    sub ra r2 '00
    jz ra &opcode_imw_byte_0
    sub ra r2 '01
    jz ra &opcode_imw_byte_1
    sub ra r2 '02
    jz ra &opcode_imw_byte_2
    jz '00 &opcode_imw_byte_3

    ; put it in the template
:opcode_imw_byte_0
    stb r0 r1 '06   ; address of <value_0>
    jz '00 &opcode_imw_byte_done
:opcode_imw_byte_1
    stb r0 r1 '07   ; address of <value_1>
    jz '00 &opcode_imw_byte_done
:opcode_imw_byte_2
    stb r0 r1 '02   ; address of <value_2>
    jz '00 &opcode_imw_byte_done
:opcode_imw_byte_3
    stb r0 r1 '03   ; address of <value_3>
:opcode_imw_byte_done

    ; bump iterator. if it's 4, we're done reading
    add r2 r2 '01
    sub ra r2 '04
    jz ra &opcode_imw_emit_template
    stw r2 rfp 'FC

    ; call consume_whitespace_and_comments()
    ims ra <consume_whitespace_and_comments
    ims ra >consume_whitespace_and_comments
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; next
    jz '00 &opcode_imw_loop

:opcode_imw_emit_template

    ; get our data
    ims ra <opcode_imw_template
    ims ra >opcode_imw_template
    add r0 rpp ra
    add r1 '00 '08

    ; output it
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; done
    add rsp rfp '00     ; mov rsp rfp   ; leave
    ldw rfp '00 rsp     ; pop rfp       ; ^^^
    add rsp rsp '04     ; ^^^           ; ^^^
    ldw rip '00 rsp     ; ret

:opcode_imw_bytes_error
    ims r0 <error_imw_value
    ims r0 >error_imw_value
    add r0 rpp r0
    ims ra <fatal
    ims ra >fatal
    add rip rpp ra



; ==========================================================
; void opcode_jz(void)
; ==========================================================
; Implements the jz opcode.
; ==========================================================

=opcode_jz

    ; emit jz, call emit_byte_as_hex(0x7E)
    add r0 '00 '7E   ; jz
    ims ra <emit_byte_as_hex
    ims ra >emit_byte_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; predicate is mix. call parse_mix()
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; emit it, call emit_byte_as_hex()
    ims ra <emit_byte_as_hex
    ims ra >emit_byte_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; tail-call opcode_short()
    ; TODO: we should consider making all jump opcodes accept only a relative
    ; label. would greatly simplify the language. relative jump by number isn't
    ; necessarily a good idea to allow anyway because the number doesn't
    ; correspond to the compound instructions, and you shouldn't have to know
    ; how many bytecode instructions a compound instruction emits
    ims ra <opcode_short
    ims ra >opcode_short
    add rip rpp ra    ; jump



; ==========================================================
; void opcode_cmpu(void)
; ==========================================================
; Implements the cmpu opcode.
;
; TODO this only exists temporarily until we convert the compiler to use
; ltu/lts.
; ==========================================================

=opcode_cmpu

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
    ims ra <opcode_cmpu_template
    ims ra >opcode_cmpu_template
    add r0 rpp ra
    add r1 '00 '24    ; 36 bytes

    ; fill it in
    ldb r2 rsp '00    ; load dest
    stb r2 r0 '09
    stb r2 r0 '19
    stb r2 r0 '21
    ldb r2 rsp '01    ; load src1
    stb r2 r0 '02
    stb r2 r0 '13
    ldb r2 rsp '02    ; load src2
    stb r2 r0 '03
    stb r2 r0 '12

    ; pop the arguments
    add rsp rsp '04   ; popd

    ; output it, tail-call emit_bytes_as_hex()
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_cmpu_template
    ltu ra '81 '82                      ; ltu ra arg1 arg2
    jz ra &opcode_cmpu_template_ge      ; jz ra &opcode_cmpu_template_ge
    add '80 '00 'ff                     ; add dest 0 -1                     ; mov dest -1
    jz '00 &opcode_cmpu_template_end    ; jz 0 &opcode_cmpu_template_end
:opcode_cmpu_template_ge
    ltu ra '82 '81                      ; ltu ra arg2 arg1
    jz ra &opcode_cmpu_template_eq      ; jz ra &opcode_cmpu_template_eq
    add '80 '00 '01                     ; add dest 0 1                      ; mov dest 1
    jz '00 &opcode_cmpu_template_end    ; jz 0 +1
:opcode_cmpu_template_eq
    add '80 '00 '00                     ; add dest 0 0                      ; zero dest
:opcode_cmpu_template_end



; ==========================================================
; void opcode_cmps(void)
; ==========================================================
; Implements the cmps opcode.
;
; TODO this only exists temporarily until we convert the compiler to use
; ltu/lts.
; ==========================================================

=opcode_cmps

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
    ims ra <opcode_cmps_template
    ims ra >opcode_cmps_template
    add r0 rpp ra
    add r1 '00 '3C    ; 60 bytes

    ; fill it in
    ldb r2 rsp '00    ; load dest
    stb r2 r0 '15
    stb r2 r0 '31
    stb r2 r0 '39
    ldb r2 rsp '01    ; load src1
    stb r2 r0 '06
    stb r2 r0 '22
    ldb r2 rsp '02    ; load src2
    stb r2 r0 '0A
    stb r2 r0 '26

    ; pop the arguments
    add rsp rsp '04   ; popd

    ; output it, tail-call emit_bytes_as_hex()
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_cmps_template
    shl rb '01 '1F                      ; shl rb 1 31       ; rb = 0x80000000
    add ra '81 rb                       ; add ra arg1 rb
    add rb '82 rb                       ; add rb arg2 rb
    ltu ra ra rb                        ; ltu ra ra rb
    jz ra &opcode_cmps_template_ge      ; jz ra &opcode_cmps_template_ge
    add '80 '00 'ff                     ; add dest 0 -1                     ; mov dest -1
    jz '00 &opcode_cmps_template_end    ; jz 0 &opcode_cmps_template_end
:opcode_cmps_template_ge
    shl rb '01 '1F                      ; shl rb 1 31       ; rb = 0x80000000
    add ra '81 rb                       ; add ra arg1 rb
    add rb '82 rb                       ; add rb arg2 rb
    ltu ra rb ra                        ; ltu ra rb ra
    jz ra &opcode_cmps_template_eq      ; jz ra &opcode_cmps_template_eq
    add '80 '00 '01                     ; add dest 0 1                      ; mov dest 1
    jz '00 &opcode_cmps_template_end    ; jz 0 &opcode_cmps_template_end
:opcode_cmps_template_eq
    add '80 '00 '00                     ; add dest 0 0                      ; zero dest
:opcode_cmps_template_end



; ==========================================================
; void opcode_ltu(void)
; ==========================================================
; Implements the ltu opcode.
; ==========================================================

=opcode_ltu
    add r0 '00 '7D  ; ltu
    ims ra <opcode_standard
    ims ra >opcode_standard
    add rip rpp ra



; ==========================================================
; void opcode_lts(void)
; ==========================================================
; Implements the lts opcode.
; ==========================================================

=opcode_lts

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
    ims ra <opcode_lts_template
    ims ra >opcode_lts_template
    add r0 rpp ra
    add r1 '00 '10

    ; fill it in
    ldb r2 rsp '00    ; load dest
    stb r2 r0 '0D
    ldb r2 rsp '01    ; load src1
    stb r2 r0 '06
    ldb r2 rsp '02    ; load src2
    stb r2 r0 '0A

    ; pop the arguments
    add rsp rsp '04   ; popd

    ; output it, tail-call emit_bytes_as_hex()
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

=opcode_lts_template
    shl rb '01 '1F   ; shl rb 1 31       ; rb = 0x80000000
    add ra '81 rb    ; add ra src1 rb
    add rb '82 rb    ; add rb src2 rb
    ltu '80 ra rb    ; ltu dest ra rb



; ==========================================================
; void opcode_sys(void)
; ==========================================================
; Implements the sys opcode.
; ==========================================================

=opcode_sys

    ; call consume_whitespace_and_comments()
    ims ra <consume_whitespace_and_comments
    ims ra >consume_whitespace_and_comments
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; TODO allow a number or quoted byte for syscall. For now just identifier.

    ; call try_parse_identifier()
    ims ra <try_parse_identifier
    ims ra >try_parse_identifier
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address
    jz r0 &opcode_sys_error

    ; identifier found. call convert_keyword(syscalls)
    ims ra <syscalls
    ims ra >syscalls
    add r0 rpp ra
    ims ra <convert_keyword
    ims ra >convert_keyword
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if it failed to convert, fatal error
    sub ra r0 'FF
    jz ra &opcode_sys_error

    ; get our data, put the byte in it
    add r2 r0 '00
    ims ra <opcode_sys_template
    ims ra >opcode_sys_template
    add r0 rpp ra
    add r1 '00 '04
    stb r2 r0 '01

    ; output it, call emit_bytes_as_hex()
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; we need two zeroes to follow sys. these can be numbers or quoted bytes.
    ; we don't bother to check what they are, just parse mix twice and ignore.

    ; call parse_mix()
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; call parse_mix()
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; done
    ldw rip '00 rsp        ; ret

:opcode_sys_error
    ims r0 <error_sys
    ims r0 >error_sys
    add r0 rpp r0
    ims ra <fatal
    ims ra >fatal
    add rip rpp ra

@opcode_sys_template
    '7F '03 '00 '00



; ==========================================================
; void opcode_jnz(void)
; ==========================================================
; Implements the jnz opcode.
; ==========================================================

=opcode_jnz

    ; predicate is mix. call parse_mix()
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the data, put the predicate in it
    add r2 r0 '00
    ims ra <opcode_jnz_template
    ims ra >opcode_jnz_template
    add r0 rpp ra
    add r1 '00 '06
    stb r2 r0 '01

    ; output it, call emit_bytes_as_hex()
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; tail-call opcode_short()
    ims ra <opcode_short
    ims ra >opcode_short
    add rip rpp ra    ; jump

@opcode_jnz_template
    ; jz <pred> +1
    ; jz 0 <label>
    '7E 'FF '01 '00
    '7E '00 'FF 'FF


; ==========================================================
; void opcode_jmp(void)
; ==========================================================
; Implements the jmp opcode.
; ==========================================================

=opcode_jmp

    ; call consume_whitespace_and_comments()
    ims ra <consume_whitespace_and_comments
    ims ra >consume_whitespace_and_comments
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; jmp can take an absolute label which outputs very different code. check
    ; that first.

    ; check for a linker directive, call try_parse_linker()
    ims ra <try_parse_linker
    ims ra >try_parse_linker
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if we found one, tail-call opcode_jmp_linker()
    ; (the control byte is already in r0)
    jz r0 &opcode_jmp_not_linker
    ims ra <opcode_jmp_linker
    ims ra >opcode_jmp_linker
    add rip rpp ra    ; jump
:opcode_jmp_not_linker

    ; otherwise we emit a conditional jump with zero predicate.

    ; emit jz, call emit_byte_as_hex(0x7E)
    add r0 '00 '7E   ; jz
    ims ra <emit_byte_as_hex
    ims ra >emit_byte_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; emit 0, call emit_byte_as_hex(0x00)
    add r0 '00 '00
    ims ra <emit_byte_as_hex
    ims ra >emit_byte_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; tail-call opcode_short()
    ; (note that this gives a slightly misleading error message on an invalid
    ; jmp: it implies that an absolute label is not allowed. it's not easy or
    ; worthwhile to fix this; the final assembler does it properly.)
    ims ra <opcode_short
    ims ra >opcode_short
    add rip rpp ra    ; jump



; ==========================================================
; void opcode_jmp_linker(char control_byte)
; ==========================================================
; Implements the jmp opcode when given a linker directive (which has already
; been parsed.)
; ==========================================================

=opcode_jmp_linker

    ; figure out whether we are relative or absolute
    sub ra r0 "&"
    jz ra &opcode_jmp_linker_relative
    sub ra r0 "^"
    jz ra &opcode_jmp_linker_absolute
    jz '00 &opcode_jmp_linker_error

:opcode_jmp_linker_relative

    ; emit a conditional jump with zero predicate.

    ; emit jz, call emit_byte_as_hex(0x7E)
    add r0 '00 '7E   ; jz
    ims ra <emit_byte_as_hex
    ims ra >emit_byte_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; emit 0, call emit_byte_as_hex(0x00)
    add r0 '00 '00
    ims ra <emit_byte_as_hex
    ims ra >emit_byte_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; emit the invocation, tail-call emit_linker('&')
    add r0 '00 "&"
    ims ra <emit_linker
    ims ra >emit_linker
    add rip rpp ra    ; jump

:opcode_jmp_linker_absolute

    ; put register ra into the 32-bit load template
    ims ra <opcode_imw_template
    ims ra >opcode_imw_template
    add ra rpp ra
    ims rb '8A '00
    stb rb ra '01
    stb rb ra '05

    ; emit an immediate load of the label, call opcode_32bit_load()
    ims ra <opcode_32bit_load
    ims ra >opcode_32bit_load
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the jump data
    ims ra <opcode_jmp_linker_absolute_template
    ims ra >opcode_jmp_linker_absolute_template
    add r0 rpp ra
    add r1 '00 '04

    ; output the data, tail-call emit_bytes_as_hex()
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

:opcode_jmp_linker_error
    ims r0 <error_jmp_linker
    ims r0 >error_jmp_linker
    add r0 rpp r0
    ims ra <fatal
    ims ra >fatal
    add rip rpp ra

=opcode_jmp_linker_absolute_template
    add rip rpp ra


; ==========================================================
; void opcode_enter(void)
; ==========================================================
; Implements the enter opcode.
; ==========================================================

=opcode_enter

    ; the enter opcode takes no arguments.

    ; get the data
    ims ra <opcode_enter_template
    ims ra >opcode_enter_template
    add r0 rpp ra
    add r1 '00 '0C

    ; tail-call emit_bytes_as_hex()
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

@opcode_enter_template
    sub rsp rsp '04     ; push rfp
    stw rfp '00 rsp     ; ...
    add rfp rsp '00     ; mov rfp rsp



; ==========================================================
; void opcode_leave(void)
; ==========================================================
; Implements the leave opcode.
; ==========================================================

=opcode_leave

    ; the leave opcode takes no arguments.

    ; get the data
    ims ra <opcode_leave_template
    ims ra >opcode_leave_template
    add r0 rpp ra
    add r1 '00 '0C

    ; tail-call emit_bytes_as_hex()
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

@opcode_leave_template
    add rsp rfp '00     ; mov rsp rfp
    ldw rfp '00 rsp     ; pop rfp
    add rsp rsp '04     ; ...



; ==========================================================
; void opcode_call(void)
; ==========================================================
; Implements the call opcode.
;
; This handles an absolute invocation (i.e. a normal call to a named function.)
; If it's a register instead, we forward to opcode_call_register().
; ==========================================================

=opcode_call

    ; call consume_whitespace_and_comments()
    ims ra <consume_whitespace_and_comments
    ims ra >consume_whitespace_and_comments
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; call try_parse_linker()
    ims ra <try_parse_linker
    ims ra >try_parse_linker
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if it failed, we must be calling by register
    jz r0 &opcode_call_forward_register

    ; we have a linker directive. it must be a 32 bit invocation
    sub ra r0 "^"
    jz ra &opcode_call_ok
    jz '00 &opcode_call_error
:opcode_call_ok

    ; get the load data
    ims ra <opcode_call_template_load
    ims ra >opcode_call_template_load
    add r0 rpp ra
    add r1 '00 '02

    ; output it, call emit_bytes_as_hex()
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; emit a high invocation, call emit_linker('<')
    add r0 '00 "<"
    ims ra <emit_linker
    ims ra >emit_linker
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the load data again
    ims ra <opcode_call_template_load
    ims ra >opcode_call_template_load
    add r0 rpp ra
    add r1 '00 '02

    ; output it again, call emit_bytes_as_hex()
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; emit a low invocation, call emit_linker('>')
    add r0 '00 ">"
    ims ra <emit_linker
    ims ra >emit_linker
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the jump-and-link data
    ims ra <opcode_call_template_jump
    ims ra >opcode_call_template_jump
    add r0 rpp ra
    add r1 '00 '14

    ; tail-call emit_bytes_as_hex()
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

:opcode_call_error
    ims r0 <error_call
    ims r0 >error_call
    add r0 rpp r0
    ims ra <fatal
    ims ra >fatal
    add rip rpp ra

:opcode_call_forward_register
    ; tail-call opcode_call_register
    ims ra <opcode_call_register
    ims ra >opcode_call_register
    add rip rpp ra  ; jump

@opcode_call_template_load
    ; we only use the first two bytes.
    ims ra '00 '00

@opcode_call_template_jump
    ; no replacements, these instructions are the same for every call.
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra      ; jump
    add rsp rsp '04     ; pop return address



; ==========================================================
; void opcode_call_register(void)
; ==========================================================
; Implements the call opcode for a register (i.e. calling a function pointer.)
; ==========================================================

=opcode_call_register

    ; call parse_register()
    ; TODO it would be nice to have try_parse_register() here so we can give a
    ; better error message (otherwise an incorrect invocation argument to a
    ; call instruction gives "Expected register".)
    ims ra <parse_register
    ims ra >parse_register
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the data, put the register in it
    add r2 r0 '00
    ims ra <opcode_call_register_template
    ims ra >opcode_call_register_template
    add r0 rpp ra
    add r1 '00 '14   ; template is twenty bytes
    stb r2 r0 '0F

    ; output it, tail-call emit_bytes_as_hex()
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

@opcode_call_register_template
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip '00 '80     ; jump
    add rsp rsp '04     ; pop return address



; ==========================================================
; void opcode_ret(void)
; ==========================================================
; Implements the ret opcode.
;
; This simply jumps to the address on top of the stack.
; ==========================================================

=opcode_ret

    ; the ret opcode takes no arguments.

    ; get the data
    ims ra <opcode_ret_template
    ims ra >opcode_ret_template
    add r0 rpp ra
    add r1 '00 '04

    ; tail-call emit_bytes_as_hex()
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    add rip rpp ra    ; jump

@opcode_ret_template
    ldw rip '00 rsp   ; ret
