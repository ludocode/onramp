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
; This file contains functions to parse data from the input file.



; ==========================================================
; char* registers;
; ==========================================================
; A null-terminated list of register names. Each name is followed by its
; replacement byte.
; ==========================================================

=registers
    "r0"  '00 '80
    "r1"  '00 '81
    "r2"  '00 '82
    "r3"  '00 '83
    "r4"  '00 '84
    "r5"  '00 '85
    "r6"  '00 '86
    "r7"  '00 '87
    "r8"  '00 '88
    "r9"  '00 '89
    "ra"  '00 '8A
    "rb"  '00 '8B
    "rsp" '00 '8C
    "rfp" '00 '8D
    "rpp" '00 '8E
    "rip" '00 '8F
    '00



; ==========================================================
; char* syscalls;
; ==========================================================
; A null-terminated list of syscall names. Each name is followed by its
; replacement byte.
; ==========================================================

=syscalls
    ;  system
    "halt"      '00 '00
    "time"      '00 '01
    "spawn"     '00 '02
    ;  io handles
    "open"      '00 '10
    "close"     '00 '11
    "read"      '00 '12
    "write"     '00 '13
    "seek"      '00 '14
    "tell"      '00 '15
    "truncate"  '00 '16
    "type"      '00 '17
    ;  filesystem
    "stat"      '00 '20
    "rename"    '00 '21
    "symlink"   '00 '22
    "unlink"    '00 '23
    "chmod"     '00 '24
    "mkdir"     '00 '25
    "rmdir"     '00 '26
    '00



; ==========================================================
; char identifier[128];
; ==========================================================
; A fixed-size buffer for storing keywords and label names.
;
; The final stage assembler has no limit on identifier length.
; ==========================================================

=identifier
    '00 '00 '00 '00 '00 '00 '00 '00  '00 '00 '00 '00 '00 '00 '00 '00
    '00 '00 '00 '00 '00 '00 '00 '00  '00 '00 '00 '00 '00 '00 '00 '00
    '00 '00 '00 '00 '00 '00 '00 '00  '00 '00 '00 '00 '00 '00 '00 '00
    '00 '00 '00 '00 '00 '00 '00 '00  '00 '00 '00 '00 '00 '00 '00 '00
    '00 '00 '00 '00 '00 '00 '00 '00  '00 '00 '00 '00 '00 '00 '00 '00
    '00 '00 '00 '00 '00 '00 '00 '00  '00 '00 '00 '00 '00 '00 '00 '00
    '00 '00 '00 '00 '00 '00 '00 '00  '00 '00 '00 '00 '00 '00 '00 '00
    '00 '00 '00 '00 '00 '00 '00 '00  '00 '00 '00 '00 '00 '00 '00 '00



; ==========================================================
; static bool try_parse_whitespace(void);
; ==========================================================
; Attempts to consume a whitespace character.
;
; Returns true if consumed, false otherwise.
;
; When a newline is encountered, a newline is emitted to the output and the
; current line is incremented.
; ==========================================================

@try_parse_whitespace

    ; get the current char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; check if it's a newline. these are handled separately.
    cmpu ra r0 '0A   ; (line feed)
    jz ra &try_parse_whitespace_newline

    ; check if it's whitespace, call isspace()
    ims ra <isspace
    ims ra >isspace
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if false, return false
    jz r0 &try_parse_whitespace_false

    ; consume the character, call next_char()
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; return true
    add r0 '00 '01        ; mov r0 1
:try_parse_whitespace_false
    ldw rip '00 rsp     ; ret

:try_parse_whitespace_newline

    ; when we encounter a newline, we emit a newline.

    ; call emit_byte('\n')  ('\n' is already in r0)
    ims ra <emit_byte
    ims ra >emit_byte
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; consume the newline, call next_char()
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; increment current_line
    ims r0 <current_line
    ims r0 >current_line
    ldw r1 rpp r0
    add r1 r1 '01
    stw r1 rpp r0

    ; return true
    add r0 '00 '01        ; mov r0 1
    ldw rip '00 rsp     ; ret



; ==========================================================
; void try_parse_comment(void);
; ==========================================================
; Attempts to consume a comment.
;
; Returns true if consumed, false otherwise.
; ==========================================================

=try_parse_comment

    ; get the current char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; check if it's the start of a comment
    cmpu ra r0 ";"
    jz ra &try_parse_comment_loop

    ; TODO for now we also treat debug info as comments. The final stage linker
    ; supports debug info so we don't really need to implement this.
    cmpu ra r0 "#"
    jz ra &try_parse_comment_loop

    ; not a comment, return false
    add r0 '00 '00
    ldw rip '00 rsp        ; ret

