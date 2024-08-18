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
; The compound assembler adds support for compound instructions with variable
; numbers of arguments. It also adds decimal numbers.
;
; This file contains the main parsing and output logic of the assembler. It
; reads opcode names, searches the opcode tables for matching opcodes, and
; dispatches to the appropriate opcode handlers. It also contains helpers for
; parsing strings, hex bytes, decimal numbers and labels.



; TODO remove ERROR prefix and trailing newline, don't use __fatal, libo now
; has fatal() and checks whether file/line info exists

=error_incorrect_arguments
    "ERROR: Incorrect arguments." '0A
    "Usage: <as> [input] -o [output]" '0A '00

=error_input_file
    "ERROR: Failed to read input file." '0A '00

=error_output_file
    "ERROR: Failed to write output file." '0A '00

=error_invalid_character
    "Invalid character." '0A '00

=error_unrecognized_character
    "Unrecognized character." '0A '00

=error_invalid_string
    "ERROR: Invalid string." '0A '00

=error_identifier_too_long
    "Identifier is too long." '00

=error_unrecognized_opcode
    "Unrecognized opcode." '00

=error_expected_register
    "Expected register." '00

=error_expected_mix
    "Expected a mix-type byte." '00

=error_string_arg_must_be_single_char
    "String argument to an opcode must be a single character." '00

=error_linker_control_must_have_identifier
    "A linker control character must be followed by a label or symbol name." '00

=error_linker_malformed
    "A linker directive is malformed." '00

=error_short_value
    "Expected a 16-bit invocation, a number, or two bytes." '00

=error_imw_value
    "The value for `imw` must be a 32-bit invocation, a number, or four bytes." '00

=error_number
    "A number is malformed." '00

=error_call
    "The argument for `call` must be a 32-bit invocation." '00

=error_jmp_linker
    "The linker directive for `jmp` must be a 16-bit relative or 32-bit absolute invocation." '00

=error_sys
    "Expected a syscall name and double zeroes."



; ==========================================================
; int main(int argc, char** argv);
; ==========================================================
; The entry point of the assembler. We parse the arguments to determine the
; input and output filenames, then hand over control to open_files.
;
; params:
; - argc: r0
; - argv: r1, rfp-4
; vars:
; - input filename index: r2
; - output filename index: r3
; ==========================================================

