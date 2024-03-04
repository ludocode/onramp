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



; This is the first stage Onramp compiler. It is written in (compound) Onramp
; Assembly and compiles Onramp Minimal C (omC).

; Most of this file is adapted from cpp/0-strip. It deals with command-line
; arguments, opening/closing files, and initializing the rest of the compiler's
; components.



@dash_o
    "-o" '00

@r
    "r" '00

@w
    "w" '00



; ==========================================================
; int main(int argc, char** argv);
; ==========================================================
; The entry point of the compiler.
;
; vars:
; - argv: rfp-4
; ==========================================================

=main
    ; preserve argv
    enter
    push r1

    ; make sure we have exactly three arguments (plus the program name)
    cmpu r0 r0 4
    jne r0 &main_usage

    ; check if "-o" is the first argument
    ldw r1 r1 4
    imw r0 ^dash_o
    add r0 r0 rpp
    call ^strcmp
    jnz r0 &main_not_first

    ; it is; output comes first
    ldw r9 rfp -4
    ldw r1 r9 8
    ldw r0 r9 12
    jmp &main_ok

:main_not_first

    ; check if "-o" is the second argument
    ldw r9 rfp -4
    ldw r1 r9 8
    imw r0 ^dash_o
    add r0 r0 rpp
    call ^strcmp
    jnz r0 &main_usage

    ; it is; output comes last
    ldw r9 rfp -4
    ldw r0 r9 4
    ldw r1 r9 12

:main_ok
    call ^open_files
    call ^run
    call ^close_files

    ; free the current filename
    zero r0
    call ^set_current_filename

    ; done
    zero r0
    leave
    ret

:main_usage
    imw r0 ^error_usage
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; void open_files(const char* input_filename, const char* output_filename);
; ==========================================================

=open_files
    enter

    ; preserve output filename
    push r1

    ; store the initial source filename for error printing
    push r0
    call ^set_current_filename
    pop r0

    ; open input file
    imw r1 ^r
    add r1 r1 rpp
    call ^fopen
    jz r0 &open_files_input_failed
    imw r1 ^input_file
    stw r0 rpp r1

    ; open output file
    pop r0
    imw r1 ^w
    add r1 r1 rpp
    call ^fopen
    jz r0 &open_files_output_failed
    imw r1 ^output_file
    stw r0 rpp r1

    ; prime the current char
    ; TODO
    ;call ^next_char

    ; done
    leave
    ret

:open_files_input_failed
    imw r0 ^error_input
    add r0 r0 rpp
    call ^fatal

:open_files_output_failed
    imw r0 ^error_output
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; void close_files(void);
; ==========================================================

=close_files

    ; close output file
    imw r0 ^output_file
    ldw r0 rpp r0
    call ^fclose

    ; close input file
    imw r0 ^input_file
    ldw r0 rpp r0
    call ^fclose

    ; free filename
    imw r1 ^current_filename
    ldw r0 rpp r1
    call ^free

    ret



; ==========================================================
; void run(void);
; ==========================================================

=run
    ; no stack frame

    call ^lexer_init
    call ^emit_init
    call ^type_init
    call ^globals_init
    call ^locals_init
    call ^compile_init
    call ^parse_init

    ;call ^lexer_dump_tokens
    call ^parse

    call ^parse_destroy
    call ^compile_destroy
    call ^locals_destroy
    call ^globals_destroy
    call ^type_destroy
    call ^emit_destroy
    call ^lexer_destroy

    ret
