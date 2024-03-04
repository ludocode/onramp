; The MIT License (MIT)
;
; Copyright (c) 2024 Fraser Heavy Software
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



; ==========================================================
; void emit_init(void);
; ==========================================================

=emit_init
    ret



; ==========================================================
; void emit_destroy(void);
; ==========================================================

=emit_destroy
    ret



; ==========================================================
; void emit_char(char c);
; ==========================================================

=emit_char
    imw r1 ^output_file
    ldw r1 rpp r1
    jmp ^fputc



; ==========================================================
; void emit_string(void);
; ==========================================================

=emit_string
    imw r1 ^output_file
    ldw r1 rpp r1
    jmp ^fputs



; ==========================================================
; void emit_label(char type, const char* label_name);
; ==========================================================

=emit_label
    push r1

    ; emit type
    call ^emit_char
    pop r0

    ; emit name
    call ^emit_string

    ; emit a space
    mov r0 " "
    jmp ^emit_char



; ==========================================================
; void emit_prefixed_linker_string(char type, const char* prefix, const char* name);
; ==========================================================

=emit_prefixed_linker_string
    push r2
    push r1

    ; emit type
    call ^emit_char

    ; emit prefix
    pop r0
    call ^emit_string

    ; emit name
    pop r0
    call ^emit_string

    ; emit a space
    mov r0 " "
    jmp ^emit_char



; ==========================================================
; void emit_prefixed_linker_number(char type, const char* prefix, int number);
; ==========================================================

=emit_prefixed_linker_number
    push r2
    push r1

    ; emit type
    call ^emit_char

    ; emit prefix
    pop r0
    call ^emit_string

    ; emit number
    pop r0
    call ^emit_hex_number

    ; emit a space
    mov r0 " "
    jmp ^emit_char



; ==========================================================
; void emit_term(const char* str);
; ==========================================================

=emit_term

    ; emit the string
    call ^emit_string

    ; emit a space
    mov r0 " "
    jmp ^emit_char



; ==========================================================
; void emit_newline(void);
; ==========================================================

=emit_newline
    mov r0 '0A  ; \n
    jmp ^emit_char



; ==========================================================
; void emit_zero(void);
; ==========================================================

=emit_zero
    mov r0 "0"
    call ^emit_char
    mov r0 " "
    jmp ^emit_char



; ==========================================================
; void emit_hex_char(char c);
; ==========================================================
; Emits the given four bits as one hexadecimal character.
; ==========================================================

@emit_hex_char
    ; don't bother to set up a stack frame

    ; see if we need a letter or a number
    cmpu r1 r0 9
    jg r1 &emit_hex_bits_alpha

    ; number
    add r0 r0 "0"
    jmp &emit_hex_bits_done

    ; letter
:emit_hex_bits_alpha
    add r0 r0 '37  ; 'A' - 10

    ; tail-call emit_char
:emit_hex_bits_done
    jmp ^emit_char



; ==========================================================
; void emit_hex_number(int number);
; ==========================================================

=emit_hex_number
    push r0

    ; This could probably be simplified with a loop, but this is already pretty
    ; simple. It's just really verbose.

    ; check if we can skip the first char
    ldw r0 rsp 0
    imw r1 0xF0000000
    and r0 r0 r1
    jz r0 &emit_hex_number_1_skip

    ; write the first char
    shru r0 r0 28
    and r0 r0 0xF
    call ^emit_hex_char
:emit_hex_number_1_skip

    ; check if we can skip the second char
    ldw r0 rsp 0
    imw r1 0xFF000000
    and r0 r0 r1
    jz r0 &emit_hex_number_2_skip

    ; write the second char
    shru r0 r0 24
    and r0 r0 0xF
    call ^emit_hex_char
:emit_hex_number_2_skip

    ; check if we can skip the third char
    ldw r0 rsp 0
    imw r1 0xFFF00000
    and r0 r0 r1
    jz r0 &emit_hex_number_3_skip

    ; write the third char
    shru r0 r0 20
    and r0 r0 0xF
    call ^emit_hex_char
:emit_hex_number_3_skip

    ; check if we can skip the fourth char
    ldw r0 rsp 0
    imw r1 0xFFFF0000
    and r0 r0 r1
    jz r0 &emit_hex_number_4_skip

    ; write the fourth char
    shru r0 r0 16
    and r0 r0 0xF
    call ^emit_hex_char
:emit_hex_number_4_skip

    ; check if we can skip the fifth char
    ldw r0 rsp 0
    imw r1 0xFFFFF000
    and r0 r0 r1
    jz r0 &emit_hex_number_5_skip

    ; write the fifth char
    shru r0 r0 12
    and r0 r0 0xF
    call ^emit_hex_char
:emit_hex_number_5_skip

    ; check if we can skip the sixth char
    ldw r0 rsp 0
    imw r1 0xFFFFFF00
    and r0 r0 r1
    jz r0 &emit_hex_number_6_skip

    ; write the sixth char
    shru r0 r0 8
    and r0 r0 0xF
    call ^emit_hex_char