:try_parse_comment_loop

    ; consume the current char, call next_char()
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the current char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; check if it's the end of a line or the end of the file
    cmpu ra r0 '0A  ; (line feed)
    jz ra &try_parse_comment_done
    cmpu ra r0 '0D  ; (carriage return)
    jz ra &try_parse_comment_done
    cmpu ra r0 'FF  ; (end-of-file)
    jz ra &try_parse_comment_done

    ; keep looping
    jz '00 &try_parse_comment_loop  ; jz 0 &try_parse_comment_loop

:try_parse_comment_done

    ;return true
    add r0 '00 '01
    ldw rip '00 rsp        ; ret



;===========================================================
; void consume_whitespace_and_comments(void);
;===========================================================
; Consumes all whitespace and comments.
;===========================================================

=consume_whitespace_and_comments
    ; no stack frame

    ; call try_parse_whitespace()
    ims ra <try_parse_whitespace
    ims ra >try_parse_whitespace
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if it returned true, start over
    jz r0 &consume_whitespace_and_comments_not_whitespace
    jz '00 &consume_whitespace_and_comments

:consume_whitespace_and_comments_not_whitespace

    ; call try_parse_comment()
    ims ra <try_parse_comment
    ims ra >try_parse_comment
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if it returned true, start over
    jz r0 &consume_whitespace_and_comments_not_comment
    jz '00 &consume_whitespace_and_comments

:consume_whitespace_and_comments_not_comment

    ; return
    ldw rip '00 rsp        ; ret



; ==========================================================
; int convert_keyword(char* table);
; ==========================================================
; Converts the given identifier to its bytecode value using the given
; conversion table (either `registers` or `syscalls`.)
;
; Returns -1 if not found.
;
; vars:
; - table_ptr: rfp-4
; ==========================================================

=convert_keyword

    ; set up a stack frame
    sub rsp rsp '04     ; push rfp       ; enter
    stw rfp '00 rsp     ; ...
    add rfp rsp '00     ; mov rfp rsp

    ; push the table pointer
    sub rsp rsp '04
    stw r0 rsp '00

:convert_keyword_loop

    ; get keyword in table
    ldw r0 rfp 'FC

    ; call strlen() on it
    ims ra <strlen
    ims ra >strlen
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if the keyword is empty, no match
    jz r0 &convert_keyword_not_found

    ; bump the table ptr to the next entry
    ldw r1 rfp 'FC
    add r0 r1 r0
    add r0 r0 '02
    stw r0 rfp 'FC

    ; get identifier
    ims ra <identifier
    ims ra >identifier
    add r0 rpp ra

    ; check if identifier matches table entry, call strcmp()
    ims ra <strcmp
    ims ra >strcmp
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address
    jz r0 &convert_keyword_found

    ; no match; loop
    jz '00 &convert_keyword_loop

:convert_keyword_not_found
    ; return -1
    add r0 '00 'FF
    jz '00 &convert_keyword_ret

:convert_keyword_found
    ; return the byte matching the entry
    ldw r0 rfp 'FC
    ldb r0 r0 'FF

:convert_keyword_ret
    add rsp rfp '00     ; mov rsp rfp   ; leave
    ldw rfp '00 rsp     ; pop rfp       ; ^^^
    add rsp rsp '04     ; ^^^           ; ^^^
    ldw rip '00 rsp     ; ret



; ==========================================================
; uint8_t parse_register(void)
; ==========================================================
; Parses a register and returns its byte representation (in the range
; 0x80-0x8F).
;
; The register can be given as a name (e.g. r0, rsp) or as a quoted byte.
;
; If the next token is not a register, the assembler aborts with an error.
; ==========================================================

=parse_register

    ; call consume_whitespace_and_comments()
    ims ra <consume_whitespace_and_comments
    ims ra >consume_whitespace_and_comments
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; call try_parse_quoted_byte(rsp)
    ims ra <try_parse_quoted_byte
    ims ra >try_parse_quoted_byte
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if successful, return it
    ; (we don't bother to check that it's actually a register, but the final
    ; stage assembler does.)
    cmpu ra r0 'FF
    jz ra &parse_register_not_quoted_byte
    ldw rip '00 rsp        ; ret
:parse_register_not_quoted_byte

    ; check for a register name, call try_parse_identifier()
    ims ra <try_parse_identifier
    ims ra >try_parse_identifier
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address
    jz r0 &parse_register_fail

    ; identifier found. call convert_keyword(registers)
    ims ra <registers
    ims ra >registers
    add r0 rpp ra
    ims ra <convert_keyword
    ims ra >convert_keyword
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if successful, return it
    cmpu ra r0 'FF
    jz ra &parse_register_fail
    ldw rip '00 rsp        ; ret

    ; not a register. fatal error