=main
    ; set up a stack frame (without pushing rfp because we won't return)
    add rfp rsp '00     ; mov rfp rsp

    ; check that we have exactly four arguments
    cmpu ra r0 '04
    jz ra &main_four_args
    jz '00 &main_incorrect_arguments
:main_four_args

    ; preserve r1 now
    sub rsp rsp '04    ; push r1
    stw r1 '00 rsp     ; ...

    ; push "-o" to the stack for comparison
    ims ra '00 '00
    ims ra "-" "o"
    sub rsp rsp '04
    stw ra '00 rsp

    ; check if -o is the first argument
    ldw r0 r1 '04       ; r0 = argv[1]
    add r1 '00 rsp      ; r1 = rsp ("-o")
        ; call strcmp()
        ims ra <strcmp
        ims ra >strcmp
        sub rsp rsp '04     ; push return address
        add rb rip '08      ; ...
        stw rb '00 rsp      ; ...
        add rip rpp ra    ; jump
        add rsp rsp '04     ; pop return address
    ldw r1 rfp 'FC     ; restore r1
    jz r0 &main_first_arg
    jz '00 &main_check_second_arg

    ; -o is the first argument.
:main_first_arg
    add r2 '00 '03
    add r3 '00 '02
    jz '00 &main_parsed_arguments

    ; check if -o is the second argument
:main_check_second_arg
    ldw r0 r1 '08       ; r0 = argv[2]
    add r1 '00 rsp      ; r1 = rsp ("-o")
        ; call strcmp
        ims ra <strcmp
        ims ra >strcmp
        sub rsp rsp '04     ; push return address
        add rb rip '08      ; ...
        stw rb '00 rsp      ; ...
        add rip rpp ra      ; jump
        add rsp rsp '04     ; pop return address
    ldw r1 rfp 'FC     ; restore r1
    jz r0 &main_second_arg
    jz '00 &main_incorrect_arguments

    ; -o is the second argument.
:main_second_arg
    add r2 '00 '01
    add r3 '00 '03
    jz '00 &main_parsed_arguments

:main_incorrect_arguments
    ims r0 <error_incorrect_arguments
    ims r0 >error_incorrect_arguments
    add r0 rpp r0
    ims ra <__fatal
    ims ra >__fatal
    add rip rpp ra

    ; argument parsing is done. get the filenames
:main_parsed_arguments
    mul r2 r2 '04
    ldw r0 r1 r2
    mul r3 r3 '04
    ldw r1 r1 r3

    ; clean up our stack and jump to open_files. (it never returns.)
    add rsp rfp '00     ; mov rsp rfp
    ims ra <open_files
    ims ra >open_files
    add rip rpp ra



; ==========================================================
; [[noreturn]] void open_files(const char* input_filename, const char* output_filename);
; ==========================================================
; params/vars:
; - input_filename, input_file: r0, rfp-4
; - output_filename, output_file: r1, rfp-8
; - "r": rfp-10
; - "w": rfp-12
; ==========================================================

=open_files
    ; set up a stack frame (without pushing rfp because we won't return)
    add rfp rsp '00     ; mov rfp rsp
    sub rsp rsp '0C

    ; preserve r0 and r1 on the stack
    stw r0 rfp 'FC   ; push r0
    stw r1 rfp 'F8   ; push r1

    ; call strdup(input_filename)
    ims ra <strdup
    ims ra >strdup
    sub rsp rsp '04     ; push return address
    add rb rip '08      ; ...
    stw rb '00 rsp      ; ...
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; store the result in current_filename
    ims ra <current_filename
    ims ra >current_filename
    stw r0 rpp ra

    ; push "r" and "w" to the stack (in one word)
    ims ra "r" '00
    ims ra "w" '00
    stw ra rfp 'F4   ; push "w\0r\0"

    ; open input: fopen(input_filename, "r")
    ldw r0 rfp 'FC      ; input_filename
    add r1 rfp 'F6      ; "r" (-10)
    ims ra <fopen
    ims ra >fopen
    sub rsp rsp '04     ; push return address
    add rb rip '08      ; ...
    stw rb '00 rsp      ; ...
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; make sure it's good
    cmpu ra r0 '00
    jz ra &open_files_input_error
    jz '00 &open_files_input_ok

    ; opening input failed, fatal error
:open_files_input_error
    ims r0 <error_input_file
    ims r0 >error_input_file
    add r0 rpp r0
    ims ra <__fatal
    ims ra >__fatal
    add rip rpp ra

    ; opening input succeeded, store it
:open_files_input_ok
    ims ra <input_file
    ims ra >input_file
    stw r0 rpp ra

    ; open output: fopen(output_filename, "w")
    ldw r0 rfp 'F8
    add r1 rfp 'F4
    ims ra <fopen
    ims ra >fopen
    sub rsp rsp '04     ; push return address
    add rb rip '08      ; ...
    stw rb '00 rsp      ; ...
    add rip rpp ra      ; jump
    add rsp rsp '04     ; pop return address
    stw r0 rfp 'F8

    ; make sure it's good
    cmpu ra r0 '00
    jz ra &open_files_output_error
    jz '00 &open_files_output_ok

    ; opening output failed, fatal error
:open_files_output_error
    ims r0 <error_output_file
    ims r0 >error_output_file
    add r0 rpp r0
    ims ra <__fatal
    ims ra >__fatal
    add rip rpp ra

    ; opening output succeeded, store it
:open_files_output_ok
    ims ra <output_file
    ims ra >output_file
    stw r0 rpp ra

    ; clean up our stack and jump to run. (it never returns.)
    add rsp rfp '00     ; mov rsp rfp
    ims ra <run
    ims ra >run
    add rip rpp ra



; ==========================================================
; FILE* input_file;
; ==========================================================

=input_file
    '00 '00 '00 '00



; ==========================================================
; FILE* output_file;
; ==========================================================

=output_file
    '00 '00 '00 '00



; ==========================================================
; uint8_t current_char;
; ==========================================================
; The current character being processed, i.e. the last character that was read.
;
; The current character is zero when the end of the file has been reached.
; ==========================================================

=current_char
    '00



; ==========================================================
; void next_char(void);
; ==========================================================
; Reads a character, or -1 if the end of the file has been reached, into
; current_char.
; ==========================================================

=next_char
    ; don't bother to set up a stack frame

    ; get the address of current_char into r0
    ims ra <current_char
    ims ra >current_char
    add r0 rpp ra

    ; element_size and count are both 1
    add r1 '00 '01
    add r2 '00 '01

    ; input file goes in r3
    ims ra <input_file
    ims ra >input_file
    ldw r3 rpp ra

    ; call fread
    ims ra <fread
    ims ra >fread
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if return value is 0, assume it's the end of the file
    jz r0 &next_char_eof
    jz '00 &next_char_done

    ; on end-of-file set current_char to -1
:next_char_eof
    ims ra <current_char
    ims ra >current_char
    stw 'FF rpp ra

:next_char_done
    ldw rip '00 rsp     ; ret



; ==========================================================
; [[noreturn]] void run(void);
; ==========================================================
; Parses the file with parse(), then cleans up and exits.
; ==========================================================

=run
    ; call opcodes_init()
    ims ra <opcodes_init
    ims ra >opcodes_init
    sub rsp rsp '04     ; push return address
    add rb rip '08      ; ...
    stw rb '00 rsp      ; ...
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; read the first character
    ims ra <next_char
    ims ra >next_char
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; call parse()
    ims ra <parse
    ims ra >parse
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the output file
    ims ra <output_file
    ims ra >output_file
    ldw r0 rpp ra

    ; call fclose()
    ims ra <fclose
    ims ra >fclose
    sub rsp rsp '04     ; push return address
    add rb rip '08      ; ...
    stw rb '00 rsp      ; ...
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the input file
    ims ra <input_file
    ims ra >input_file
    ldw r0 rpp ra

    ; call fclose()
    ims ra <fclose
    ims ra >fclose
    sub rsp rsp '04     ; push return address
    add rb rip '08      ; ...
    stw rb '00 rsp      ; ...
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get current_filename
    ims ra <current_filename
    ims ra >current_filename
    ldw r0 rpp ra

    ; call free()
    ims ra <free
    ims ra >free
    sub rsp rsp '04     ; push return address
    add rb rip '08      ; ...
    stw rb '00 rsp      ; ...
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; call opcodes_destroy()
    ims ra <opcodes_destroy
    ims ra >opcodes_destroy
    sub rsp rsp '04     ; push return address
    add rb rip '08      ; ...
    stw rb '00 rsp      ; ...
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; tail-call _Exit(0)
    add r0 '00 '00
    ims ra <_Exit
    ims ra >_Exit
    add rip rpp ra    ; jump
