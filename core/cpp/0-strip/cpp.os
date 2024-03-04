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



; This is the first stage preprocessor. It is written in compound assembly. It
; simply treats preprocessor directives as comments and strips all comments.
;
; TODO: This strips # even inside string and character literals. We should
; probably make it not do that. This is only used to build cpp/1 so for now we
; just avoid it there.



@dash_o
    "-o" '00

@r
    "r" '00

@w
    "w" '00

@error_usage
    "ERROR: Invalid arguments." '0A
    "Usage: <cpp> <input> -o <output>" '0A '00

@error_input
    "ERROR: Failed to read from input file." '0A '00

@error_output
    "ERROR: Failed to read from output file." '0A '00

@error_unclosed_comment
    "ERROR: Unclosed C-style comment." '0A '00

@input_file
    '00 '00 '00 '00

@output_file
    '00 '00 '00 '00

@current_char
    '00



; ==========================================================
; int main(int argc, char** argv);
; ==========================================================
; The entry point of the preprocessor.
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
    push r0
    call ^open_files
    pop r0
    call ^print_debug_line
    call ^run
    call ^close_files

    ; done
    zero r0
    leave
    ret

:main_usage
    imw r0 ^error_usage
    add r0 r0 rpp
    call ^__fatal



; ==========================================================
; void open_files(const char* input_filename, const char* output_filename);
; ==========================================================

=open_files
    ; preserve output filename
    enter
    push r1

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
    call ^next_char

    ; done
    leave
    ret

:open_files_input_failed
    imw r0 ^error_input
    add r0 r0 rpp
    call ^__fatal

:open_files_output_failed
    imw r0 ^error_output
    add r0 r0 rpp
    call ^__fatal



; ==========================================================
; void close_files(void);
; ==========================================================

=close_files
    imw r0 ^output_file
    ldw r0 rpp r0
    call ^fclose
    imw r0 ^input_file
    ldw r0 rpp r0
    call ^fclose
    ret



;==========================================
; void next_char(void);
;==========================================
; Reads a character, or -1 if the end of the file has been reached, into
; current_char.
;==========================================

=next_char
    ; don't bother to set up a stack frame

    ; TODO should just call fgetc()

    ; read one character from input_file into current_char
    imw r0 ^current_char
    add r0 r0 rpp
    mov r1 1
    mov r2 1
    imw r3 ^input_file
    ldw r3 r3 rpp
    call ^fread

    ; if return value is 0, assume it's the end of the file
    jz r0 &next_char_eof

    ; otherwise we're done
    ret

:next_char_eof

    ; on end-of-file set current_char to -1
    imw r0 ^current_char
    stw -1 rpp r0
    ret



; ==========================================================
; void print_debug_line(const char* input_filename);
; ==========================================================
; Prints a #line directive containing the source filename.
;
; We only print a single #line directive at the very start of the file. All
; line endings are forwarded from the input to the output so line numbers are
; automatically correct.
; ==========================================================

=print_debug_line
    ; TODO
    ret



; ==========================================================
; void run(void);
; ==========================================================

=run
    ; no stack frame

:run_loop

    ; check if current_char is -1
    imw r0 ^current_char
    ldw r0 rpp r0
    cmpu r0 r0 -1
    je r0 &run_eof

    ; check for a preprocessor directive
    call ^try_parse_preproc
    jnz r0 &run_loop

    ; check for a comment
    call ^try_parse_comment
    jnz r0 &run_loop

    ; otherwise forward the current char to the output
    imw r0 ^current_char
    ldb r0 rpp r0
    call ^emit_byte
    call ^next_char
    jmp &run_loop

:run_eof
    ret



; ==========================================================
; void emit_byte(char c);
; ==========================================================
; Writes the given byte to the output file.
; ==========================================================

=emit_byte
    ; no stack frame

    ; push the char to the stack to get a pointer to it it
    push r0

    ; call fwrite
    mov r0 rsp
    mov r1 1
    mov r2 1
    imw r3 ^output_file
    ldw r3 rpp r3
    call ^fwrite

    ; TODO check for error

    ; done
    popd
    ret



; ==========================================================
; void try_parse_preproc(void);
; ==========================================================

=try_parse_preproc

    ; check if current_char is #
    imw r0 ^current_char
    ldw r0 rpp r0
    cmpu r0 r0 "#"
    jne r0 &try_parse_preproc_not_found

:try_parse_preproc_loop

    ; consume the current char
    call ^next_char

    ; stop consuming when we reach a line feed, carriage return or eof
    imw r1 ^current_char
    ldw r1 rpp r1
    cmpu r0 r1 '0A   ; line feed
    je r0 &try_parse_preproc_done
    cmpu r0 r1 '0D   ; carriage return
    je r0 &try_parse_preproc_done
    cmpu r0 r1 -1    ; end-of-file
    je r0 &try_parse_preproc_done

    ; keep going
    jmp &try_parse_preproc_loop