:parse_register_fail
    ims r0 <error_expected_register
    ims r0 >error_expected_register
    add r0 rpp r0
    ims ra <fatal
    ims ra >fatal
    add rip rpp ra



; ==========================================================
; bool try_parse_number(int* out_number);
; ==========================================================
; Tries to parse a decimal or hexadecimal number, storing it in the given
; pointer.
;
; Returns true if successful, false otherwise.
; ==========================================================

=try_parse_number

    ; TODO: this out_number pointer is kind of annoying. should have just
    ; returned the number in r1. might be worth fixing someday.

    ; stash out_number
    sub rsp rsp '04  ; push r0
    stw r0 rsp '00   ; ^^^

    ; get the current char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; check for a minus sign
    cmpu ra r0 "-"
    jz ra &try_parse_number_negative

    ; non-negative; push false
    sub rsp rsp '04
    stw '00 rsp '00
    jz '00 &try_parse_number_sign_done

:try_parse_number_negative

    ; negative; push true
    sub rsp rsp '04
    stw '01 rsp '00

    ; consume the minus sign, call next_char()
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

:try_parse_number_sign_done

    ; get the current char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; check if it's a zero
    cmpu ra r0 "0"
    jz ra &try_parse_number_leading_zero
    jz '00 &try_parse_number_not_zero

:try_parse_number_leading_zero

    ; consume the zero, call next_char()
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the current char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; check for x or X. if so it's hexadecimal.
    cmpu ra r0 "x"
    jz ra &try_parse_number_hexadecimal
    cmpu ra r0 "X"
    jz ra &try_parse_number_hexadecimal

    ; it's not hex. check if it's a digit
    ims ra <isdigit
    ims ra >isdigit
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if it's a digit, we don't support octal, it's a fatal error
    jz r0 &try_parse_number_return_zero
    jz '00 &try_parse_number_error

:try_parse_number_return_zero
    ; it's just zero. discard the minus sign, we're done.
    add rsp rsp '04   ; popd (negative)
    add r0 '00 '00    ; zero r0
    jz '00 &try_number_parsed_done

:try_parse_number_hexadecimal

    ; consume the x, call next_char()
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; call parse_hexadecimal()
    ims ra <parse_hexadecimal
    ims ra >parse_hexadecimal
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address
    jz '00 &try_parse_number_parsed

:try_parse_number_not_zero

    ; not zero. check for a digit, call isdigit()
    ims ra <isdigit
    ims ra >isdigit
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if it's not a digit, we don't have a number
    jz r0 &try_parse_number_not_number

    ; it's a non-zero digit. call parse_decimal()
    ims ra <parse_decimal
    ims ra >parse_decimal
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

:try_parse_number_parsed

    ; check if we consumed a minus sign. if so negate the number.
    ldw r1 rsp '00    ; pop r1
    add rsp rsp '04
    jz r1 &try_number_parsed_done
    sub r0 '00 r0

:try_number_parsed_done

    ; store it in out_pointer
    ldw r1 rsp '00    ; pop r1
    add rsp rsp '04
    stw r0 r1 '00

    ; return true
    add r0 '00 '01
    ldw rip '00 rsp    ; ret

:try_parse_number_not_number
    ; check if we consumed a minus sign. if we did but there are no digits,
    ; it's a fatal error.
    ldw r1 rsp '00    ; pop r1
    add rsp rsp '04
    jz r1 &try_parse_number_return_false
    jz '00 &try_parse_number_error

    ; otherwise we can return false.
:try_parse_number_return_false
    add rsp rsp '04    ; popd (out_number)
    add r0 '00 '00     ; zero r0
    ldw rip '00 rsp    ; ret

:try_parse_number_error
    ims r0 <error_number
    ims r0 >error_number
    add r0 rpp r0
    ims ra <fatal
    ims ra >fatal
    add rip rpp ra



; ==========================================================
; bool try_parse_and_emit_number(void);
; ==========================================================
; Tries to parse a number and, if found, emits it as four bytes.
; ==========================================================

=try_parse_and_emit_number

    ; make room for a number
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

    ; if we didn't get a number, return false
    jz r0 &try_parse_number_not_found

    ; emit the number, call emit_bytes_as_hex(rsp, 4)
    add r0 rsp '00
    add r1 '00 '04
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; return true
    add rsp rsp '04    ; popn
    add r0 '01 '00     ; mov r0 1
    ldw rip '00 rsp    ; ret