:emit_hex_number_6_skip

    ; check if we can skip the seventh char
    ldw r0 rsp 0
    imw r1 0xFFFFFFF0
    and r0 r0 r1
    jz r0 &emit_hex_number_7_skip

    ; write the seventh char
    shru r0 r0 4
    and r0 r0 0xF
    call ^emit_hex_char
:emit_hex_number_7_skip

    ; write the last char (unconditionally)
    pop r0
    and r0 r0 0xF
    jmp &emit_hex_char



; ==========================================================
; void emit_quoted_byte(uint8_t byte);
; ==========================================================

=emit_quoted_byte
    push r0

    ; emit a quote
    mov r0 "'"
    call ^emit_char

    ; emit the first char
    ldw r0 rsp 0
    shru r0 r0 4
    and r0 r0 0xF
    call ^emit_hex_char

    ; emit the last char
    pop r0
    and r0 r0 0xF
    jmp ^emit_hex_char



; ==========================================================
; void emit_int(int value);
; ==========================================================

=emit_int
    ; For now we always emit all ints as the full hexadecimal. This works
    ; regardless of whether the number is signed, but any negative numbers
    ; will be the full 8 characters.

    push r0

    ; emit "0x"
    imw r0 ^str_hex_prefix
    add r0 rpp r0
    call ^emit_string

    ; emit the number in hex
    pop r0
    call ^emit_hex_number

    mov r0 " "
    jmp ^emit_char



; ==========================================================
; bool is_string_char_valid_assembly(c)
; ==========================================================
; Returns true if the character is valid in an assembly string.
;
; All printable ASCII characters except backslash and double quote are valid.
; ==========================================================

=is_string_char_valid_assembly
    enter
    push r0

    mov r1 '22   ; 0x22 == '"'
    sub r1 r0 r1
    jz r1 &is_string_char_valid_assembly_false

    ldw r0 rfp -4
    mov r1 '5c   ; 0x5c == '\\'
    sub r1 r0 r1
    jz r1 &is_string_char_valid_assembly_false

    ldw r0 rfp -4
    leave
    jmp ^isprint

:is_string_char_valid_assembly_false
    zero r0
    leave
    ret



; ==========================================================
; void emit_character_literal(char c);
; ==========================================================

=emit_character_literal
    enter
    push r0

    call ^is_string_char_valid_assembly
    jz r0 &emit_character_literal_hex

    ; char is printable, send it in a string
    mov r0 '22   ; 0x22 == '"'
    call ^emit_char
    ldw r0 rfp -4
    call ^emit_char
    mov r0 '22   ; 0x22 == '"'
    call ^emit_char
    mov r0 " "
    call ^emit_char

    leave
    ret

:emit_character_literal_hex
    ; char is not printable, send it as quoted hex
    ldw r0 rfp -4
    call ^emit_quoted_byte

    leave
    ret



; ==========================================================
; void emit_string_literal(const char* str);
; ==========================================================
; vars:
; - str: rfp-4
; - open: rfp-8
; ==========================================================

=emit_string_literal
    enter
    push r0
    push 0

:emit_string_literal_loop

    ; check if we're done
    ldw r0 rfp -4
    ldb r0 r0 0
    jz r0 &emit_string_literal_break

    ; check if it's printable
    call ^is_string_char_valid_assembly

    ; if we're switching whether the string is open, emit a quote
    ldw r1 rfp -8
    sub r2 r0 r1
    jz r2 &emit_string_literal_no_switch
    stw r0 rfp -8
    mov r0 '22   ; 0x22 == '"'
    call ^emit_char
:emit_string_literal_no_switch

    ; check whether we emit the char quoted
    ldw r0 rfp -4
    ldb r0 r0 0
    ldw r1 rfp -8
    jz r1 &emit_string_literal_closed

    ; emit unquoted
    call ^emit_char
    jmp &emit_string_literal_next

:emit_string_literal_closed
    ; emit quoted
    call ^emit_quoted_byte

:emit_string_literal_next
    ; move to the next character
    ldw r0 rfp -4
    inc r0
    stw r0 rfp -4
    jmp &emit_string_literal_loop

:emit_string_literal_break

    ; if we're open, emit the closing quote
    ldw r1 rfp -8
    jz r1 &emit_string_literal_done
    mov r0 '22   ; 0x22 == '"'
    call ^emit_char

:emit_string_literal_done

    ; emit the null-terminator
    zero r0
    call ^emit_quoted_byte
    mov r0 " "
    call ^emit_char

    leave
    ret



; ==========================================================
; void emit_register(int register_number);
; ==========================================================
; Emits a numbered register (0-9).
; ==========================================================

=emit_register
    push r0

    mov r0 "r"
    call ^emit_char

    pop r0
    add r0 r0 "0"
    call ^emit_char

    mov r0 " "
    jmp ^emit_char



; ==========================================================
; void compile_global_divider(void);
; ==========================================================

=compile_global_divider
    ; emit extra newlines to space out globals
    call ^emit_newline
    jmp ^emit_newline