:try_parse_preproc_done
    mov r0 1
    ret

:try_parse_preproc_not_found
    zero r0
    ret



; ==========================================================
; void try_parse_comment(void);
; ==========================================================
; Checks for a C-style or C++-style comment.
;
; If a comment is found, it forwards to parse_comment_c() or
; parse_comment_cxx().
; ==========================================================

=try_parse_comment

    ; check if current_char is /
    imw r1 ^current_char
    ldw r1 rpp r1
    cmpu r0 r1 "/"
    jne r0 &try_parse_comment_not_found

    ; consume it
    call ^next_char

    ; check if current char is * or / for a C-style or C++-style comment
    ; respectively
    imw r1 ^current_char
    ldw r1 rpp r1
    cmpu r0 r1 "*"
    je r0 &try_parse_comment_c
    cmpu r0 r1 "/"
    je r0 &try_parse_comment_cxx

    ; not a comment. emit the / we consumed and fall through.
    mov r0 "/"
    call ^emit_byte

:try_parse_comment_not_found
    zero r0
    ret

:try_parse_comment_c
    jmp ^parse_comment_c

:try_parse_comment_cxx
    jmp ^parse_comment_cxx



; ==========================================================
; void parse_comment_c(void);
; ==========================================================

=parse_comment_c
    ; no stack frame

:parse_comment_c_loop

    ; consume the current char
    call ^next_char

:parse_comment_c_test

    ; check if the current char is EOF. if so it's a fatal error
    imw r1 ^current_char
    ldw r1 rpp r1
    cmpu r0 r1 -1
    je r0 &parse_comment_c_unclosed

    ; check if we have a *
    imw r1 ^current_char
    ldw r1 rpp r1
    cmpu r0 r1 "*"
    jne r0 &parse_comment_c_loop
    call ^next_char

    ; check if it's followed by /
    imw r1 ^current_char
    ldw r1 rpp r1
    cmpu r0 r1 "/"
    je r0 &parse_comment_c_done
    jmp &parse_comment_c_test

:parse_comment_c_done
    call ^next_char
    mov r0 1
    ret

:parse_comment_c_unclosed
    ; unclosed C-style comment, fatal error
    imw r0 ^error_unclosed_comment
    add r0 r0 rpp
    call ^__fatal



; ==========================================================
; void parse_comment_cxx(void);
; ==========================================================
; Parses a C++-style comment.
;
; A C++ style comment ends in a carriage return and line feed, or a carriage
; return alone, or a line feed alone, as long as these are not escaped. The
; comment can also end by the end of the file (for files that end in a
; C++-style comment without a final line ending.)
;
; A backslash at the end of a line escapes the line ending. It does not matter
; if this backslash is itself preceded by another backslash.
; ==========================================================

=parse_comment_cxx
    ; no stack frame

:parse_comment_cxx_consume

    ; consume the current char
    call ^next_char

:parse_comment_cxx_test

    ; get the current char
    imw r1 ^current_char
    ldw r1 rpp r1

    ; check if it's a backslash. if so we need to check whether we're escaping
    ; a line ending
    cmpu r0 r1 '5C   ; backslash
    je r0 &parse_comment_cxx_backslash

    ; stop consuming when we reach a line feed, carriage return or eof
    cmpu r0 r1 '0A   ; line feed
    je r0 &parse_comment_cxx_done
    cmpu r0 r1 '0D   ; carriage return
    je r0 &parse_comment_cxx_done
    cmpu r0 r1 -1    ; end-of-file
    je r0 &parse_comment_cxx_done

    ; keep going
    jmp &parse_comment_cxx_consume

:parse_comment_cxx_backslash

    ; consume the backslash
    call ^next_char

    ; get the current char
    imw r1 ^current_char
    ldw r1 rpp r1

    ; see if the next character is a carriage return, if so handle it specially
    cmpu r0 r1 '0D   ; carriage return
    je r0 &parse_comment_cxx_escaped_carriage_return

    ; see if it's a line feed, if so consume it and continue
    cmpu r0 r1 '0A   ; line feed
    je r0 &parse_comment_cxx_consume

    ; otherwise we ignore the escape sequence and keep going. we don't consume
    ; the escaped character. (a double \\ at the end of a line still escapes
    ; the line ending.)
    jmp &parse_comment_cxx_test

:parse_comment_cxx_escaped_carriage_return

    ; consume the carriage return
    call ^next_char

    ; check if we have a line feed, if so consume it and keep going
    cmpu r0 r1 '0A   ; line feed
    je r0 &parse_comment_cxx_consume

    ; otherwise test and keep going
    jmp &parse_comment_cxx_test

:parse_comment_cxx_done
    mov r0 1
    ret