:try_parse_number_not_found
    ; return false
    add rsp rsp '04    ; popn
    add r0 '00 '00     ; zero r0
    ldw rip '00 rsp    ; ret



; ==========================================================
; int parse_decimal(void)
; ==========================================================
; Parses a decimal number (that is already known to start with a digit) and
; returns it.
;
; The number must start with a digit 1-9. (A leading minus sign for negative is
; parsed separately in try_parse_number(). Call that instead.)
; ==========================================================

=parse_decimal

    ; push space for the total starting at zero
    sub rsp rsp '04   ; push 0
    stw '00 rsp '00

:parse_decimal_loop

    ; get the current char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; add its value to the current total
    sub r0 r0 "0"
    ldw r1 rsp '00
    mul r1 r1 '0A   ; times ten
    add r1 r1 r0
    stw r1 rsp '00

    ; consume it, call next_char()
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the current char again
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; check if it's a digit
    ims ra <isdigit
    ims ra >isdigit
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; loop only if we have another digit
    jz r0 &parse_decimal_done
    jz '00 &parse_decimal_loop

:parse_decimal_done
    ; return the total
    ldw r0 rsp '00   ; pop r0
    add rsp rsp '04
    ldw rip '00 rsp  ; ret




; ==========================================================
; int parse_hexadecimal(void)
; ==========================================================
; Parses a hexadecimal number and returns it.
;
; The number must start with 0x or 0X. (A leading minus sign for negative is
; parsed separately in try_parse_number(). Call that instead.)
; ==========================================================

=parse_hexadecimal

    ; push space for the total starting at zero
    sub rsp rsp '04   ; push 0
    stw '00 rsp '00

:parse_hexadecimal_loop

    ; get the current char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; convert from hex, call hex_value()
    ims ra <hex_value
    ims ra >hex_value
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if not a hex digit, break
    cmpu ra r0 'FF
    jz ra &parse_hexadecimal_done

    ; add its value to the current total
    ldw r1 rsp '00
    ror r1 r1 '1C    ; rotate left by 4, i.e. shift up
    add r1 r1 r0
    stw r1 rsp '00

    ; consume it, call next_char()
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; loop
    jz '00 &parse_hexadecimal_loop

:parse_hexadecimal_done
    ; return the total
    ldw r0 rsp '00   ; pop r0
    add rsp rsp '04
    ldw rip '00 rsp  ; ret



; ==========================================================
; uint8_t parse_mix(void)
; ==========================================================
; Parses a mix-type argument, returning its single byte representation.
;
; A mix-type argument can be:
; - A register name
; - A decimal number in the range [-112,127]
; - A quoted byte
; - A single-character string
; ==========================================================

=parse_mix

    ; call consume_whitespace_and_comments()
    ims ra <consume_whitespace_and_comments
    ims ra >consume_whitespace_and_comments
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; call try_parse_quoted_byte(rsp)
    ims ra <try_parse_quoted_byte
    ims ra >try_parse_quoted_byte
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if successful, return it
    cmpu ra r0 'FF
    jz ra &parse_mix_not_quoted_byte
    ldw rip '00 rsp        ; ret
:parse_mix_not_quoted_byte

    ; check for a register name, call try_parse_identifier()
    ims ra <try_parse_identifier
    ims ra >try_parse_identifier
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address
    jz r0 &parse_mix_not_register_name

    ; identifier found. call convert_keyword(registers)
    ims ra <registers
    ims ra >registers
    add r0 rpp ra
    ims ra <convert_keyword
    ims ra >convert_keyword
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if it failed to convert, fatal error
    cmpu ra r0 'FF
    jz ra &parse_mix_fail

    ; return the register byte
    ldw rip '00 rsp        ; ret
:parse_mix_not_register_name

    ; check for a character, call try_parse_character()
    ims ra <try_parse_character
    ims ra >try_parse_character
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; see if we got a character
    cmpu ra r0 'FF
    jz ra &parse_mix_not_character

    ; character found, return it
    ldw rip '00 rsp        ; ret
:parse_mix_not_character

    ; make room for a number
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

    ; if no number, it's a fatal error
    jz r0 &parse_mix_fail

    ; return the number
    ldw r0 rsp '00
    add rsp rsp '04
    ldw rip '00 rsp   ; ret

:parse_mix_fail
    ims r0 <error_expected_mix
    ims r0 >error_expected_mix
    add r0 rpp r0
    ims ra <fatal
    ims ra >fatal
    add rip rpp ra



; ==========================================================
; void parse_register_mix_mix(uint8_t[3])
; ==========================================================
; Parses a register and two mix-type arguments, storing their byte
; representations in the given array.
;
; Many opcodes take arguments in this form so this helper is available to
; easily parse them.
; ==========================================================

=parse_register_mix_mix

    ; push the array
    sub rsp rsp '04
    stw r0 rsp '00

    ; call parse_register()
    ims ra <parse_register
    ims ra >parse_register
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; store it
    ldw r1 rsp '00
    stb r0 r1 '00

    ; call parse_mix()
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; store it
    ldw r1 rsp '00
    stb r0 r1 '01

    ; call parse_mix()
    ims ra <parse_mix
    ims ra >parse_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; store it
    ldw r1 rsp '00
    stb r0 r1 '02

    ; done
    add rsp rsp '04
    ldw rip '00 rsp        ; ret



; ==========================================================
; bool try_parse_and_emit_string(void);
; ==========================================================

=try_parse_and_emit_string

    ; get the current char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; check if it's the start of a string
    cmpu ra r0 '22    ; '"'
    jz ra &try_parse_and_emit_string_loop

    ; not a string, return false
    add r0 '00 '00
    ldw rip '00 rsp        ; ret

:try_parse_and_emit_string_loop

    ; consume the current char, call next_char()
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the current char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; check for end-of-file or backslash. if so, fatal error
    cmpu ra r0 'FF
    jz ra &try_parse_and_emit_string_error
    cmpu ra r0 '5C  ; '\\'
    jz ra &try_parse_and_emit_string_error

    ; check for a double quote. if so, it's the end of the string
    cmpu ra r0 '22    ; '"'
    jz ra &try_parse_and_emit_string_done

    ; check if it's a printable character, call isprint()
    ims ra <isprint
    ims ra >isprint
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if it's not printable, fatal error
    jz r0 &try_parse_and_emit_string_error

    ; get the current char back into r0
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; emit it in hexadecimal, call emit_byte_as_hex
    ims ra <emit_byte_as_hex
    ims ra >emit_byte_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; keep looping
    jz '00 &try_parse_and_emit_string_loop

:try_parse_and_emit_string_done

    ; consume the closing quote, call next_char()
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ;return true
    add r0 '00 '01
    ldw rip '00 rsp        ; ret

:try_parse_and_emit_string_error
    ims r0 <error_invalid_string
    ims r0 >error_invalid_string
    add r0 rpp r0
    ims ra <fatal
    ims ra >fatal
    add rip rpp ra



; ==========================================================
; int try_parse_character(void);
; ==========================================================
; Tries to parse a single-character string, returning the character if found
; and -1 otherwise.
;
; If a string is found but it does not have exactly one character, a fatal
; error is raised.
; ==========================================================

=try_parse_character

    ; get the current char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; check if it's the start of a string
    cmpu ra r0 '22    ; '"'
    jz ra &try_parse_character_found

    ; not a string, return -1
    add r0 '00 'FF
    ldw rip '00 rsp        ; ret

:try_parse_character_found

    ; consume the opening double-quote, call next_char()
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the current char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; check for end-of-file, double-quote or backslash. if so, fatal error
    cmpu ra r0 'FF
    jz ra &try_parse_character_error
    cmpu ra r0 '22  ; '"'
    jz ra &try_parse_character_error
    cmpu ra r0 '5C  ; '\\'
    jz ra &try_parse_character_error

    ; stash it
    sub rsp rsp '04
    stw r0 rsp '00

    ; make sure it's a printable character, call isprint()
    ims ra <isprint
    ims ra >isprint
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if it's not printable, fatal error
    jz r0 &try_parse_character_error

    ; consume the character, call next_char()
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; make sure the string ends in a closing double-quote
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra
    cmpu ra r0 '22   ; '"'
    jz ra &try_parse_character_ok
    jz '00 &try_parse_character_error

:try_parse_character_ok

    ; consume the closing double-quote
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; done
    ldw r0 rsp '00    ; pop r0
    add rsp rsp '04
    ldw rip '00 rsp   ; ret

:try_parse_character_error
    ims r0 <error_string_arg_must_be_single_char
    ims r0 >error_string_arg_must_be_single_char
    add r0 rpp r0
    ims ra <fatal
    ims ra >fatal
    add rip rpp ra



; ==========================================================
; char try_parse_linker(void);
; ==========================================================
; Parses a symbol or label declaration or invocation (i.e. a linker directive.)
;
; The label or symbol name is placed in `identifier` and the control character
; is returned.
;
; If the current token is not a linker directive, 0 is returned. If the linker
; directive is invalid, a fatal error is raised.
; ==========================================================

=try_parse_linker

    ; get the current char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; check if it's the start of linker directive
    cmpu ra r0 ":"
    jz ra &try_parse_linker_found
    cmpu ra r0 "="
    jz ra &try_parse_linker_found
    cmpu ra r0 "@"
    jz ra &try_parse_linker_found
    cmpu ra r0 "^"
    jz ra &try_parse_linker_found
    cmpu ra r0 "<"
    jz ra &try_parse_linker_found
    cmpu ra r0 ">"
    jz ra &try_parse_linker_found
    cmpu ra r0 "&"
    jz ra &try_parse_linker_found

    ; not a linker directive, return 0
    add r0 '00 '00
    ldw rip '00 rsp        ; ret

:try_parse_linker_found

    ; stash the control character for later
    sub rsp rsp '04    ; push r0
    stw r0 rsp '00     ; ^^^

    ; consume it, call next_char()
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; parse the label or symbol name, call try_parse_identifier()
    ims ra <try_parse_identifier
    ims ra >try_parse_identifier
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; make sure it worked
    jz r0 &try_parse_linker_fail

    ; return the control character
    ldw r0 rsp '00     ; pop r0
    add rsp rsp '04    ; ^^^
    ldw rip '00 rsp        ; ret

:try_parse_linker_fail
    ims r0 <error_linker_control_must_have_identifier
    ims r0 >error_linker_control_must_have_identifier
    add r0 rpp r0
    ims ra <fatal
    ims ra >fatal
    add rip rpp ra



; ==========================================================
; bool try_parse_and_emit_linker(void);
; ==========================================================
; Parses and emits a symbol or label declaration or invocation (i.e. a linker
; directive.)
;
; Returns true if a linker directive was parsed and emitted, false otherwise.
;
; The linker directive is emitted verbatim to the output (with a trailing space
; to prevent hex bytes from being concatenated with it.)
;
; This stage of the assembler does not support linker flags (i.e. weak, zero,
; constructor/destructor.) The final stage assembler supports them.
; ==========================================================

=try_parse_and_emit_linker

    ; call try_parse_linker()
    ims ra <try_parse_linker
    ims ra >try_parse_linker
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if false, return false
    jz r0 &try_parse_and_emit_linker_false

    ; call emit_linker()
    ; (control character is already in r0)
    ims ra <emit_linker
    ims ra >emit_linker
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; return true
    add r0 '00 '01
    ldw rip '00 rsp        ; ret

:try_parse_and_emit_linker_false
    ; not a linker directive, return false
    ; (zero is already in r0)
    ldw rip '00 rsp        ; ret



;==========================================
; int hex_value(char c);
;==========================================
; Returns the hex value of the given char, or -1 if it's not hex.
;==========================================

=hex_value
    ; don't bother to set up a stack frame

    ; check if it's a number 0-9
    cmpu ra r0 "0"
    cmpu ra ra 'FF
    jz ra &hex_value_not_number
    cmpu ra r0 "9"
    cmpu ra ra '01
    jz ra &hex_value_not_number

    ; it's 0-9
    sub r0 r0 "0"
    ldw rip '00 rsp        ; ret
:hex_value_not_number

    ; check if it's uppercase A-F
    cmpu ra r0 "A"
    cmpu ra ra 'FF
    jz ra &hex_value_not_uppercase
    cmpu ra r0 "F"
    cmpu ra ra '01
    jz ra &hex_value_not_uppercase

    ; it's A-F
    sub r0 r0 '37   ; ('A' == 0x41, 'A'-10 == 0x37)
    ldw rip '00 rsp        ; ret
:hex_value_not_uppercase

    ; check if it's lowercase a-f
    cmpu ra r0 "a"
    cmpu ra ra 'FF
    jz ra &hex_value_not_lowercase
    cmpu ra r0 "f"
    cmpu ra ra '01
    jz ra &hex_value_not_lowercase

    ; it's a-f
    sub r0 r0 '57   ; ('a' == 0x61, 'a'-10 == 0x57)
    ldw rip '00 rsp        ; ret
:hex_value_not_lowercase

    ; it's not hex.
    add r0 '00 'FF
    ldw rip '00 rsp        ; ret



; ==========================================================
; int try_parse_quoted_byte(void);
; ==========================================================
; Tries to parse a quoted byte.
;
; Returns the quoted byte (unsigned) if successful, or -1 if the current token
; is not a quoted byte.
;
; This is called from various places. At the top-level parsing we can have
; quoted bytes, but there can also be quoted bytes as arguments to most
; opcodes.
;
; params:
; - byte: rfp-4
; ==========================================================

=try_parse_quoted_byte
    ; set up a stack frame
    sub rsp rsp '04     ; push rfp
    stw rfp '00 rsp     ; ...
    add rfp rsp '00     ; mov rfp rsp

    ; make space for a quoted byte
    sub rsp rsp '04     ; push 0
    stw '00 rfp 'FC     ; ^^^

    ; get the current char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; check if it's a single quote
    cmpu ra r0 "'"
    jz ra &try_parse_quoted_byte_found

    ; not a quote, return -1
    add r0 '00 'FF
    add rsp rfp '00     ; mov rsp rfp   ; leave
    ldw rfp '00 rsp     ; pop rfp       ; ^^^
    add rsp rsp '04     ; ^^^           ; ^^^
    ldw rip '00 rsp     ; ret

:try_parse_quoted_byte_found

    ; consume the quote, call next_char()
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; convert from hex, call hex_value
    ims ra <hex_value
    ims ra >hex_value
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; make sure it's hex
    cmpu ra r0 'FF
    jz ra &try_parse_quoted_byte_error

    ; shift it up and store it
    ror r0 r0 '1C    ; rol r0 r0 4
    stw r0 rfp 'FC

    ; consume the hex char, call next_char()
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; convert from hex, call hex_value
    ims ra <hex_value
    ims ra >hex_value
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; make sure it's hex
    cmpu ra r0 'FF
    jz ra &try_parse_quoted_byte_error

    ; mix it in
    ldw r1 rfp 'FC
    add r0 r0 r1
    stw r0 rfp 'FC

    ; consume the hex char, call next_char()
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; return the byte
    ldw r0 rfp 'FC
    add rsp rfp '00     ; mov rsp rfp   ; leave
    ldw rfp '00 rsp     ; pop rfp       ; ^^^
    add rsp rsp '04     ; ^^^           ; ^^^
    ldw rip '00 rsp     ; ret

:try_parse_quoted_byte_error
    ims r0 <error_invalid_character
    ims r0 >error_invalid_character
    add r0 rpp r0
    ims ra <fatal
    ims ra >fatal
    add rip rpp ra



; ==========================================================
; bool try_parse_and_emit_quoted_byte(void);
; ==========================================================
; Tries to parse a quoted byte, and if found, emits it.
;
; Returns true if found (and emitted), false otherwise.
; ==========================================================

=try_parse_and_emit_quoted_byte
    ; no stack frame

    ; call try_parse_quoted_byte(rsp)
    ims ra <try_parse_quoted_byte
    ims ra >try_parse_quoted_byte
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if not found, return false
    cmpu ra r0 'FF
    jz ra &parse_and_emit_quoted_byte_false

    ; found. emit it
    ims ra <emit_byte_as_hex
    ims ra >emit_byte_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; return true
    add r0 '00 '01
    ldw rip '00 rsp     ; ret

:parse_and_emit_quoted_byte_false
    ; return false
    add r0 '00 '00      ; zero r0
    ldw rip '00 rsp     ; ret



; ==========================================================
; bool is_identifier_char(char c, bool first_char);
; ==========================================================
; Returns true if the given character is valid in an identifier (i.e. a label);
; false otherwise.
;
; If first_char is true, it must be valid for the first character of an
; identifier (i.e. not a number.)
; ==========================================================

=is_identifier_char

    ; we accept $ in identifiers as an extension.
    cmpu ra r0 "$"
    jz ra &is_identifier_char_true
    cmpu ra r0 "_"
    jz ra &is_identifier_char_true

    ; a number is allowed if first_char is false. we therefore call isalpha()
    ; if first_char is true and isalnum() if first_char is false.
    jz r1 &is_identifier_char_isalnum
    ims ra <isalpha
    ims ra >isalpha
    jz '00 &is_identifier_char_call
:is_identifier_char_isalnum
    ims ra <isalnum
    ims ra >isalnum
:is_identifier_char_call
    add rip rpp ra    ; tail-call

    ; for $ and _, return true
:is_identifier_char_true
    add r0 '00 '01
    ldw rip '00 rsp     ; ret



; ==========================================================
; bool try_parse_identifier(void);
; ==========================================================
; Tries to parse an identifier, returning true if found and false otherwise.
;
; The identifier is read into `identifier` as a null-terminated string.
;
; vars:
; - length/iterator: r1, rfp-4
; ==========================================================

=try_parse_identifier

    ; get the current char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; check if it's a first identifier char
    add r1 '00 '01
    ims ra <is_identifier_char
    ims ra >is_identifier_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    jz r0 &try_parse_identifier_false
    jz '00 &try_parse_identifier_found

:try_parse_identifier_false

    ; not an identifier, return false
    add r0 '00 '00
    ldw rip '00 rsp        ; ret

:try_parse_identifier_found

    ; we've found the start of an identifier.

    ; set up a stack frame
    sub rsp rsp '04       ; push rfp       ; enter
    stw rfp '00 rsp     ; ...
    add rfp rsp '00     ; mov rfp rsp

    ; push a length iterator to the stack
    sub rsp rsp '04     ; push '00
    stw '00 '00 rsp   ; ^^^

:try_parse_identifier_read_loop

    ; get the current char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; get the length into r1
    ldw r1 rfp 'FC

    ; check that the length isn't too long
    cmpu ra r1 '7F    ; 127
    jz ra &try_parse_identifier_too_long

    ; store the current char into the string
    ims ra <identifier
    ims ra >identifier
    add ra rpp ra
    stb r0 ra r1

    ; increment the length
    add r1 r1 '01
    stw r1 rfp 'FC

    ; consume the character, call next_char()
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the current char
    ims ra <current_char
    ims ra >current_char
    ldw r0 rpp ra

    ; check if it's a (not first) identifier char
    add r1 '00 '00
    ims ra <is_identifier_char
    ims ra >is_identifier_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; not a letter or digit, we're done reading
    jz r0 &try_parse_identifier_read_done

    ; keep reading
    jz '00 &try_parse_identifier_read_loop

:try_parse_identifier_read_done

    ; append a null-terminator onto the string
    ldw r1 rfp 'FC
    ims ra <identifier
    ims ra >identifier
    add ra rpp ra
    stb '00 ra r1

    ; return true
    add rsp rfp '00     ; mov rsp rfp   ; leave
    ldw rfp '00 rsp     ; pop rfp
    add rsp rsp '04     ; ...
    add r0 '00 '01  ; return true
    ldw rip '00 rsp     ; ret

:try_parse_identifier_too_long
    ims r0 <error_identifier_too_long
    ims r0 >error_identifier_too_long
    add r0 rpp r0
    ims ra <fatal
    ims ra >fatal
    add rip rpp ra



;===========================================================
; void parse(void);
;===========================================================
; Parses all input.
;
; When an identifier is encountered here, it must be an opcode. It is
; dispatched to the opcode's handler which parses the entire instruction,
; verifies it for correctness, and emits the appropriate primitive bytecode
; instructions.
;
; Otherwise, quoted bytes, strings and labels are emitted directly to the
; output.
;===========================================================

=parse
    ; no stack frame

:parse_loop

    ; call consume_whitespace_and_comments()
    ims ra <consume_whitespace_and_comments
    ims ra >consume_whitespace_and_comments
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; check for EOF. if so we're done
    ims ra <current_char
    ims ra >current_char
    ldw ra rpp ra
    cmpu ra ra 'FF
    jz ra &parse_done

    ; try to parse an identifier
    ims ra <try_parse_identifier
    ims ra >try_parse_identifier
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address
    jz r0 &parse_not_identifier

    ; an identifier was found; it must be an opcode. call the opcode
    ; dispatch handler.
    ims ra <opcodes_dispatch
    ims ra >opcodes_dispatch
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address
    jz '00 &parse_loop
:parse_not_identifier

    ; check for a quoted byte
    ims ra <try_parse_and_emit_quoted_byte
    ims ra >try_parse_and_emit_quoted_byte
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address
    jz r0 &parse_not_quoted_byte
    jz '00 &parse_loop
:parse_not_quoted_byte

    ; check for a string
    ims ra <try_parse_and_emit_string
    ims ra >try_parse_and_emit_string
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address
    jz r0 &parse_not_string
    jz '00 &parse_loop
:parse_not_string

    ; check for a linker directive
    ims ra <try_parse_and_emit_linker
    ims ra >try_parse_and_emit_linker
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address
    jz r0 &parse_not_linker
    jz '00 &parse_loop
:parse_not_linker

    ; check for a number
    ims ra <try_parse_and_emit_number
    ims ra >try_parse_and_emit_number
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address
    jz r0 &parse_not_number
    jz '00 &parse_loop
:parse_not_number

    ; fatal, unrecognized character
    ims r0 <error_unrecognized_character
    ims r0 >error_unrecognized_character
    add r0 rpp r0
    ims ra <fatal
    ims ra >fatal
    add rip rpp ra

:parse_done
    ldw rip '00 rsp        ; ret
