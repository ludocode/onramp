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



@lexer_char
    0

@lexer_token_capacity
    ; Currently we hardcode this to 256 bytes. It will contain string literals
    ; so it needs to be fairly large. This could be made growable but there's
    ; probably no point.
    256



; ==========================================================
; static char* lexer_token;
; ==========================================================
; Contains the current token being processed.
;
; This points to an allocated buffer of size lexer_token_capacity.
;
; In the case of an alphanumeric (identifier or keyword), a number, or
; punctuation (operators, braces, etc.), this is the literal token text as
; parsed.
;
; In the case of a string or character literal, this is the content of the
; string with escape sequences parsed.
;
; When the end of the input has been reached, this is an empty string.
; ==========================================================

=lexer_token
    0



; ==========================================================
; int lexer_type;
; ==========================================================
; The type of the token in lexer_token.
;
; A token can be one of the following types:
;
; - "a": alphanumeric (identifier or keyword)
; - "n": number
; - "c": character literal
; - "s": string literal
; - "p": punctuation
; - "e": end-of-file
; ==========================================================

=lexer_type
    0



; ==========================================================
; void lexer_init(void)
; ==========================================================

=lexer_init
    ; no stack frame

    ; allocate a token buffer
    imw r0 ^lexer_token_capacity
    ldw r0 r0 rpp
    call ^malloc
    jz r0 &lexer_init_fail

    ; assign it to lexer_token
    imw r1 ^lexer_token
    stw r0 r1 rpp

    ; set the current line to 1
    imw r0 ^current_line
    stw 1 r0 rpp

    ; the input file is already open. prime the first token
    call ^lexer_read_char
    call ^lexer_consume

    ret

:lexer_init_fail
    imw r0 ^error_out_of_memory
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; void lexer_destroy(void)
; ==========================================================

=lexer_destroy

    ; free lexer_token
    imw r0 ^lexer_token
    ldw r0 r0 rpp
    call ^free

    ret



; ==========================================================
; static char lexer_read_char(void);
; ==========================================================
; Reads one char into lexer_char and returns it.
;
; TODO I don't think we actually use the return value anywhere. We could either
; optimize the code to use it or change this to return void.
; ==========================================================

@lexer_read_char

    ; read one character from input_file
    imw r0 ^input_file
    ldw r0 r0 rpp
    call ^fgetc

    ; store it in lexer_char
    imw r1 ^lexer_char
    stw r0 r1 rpp

    ret



; ==========================================================
; static bool lexer_try_eof(void);
; ==========================================================

@lexer_try_eof
    ; no stack frame

    ; get the current character
    imw r0 ^lexer_char
    ldw r0 r0 rpp

    ; check for eof
    cmpu r0 r0 -1
    jz r0 &lexer_try_eof_true

    ; not eof
    zero r0
    ret

:lexer_try_eof_true

    ; clear the current token
    imw r0 ^lexer_token
    stb 0 r0 rpp

    ; set the type to eof
    imw r0 ^lexer_type;
    stw "e" rpp r0   ; "e" == end-of-file

    ; return true
    mov r0 1
    ret



; ==========================================================
; static bool lexer_char_is_alphanumeric(bool first);
; ==========================================================
; Returns true if the current character is valid for an alphanumeric token
; (identifier or keyword).
;
; If first is true, this accepts only letters, underscores and the dollar sign.
; If first is false, this additionally accepts digits. (Identifiers cannot
; start with digits.)
; ==========================================================

@lexer_char_is_alphanumeric
    enter
    push r0

    ; get the current character
    imw r0 ^lexer_char
    ldw r0 r0 rpp

    ; check for underscore and dollar sign
    cmpu r9 r0 "_"
    je r9 &lexer_char_is_identifier_true
    cmpu r9 r0 "$"
    je r9 &lexer_char_is_identifier_true

    ; check for letters
    call ^isalpha
    jnz r0 &lexer_char_is_identifier_true

    ; if first is true, it's not an identifier char
    ldw r0 rfp -4
    jnz r0 &lexer_char_is_identifier_false

    ; get the current character again
    imw r0 ^lexer_char
    ldw r0 r0 rpp

    ; check for digits
    call ^isdigit
    jnz r0 &lexer_char_is_identifier_true

    ; not an identifier char; fallthrough to false

:lexer_char_is_identifier_false
    ; return false
    zero r0
    leave
    ret

:lexer_char_is_identifier_true
    ; return true
    mov r0 1
    leave
    ret



; ==========================================================
; static bool lexer_try_alphanumeric(void);
; ==========================================================
; vars:
; - lexer_token: r1, rfp-4
; - length: r2, rfp-8
; - capacity: r3, rfp-12
; ==========================================================

@lexer_try_alphanumeric
    enter

    ; check if we have the first char of an alphanumeric
    mov r0 1
    call ^lexer_char_is_alphanumeric
    jz r0 &lexer_try_alphanumeric_false

    ; get a pointer to the current token
    imw r1 ^lexer_token
    ldw r1 r1 rpp
    push r1

    ; start with zero length
    zero r2
    push r2

    ; also store the capacity of the token buffer
    imw r3 ^lexer_token_capacity
    ldw r3 r3 rpp
    push r3

:lexer_try_alphanumeric_loop

    ; get the current character
    imw r0 ^lexer_char
    ldw r0 r0 rpp

    ; append the character to the current token
    stb r0 r1 r2
    inc r2
    stw r2 rfp -8

    ; check if our token is too long
    cmpu r0 r2 r3
    je r0 &lexer_try_alphanumeric_too_long

    ; consume the current char
    call ^lexer_read_char

    ; check if we have another alphanumeric char
    zero r0
    call ^lexer_char_is_alphanumeric
    jz r0 &lexer_try_alphanumeric_done

    ; restore our registers and loop
    ldw r1 rfp -4
    ldw r2 rfp -8
    ldw r3 rfp -12
    jmp &lexer_try_alphanumeric_loop

:lexer_try_alphanumeric_done

    ; null-terminate lexer_token
    ldw r1 rfp -4
    ldw r2 rfp -8
    stb 0 r1 r2

    ; set the type to alphanumeric
    imw r0 ^lexer_type;
    stw "a" rpp r0   ; "a" == alphanumeric

    ; done
    mov r0 1
    leave
    ret

:lexer_try_alphanumeric_false
    ; not a alphanumeric
    zero r0
    leave
    ret

:lexer_try_alphanumeric_too_long
    ; obviously not a keyword; identifier too long, fatal error
    imw r0 ^error_identifier_too_long
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; static char lexer_parse_one_literal_char(void);
; ==========================================================
; Reads one character (possibly escaped) in a string or char literal, consuming
; and returning it.
;
; This converts character escape sequences and checks for truncated literals.
; ==========================================================

@lexer_parse_one_literal_char
    enter

    ; get the current character
    imw r1 ^lexer_char
    ldw r0 r1 rpp

    ; check for newline or end-of-file. these indicate a
    ; truncated string or char literal.
    cmpu r9 r0 '0A   ; line feed
    je r9 &lexer_parse_one_literal_char_truncated
    cmpu r9 r0 '0D   ; carriage return
    je r9 &lexer_parse_one_literal_char_truncated
    cmpu r9 r0 -1    ; end-of-file
    je r9 &lexer_parse_one_literal_char_truncated

    ; consume the char
    push r0
    call ^lexer_read_char
    pop r0

    ; check for an escape sequence
    cmpu r9 r0 '5C   ; 5C == "\\"
    je r9 &lexer_parse_one_literal_char_escape

    ; not an escape sequence. just return the character as is.
    leave
    ret

:lexer_parse_one_literal_char_escape

    ; get the next character
    imw r1 ^lexer_char
    ldw r0 r1 rpp

    ; Handle each escape sequence. We implement all the single-character
    ; sequences in standard C plus the \e extension. We don't bother to
    ; implement octal, hex or unicode escapes at this stage.
    ; (TODO but we could as a special hack support \0 if we need it. we just
    ; need to check that it's not followed by a digit.)
    ; (TODO or we could just fully support octal and/or hex.)

    ; \\
    cmpu r9 r0 '5C
    je r9 &lexer_parse_one_literal_char_escape_ok

    ; \'
    cmpu r9 r0 "'"
    je r9 &lexer_parse_one_literal_char_escape_ok

    ; \"
    cmpu r9 r0 '22   ; 22 == "
    je r9 &lexer_parse_one_literal_char_escape_ok

    ; \?
    cmpu r9 r0 "?"
    je r9 &lexer_parse_one_literal_char_escape_ok

    ; \a
    cmpu r9 r0 "a"
    jne r9 &lexer_parse_one_literal_char_not_a
    mov r0 '07   ; audible bell
    jmp &lexer_parse_one_literal_char_escape_ok
:lexer_parse_one_literal_char_not_a

    ; \b
    cmpu r9 r0 "b"
    jne r9 &lexer_parse_one_literal_char_not_b
    mov r0 '08   ; backspace
    jmp &lexer_parse_one_literal_char_escape_ok
:lexer_parse_one_literal_char_not_b

    ; \f
    cmpu r9 r0 "f"
    jne r9 &lexer_parse_one_literal_char_not_f
    mov r0 '0C   ; form feed
    jmp &lexer_parse_one_literal_char_escape_ok
:lexer_parse_one_literal_char_not_f

    ; \n
    cmpu r9 r0 "n"
    jne r9 &lexer_parse_one_literal_char_not_n
    mov r0 '0A   ; line feed
    jmp &lexer_parse_one_literal_char_escape_ok
:lexer_parse_one_literal_char_not_n

    ; \r
    cmpu r9 r0 "r"
    jne r9 &lexer_parse_one_literal_char_not_r
    mov r0 '0D   ; carriage return
    jmp &lexer_parse_one_literal_char_escape_ok
:lexer_parse_one_literal_char_not_r

    ; \t
    cmpu r9 r0 "t"
    jne r9 &lexer_parse_one_literal_char_not_t
    mov r0 '09   ; horizontal tab
    jmp &lexer_parse_one_literal_char_escape_ok
:lexer_parse_one_literal_char_not_t

    ; \v
    cmpu r9 r0 "v"
    jne r9 &lexer_parse_one_literal_char_not_v
    mov r0 '0B   ; vertical tab
    jmp &lexer_parse_one_literal_char_escape_ok
:lexer_parse_one_literal_char_not_v

    ; \e (extension, not standard C)
    cmpu r9 r0 "e"
    jne r9 &lexer_parse_one_literal_char_not_e
    mov r0 '1B   ; escape
    jmp &lexer_parse_one_literal_char_escape_ok
:lexer_parse_one_literal_char_not_e

    ; fatal, unsupported escape sequence
    imw r0 ^error_unsupported_escape
    add r0 r0 rpp
    call ^fatal

:lexer_parse_one_literal_char_escape_ok

    ; consume the escaped character
    push r0
    call ^lexer_read_char
    pop r0

    ; return the converted escape character
    leave
    ret

:lexer_parse_one_literal_char_truncated
    ; truncated literal
    imw r0 ^error_literal_truncated
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; static bool lexer_try_char_literal(void);
; ==========================================================
; Parses a character literal.
; ==========================================================

@lexer_try_char_literal
    enter

    ; get the current character
    imw r0 ^lexer_char
    ldw r0 r0 rpp

    ; check if it's a single quote
    cmpu r9 r0 "'"
    jne r9 &lexer_try_char_literal_false

    ; consume it
    call ^lexer_read_char

    ; if the current character is another single quote, it's a fatal error
    imw r0 ^lexer_char
    ldw r0 r0 rpp
    cmpu r9 r0 "'"
    je r9 &lexer_try_char_literal_count

    ; read the character, handling escape sequences and checking for errors
    call ^lexer_parse_one_literal_char

    ; put it in lexer_token and null-terminate
    imw r1 ^lexer_token
    ldw r1 r1 rpp
    stb r0 r1 0
    stb 0 r1 1

    ; the current character must be a single quote again
    imw r0 ^lexer_char
    ldw r0 r0 rpp
    cmpu r9 r0 "'"
    jne r9 &lexer_try_char_literal_count

    ; consume the closing quote
    call ^lexer_read_char

    ; set the type
    imw r0 ^lexer_type;
    stw "c" rpp r0   ; "c" == character literal

    ; return true
    mov r0 1
    leave
    ret

:lexer_try_char_literal_false
    ; not a char literal
    zero r0
    leave
    ret

:lexer_try_char_literal_count
    ; wrong number of characters in char literal
    imw r0 ^error_char_literal_count
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; static bool lexer_try_string_literal(void);
; ==========================================================
; Parses a string literal.
;
; vars:
; - lexer_token: r1, rfp-4
; - length: r2, rfp-8
; - capacity: r3, rfp-12
; - &lexer_char: r4, rfp-16
; ==========================================================

@lexer_try_string_literal
    enter

    ; get the current character
    imw r0 ^lexer_char
    ldw r0 r0 rpp

    ; check if it's a double quote
    cmpu r9 r0 '22   ; 0x22 == "
    jne r9 &lexer_try_string_literal_false

    ; consume it
    call ^lexer_read_char

    ; get a pointer to the current token
    imw r1 ^lexer_token
    ldw r1 r1 rpp
    push r1

    ; start with zero length
    zero r2
    push r2

    ; also store the capacity of the token buffer
    imw r3 ^lexer_token_capacity
    ldw r3 r3 rpp
    push r3

    ; and store a pointer to the current character
    imw r4 ^lexer_char
    add r4 r4 rpp
    push r4

:lexer_try_string_literal_loop

    ; check if the current character is a double-quote. if so we're done.
    ldw r5 r4 0
    cmpu r9 r5 '22   ; 0x22 == "
    je r9 &lexer_try_string_literal_done

    ; read a character, handling escape sequences and checking for errors
    call ^lexer_parse_one_literal_char

    ; restore our registers
    ldw r1 rfp -4
    ldw r2 rfp -8
    ldw r3 rfp -12
    ldw r4 rfp -16

    ; append the new char to lexer_token
    stb r0 r1 r2
    inc r2
    stw r2 rfp -8

    ; check if our token is too long
    cmpu r0 r2 r3
    je r0 &lexer_try_string_literal_too_long

    ; loop
    jmp &lexer_try_string_literal_loop

:lexer_try_string_literal_done

    ; null-terminate lexer_token
    stb 0 r1 r2

    ; consume the closing double quote
    call ^lexer_read_char

    ; set the type
    imw r0 ^lexer_type;
    stw "s" rpp r0   ; "s" == string literal

    ; return true
    mov r0 1
    leave
    ret

:lexer_try_string_literal_false
    ; not a literal
    zero r0
    leave
    ret

:lexer_try_string_literal_too_long
    ; literal too long, fatal error
    imw r0 ^error_literal_too_long
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; static bool lexer_try_number(void);
; ==========================================================
; Parses a number.
;
; Anything that starts with a digit is a number. We consume all subsequent
; alphanumeric characters, periods for floats, and quotes for the digit
; separator in order to ensure our syntax is forwards-compatible with C.
;
; TODO we also need to consume +/- after e/E. Maybe we should make this a more
; correct parsing of the float syntax. See e.g.:
;     https://en.cppreference.com/w/c/language/floating_constant
;     https://learn.microsoft.com/en-us/cpp/c-language/c-floating-point-constants
; or maybe we should just simplify it, get rid of periods and quotes, they're
; not supported in omC anyway. we don't even support any suffixes.
;
; The lexer doesn't check the validity of numbers. The parser will parse them
; into supported number types and check for errors.
;
; vars:
; - lexer_token: r1, rfp-4
; - length: r2, rfp-8
; - capacity: r3, rfp-12
; - &lexer_char: r4, rfp-16
; ==========================================================

@lexer_try_number
    enter

    ; get the current character
    imw r0 ^lexer_char
    ldw r0 r0 rpp

    ; make sure it's a digit
    call ^isdigit
    jz r0 &lexer_try_number_false

    ; get a pointer to the current token
    imw r1 ^lexer_token
    ldw r1 r1 rpp
    push r1

    ; start with zero length
    zero r2
    push r2

    ; also store the capacity of the token buffer
    imw r3 ^lexer_token_capacity
    ldw r3 r3 rpp
    push r3

    ; and store a pointer to the current character
    imw r4 ^lexer_char
    add r4 r4 rpp
    push r4

:lexer_try_number_loop

    ; append the current character to the current token
    ldb r0 r4 0
    stb r0 r1 r2
    inc r2
    stw r2 rfp -8

    ; check if our token is too long
    cmpu r0 r2 r3
    je r0 &lexer_try_number_too_long

    ; consume the current char
    call ^lexer_read_char

    ; check if we have another number character
    ldw r4 rfp -16
    ldb r0 r4 0
    cmpu r9 r0 "."
    je r9 &lexer_try_number_more
    cmpu r9 r0 "'"
    je r9 &lexer_try_number_more
    call ^isalnum
    jnz r0 &lexer_try_number_more
    jmp &lexer_try_number_done

:lexer_try_number_more

    ; restore our registers and loop
    ldw r1 rfp -4
    ldw r2 rfp -8
    ldw r3 rfp -12
    ldw r4 rfp -16
    jmp &lexer_try_number_loop

:lexer_try_number_done

    ; null-terminate lexer_token
    ldw r1 rfp -4
    ldw r2 rfp -8
    stb 0 r1 r2

    ; set the type to number
    imw r0 ^lexer_type;
    stw "n" rpp r0   ; "n" == number

    ; done
    mov r0 1
    leave
    ret

:lexer_try_number_false
    ; not a number
    zero r0
    leave
    ret

:lexer_try_number_too_long
    ; number too long, fatal error
    imw r0 ^error_number_too_long
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; static char* lexer_punctuation;
; ==========================================================
; A list of all punctuation accepted by the lexer.
;
; When we test for punctuation, we append characters to the current token until
; we no longer match anything in this list. This gives us the longest matching
; punctuation. (All leading substrings of valid punctuation are themselves
; valid punctuation, except for `...` which we ignore.)
;
; We match all punctuation in C (except `...` and digraphs), even those that
; aren't in omC, in order to ensure we are forwards-compatible and to give
; better error messages for unsupported syntax.
;
; This list is quite long so punctuation is tested last for performance.
; TODO we should probably convert this to a hashtable.
; ==========================================================

@lexer_punctuation
    ; parentheses, brackets and related syntax
    "(" '00
    ")" '00
    "[" '00
    "]" '00
    "{" '00
    "}" '00
    ";" '00

    ; single-character operators
    "+" '00
    "-" '00
    "*" '00
    "/" '00
    "%" '00
    "&" '00
    "|" '00
    "^" '00
    "!" '00
    "~" '00
    "<" '00
    ">" '00
    "=" '00
    "." '00
    "?" '00
    ":" '00
    "," '00

    ; compound assignment operators
    "+=" '00
    "-=" '00
    "*=" '00
    "/=" '00
    "%=" '00
    "&=" '00
    "|=" '00
    "^=" '00
    "<<=" '00
    ">>=" '00

    ; two-character comparison operators
    "!=" '00
    "<=" '00
    ">=" '00
    "==" '00

    ; other operators
    "++" '00
    "--" '00
    "<<" '00
    ">>" '00
    "&&" '00
    "||" '00
    "->" '00

    ; end of list
    '00



; ==========================================================
; static bool lexer_is_punctuation(void);
; ==========================================================
; Returns true if lexer_token contains valid punctuation.
;
; vars:
; - lexer_token: r0, rfp-4
; - position in lexer_punctuation: r1, rfp-8
; ==========================================================

@lexer_is_punctuation
    enter

    ; get the current token
    imw r0 ^lexer_token
    ldw r0 rpp r0
    push r0

    ; get the list of punctuation
    imw r1 ^lexer_punctuation
    add r1 rpp r1
    push r1

:lexer_is_punctuation_loop

    ; if we have no punctuation left to match, it's not punctuation
    ldb r9 0 r1
    jz r9 &lexer_is_punctuation_false

    ; call strcmp(). if match, it's punctuation
    call ^strcmp
    jz r0 &lexer_is_punctuation_true

    ; reload our registers
    ldw r0 rfp -4
    ldw r1 rfp -8

    ; step to the next punctuation in the list
:lexer_is_punctuation_step
    ldb r9 0 r1
    inc r1
    jnz r9 &lexer_is_punctuation_step
    stw r1 rfp -8

    ; keep looping
    jmp &lexer_is_punctuation_loop

:lexer_is_punctuation_false
    zero r0
    leave
    ret

:lexer_is_punctuation_true
    mov r0 1
    leave
    ret



; ==========================================================
; static bool lexer_try_punctuation(void);
;
; vars:
; - lexer_token: r0, rfp-4
; - &lexer_char: r1, rfp-8
; - token length: r2, rfp-12
; ==========================================================

@lexer_try_punctuation
    enter

    ; get the token buffer
    imw r0 ^lexer_token
    ldw r0 rpp r0
    push r0

    ; get a pointer to lexer_char
    imw r1 ^lexer_char
    add r1 r1 rpp
    push r1

    ; start with zero length
    zero r2
    push r2

:lexer_try_punctuation_loop

    ; append lexer_char to lexer_token and null-terminate. note we aren't
    ; consuming the char yet; we'll consume it if it's confirmed to be part of
    ; valid punctuation.
    ldb r9 0 r1
    stb r9 r0 r2
    inc r2
    stw r2 rfp -12
    stb 0 r0 r2

    ; check if this is valid punctuation. if not, break out of the loop
    call ^lexer_is_punctuation
    jz r0 &lexer_try_punctuation_done

    ; it's valid, so consume the char
    call ^lexer_read_char

    ; restore our registers
    ldw r0 rfp -4
    ldw r1 rfp -8
    ldw r2 rfp -12

    ; loop
    jmp &lexer_try_punctuation_loop

:lexer_try_punctuation_done

    ; restore the registers we need
    ldw r0 rfp -4
    ldw r2 rfp -12

    ; the last character is not valid for punctuation so we'll trim it off. if
    ; our resulting length is zero, this is not valid punctuation.
    dec r2
    jz r2 &lexer_try_punctuation_false

    ; trim the last character off
    stb 0 r0 r2

    ; set the type to punctuation
    imw r0 ^lexer_type;
    stw "p" rpp r0   ; "p" == punctuation

    ; done
    mov r0 1
    leave
    ret

:lexer_try_punctuation_false
    ; not punctuation
    zero r0
    leave
    ret



; ==========================================================
; static void lexer_consume_whitespace(void);
; ==========================================================
; Consumes all whitespace.
; ==========================================================

@lexer_consume_whitespace
    enter

:lexer_consume_whitespace_loop

    ; get the current character
    imw r0 ^lexer_char
    ldw r0 r0 rpp

    ; check if it's whitespace. if not, we're done.
    call ^isspace
    jz r0 &lexer_consume_whitespace_done

    ; get the current character again
    imw r0 ^lexer_char
    ldw r0 r0 rpp

    ; check if this is a line feed
    cmpu r9 r0 '0A
    je r9 &lexer_consume_whitespace_line_feed

    ; check if this is a carriage return
    cmpu r9 r0 '0D
    je r9 &lexer_consume_whitespace_carriage_return

    ; it's not a newline. consume the whitespace
    call ^lexer_read_char

    ; loop
    jmp &lexer_consume_whitespace_loop

:lexer_consume_whitespace_line_feed

    ; consume the line feed
    call ^lexer_read_char

    ; increment the line
    jmp &lexer_consume_whitespace_increment_line

:lexer_consume_whitespace_carriage_return

    ; consume the carriage return
    call ^lexer_read_char

    ; check if it's followed by a line feed. if so, it's part of the same line;
    ; we can go to line feed to consume it as well.
    imw r0 ^lexer_char
    ldw r0 r0 rpp
    cmpu r9 r0 '0A
    je r9 &lexer_consume_whitespace_line_feed

    ; fallthrough

:lexer_consume_whitespace_increment_line

    ; increment the current line
    imw r1 ^current_line
    ldw r0 r1 rpp
    inc r0
    stw r0 r1 rpp

    ; loop
    jmp &lexer_consume_whitespace_loop

:lexer_consume_whitespace_done
    leave
    ret



; ==========================================================
; static void lexer_parse_line_number(void);
; ==========================================================
; Parses the number in a #line directive.
;
; vars:
; - value: rfp-4
; ==========================================================

@lexer_parse_line_number
    enter
    push 0

:lexer_parse_line_number_loop

    ; check if the current character is a digit. if not we're done.
    imw r0 ^lexer_char
    ldw r0 r0 rpp
    call ^isdigit
    jz r0 &lexer_parse_line_number_done

    ; add it to the number
    imw r0 ^lexer_char
    ldw r0 r0 rpp
    sub r0 r0 "0"
    ldw r1 rfp -4
    mul r1 r1 10
    add r1 r1 r0
    stw r1 rfp -4

    ; next char
    call ^lexer_read_char
    jmp &lexer_parse_line_number_loop

:lexer_parse_line_number_done

    ; we decrement the line by 1 because it will get incremented again at the
    ; end of the directive line.
    ldw r0 rfp -4
    dec r0

    ; set the current line
    imw r1 ^current_line
    stw r0 rpp r1

    leave
    ret



; ==========================================================
; static void lexer_parse_line_directive(void);
; ==========================================================
; Parses a #line directive.
; ==========================================================

@lexer_parse_line_directive

    ; horizontal space
    call ^lexer_consume_horizontal_space

    ; #line must be followed by a number.
    imw r0 ^lexer_char
    ldw r0 r0 rpp
    call ^isdigit
    jz r0 &lexer_parse_line_directive_error
    call ^lexer_parse_line_number

    ; more horizontal space
    call ^lexer_consume_horizontal_space

    ; filename is optional
    call ^lexer_try_string_literal
    jz r0 &lexer_parse_line_directive_done

    ; set the filename
    imw r0 ^lexer_token
    ldw r0 rpp r0
    call ^set_current_filename

:lexer_parse_line_directive_done
    ret

:lexer_parse_line_directive_error
    ; error, unrecognized preprocessor directive
    -1
    imw r0 ^error_preprocessor_directive
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; static void lexer_discard_line(void);
; ==========================================================
; Consumes the rest of the line.
;
; This is used to consume the rest of preprocessor directives.
; ==========================================================

@lexer_discard_line

    ; get the current character
    imw r0 ^lexer_char
    ldw r0 r0 rpp

    ; check if it's end of line
    cmpu r1 r0 -1
    je r1 &lexer_discard_line_done
    cmpu r1 r0 '0A   ; line feed
    je r1 &lexer_discard_line_done
    cmpu r1 r0 '0D   ; carriage return
    je r1 &lexer_discard_line_done

    ; consume it
    call ^lexer_read_char

    ; keep looping
    jmp &lexer_discard_line

:lexer_discard_line_done
    ret



; ==========================================================
; static void lexer_consume_horizontal_space(void);
; ==========================================================
; Consumes all horizontal whitespace (space or tab).
; ==========================================================

@lexer_consume_horizontal_space

    ; check if the current character is space or tab
    imw r0 ^lexer_char
    ldw r0 r0 rpp
    sub r1 r0 " "  ; space
    je r1 &lexer_consume_horizontal_space_found
    sub r1 r0 9    ; horizontal tab
    je r1 &lexer_consume_horizontal_space_found
    jmp &lexer_consume_horizontal_space_done

:lexer_consume_horizontal_space_found
    ; consume it and loop
    call ^lexer_read_char
    jmp &lexer_consume_horizontal_space

:lexer_consume_horizontal_space_done
    ret



; ==========================================================
; static void lexer_parse_directive(void);
; ==========================================================
; Consumes a preprocessor directive.
;
; We only support #line. All other directives are ignored.
; ==========================================================

@lexer_parse_directive

    ; consume the '#'
    call ^lexer_read_char

    ; consume horizontal space (we allow spaces between # and the command)
    call ^lexer_consume_horizontal_space

    ; check if we're at the end of the line. if so, we're done.
    cmpu r1 r0 -1
    je r1 &lexer_parse_directive_done
    cmpu r1 r0 '0A   ; line feed
    je r1 &lexer_parse_directive_done
    cmpu r1 r0 '0D   ; carriage return
    je r1 &lexer_parse_directive_done

    ; parse the command. if it fails, the line is invalid.
    call ^lexer_try_alphanumeric
    jz r0 &lexer_parse_directive_error

    ; check if it's #line
    imw r0 ^lexer_token
    ldw r0 rpp r0
    imw r1 ^str_line
    add r1 rpp r1
    call ^strcmp
    je r0 &lexer_parse_directive_line

    ; check if it's #pragma
    imw r0 ^lexer_token
    ldw r0 rpp r0
    imw r1 ^str_pragma
    add r1 rpp r1
    call ^strcmp
    je r0 &lexer_parse_directive_pragma

    ; otherwise we don't support it.
    jmp &lexer_parse_directive_error

:lexer_parse_directive_line
    ; it's a #line directive.
    jmp ^lexer_parse_line_directive

:lexer_parse_directive_pragma
    ; it's a #pragma directive. we don't support them.
    jmp ^lexer_discard_line

:lexer_parse_directive_done
    ret

:lexer_parse_directive_error
    ; error, unrecognized preprocessor directive
    imw r0 ^error_preprocessor_directive
    add r0 r0 rpp
    call ^fatal


; ==========================================================
; static void lexer_consume_whitespace_and_directives(void);
; ==========================================================
; Consumes all whitespace and preprocessor directives.
; ==========================================================

@lexer_consume_whitespace_and_directives
    enter

:lexer_consume_whitespace_and_directives_loop
    call ^lexer_consume_whitespace

    ; get the current character
    imw r0 ^lexer_char
    ldw r0 r0 rpp

    ; check if it's '#'. if not, we're done.
    ; (note that we don't bother to check if it's at the beginning of a line.)
    cmpu r0 r0 "#"
    je r0 &lexer_consume_whitespace_and_directives_found
    leave
    ret

:lexer_consume_whitespace_and_directives_found
    ; directive found. parse it, then go back to consuming whitespace.
    call ^lexer_parse_directive
    jmp &lexer_consume_whitespace_and_directives_loop



; ==========================================================
; void lexer_consume(void);
; ==========================================================
; Consumes the current token, reading the next into lexer_token.
; ==========================================================

=lexer_consume
    enter

    ; Skip whitespace and handle preprocessor directives. This brings us to the
    ; start of the next real token.
    call ^lexer_consume_whitespace_and_directives

    ; Check for end of file (type 'e')
    call ^lexer_try_eof
    jnz r0 &lexer_consume_done

    ; Check for alphanumeric (type 'a')
    call ^lexer_try_alphanumeric
    jnz r0 &lexer_consume_done

    ; Check for a number (type 'n')
    call ^lexer_try_number
    jnz r0 &lexer_consume_done

    ; Check for a character literal (type 'c')
    call ^lexer_try_char_literal
    jnz r0 &lexer_consume_done

    ; Check for a string literal (type 's')
    call ^lexer_try_string_literal
    jnz r0 &lexer_consume_done

    ; Check for punctuation (type 'p')
    ; (This is probably slowest so we test it last)
    call ^lexer_try_punctuation
    jnz r0 &lexer_consume_done

    ; Fail
    imw r0 ^error_unexpected_character
    add r0 r0 rpp
    call ^fatal

:lexer_consume_done
    leave
    ret



; ==========================================================
; char* lexer_take(void);
; ==========================================================
; Consumes the current token, returning ownership of it (or of a newly
; allocated copy.)
;
; You must free the returned string.
; ==========================================================

=lexer_take

    ; strdup() the current token
    imw r0 ^lexer_token
    ldw r0 rpp r0
    call ^strdup
    jz r0 &lexer_take_fail

    ; push it to the stack
    push r0

    ; consume the current token
    call ^lexer_consume

    ; pop and return
    pop r0
    ret

:lexer_take_fail
    imw r0 ^error_out_of_memory
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; void lexer_expect(const char*);
; ==========================================================
; Consumes the current token, raising an error if it does not match what is
; given.
; ==========================================================

=lexer_expect

    ; make sure the token matches
    call ^lexer_is
    jz r0 &lexer_expect_fail

    ; tail-call to consume the token
    jmp ^lexer_consume

:lexer_expect_fail
    ; the token doesn't match. fatal error
    imw r0 ^error_unexpected_token
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; bool lexer_accept(const char* token_string);
; ==========================================================
; If the current token matches that given, it is consumed, and true is
; returned; otherwise the current token is unchanged and false is returned.
;
; This always returns false if the current token is a string literal, a
; character literal or the end of the input.
; ==========================================================

=lexer_accept
    enter

    ; check whether the token matches
    call ^lexer_is
    jz r0 &lexer_accept_false

    ; consume the token
    call ^lexer_consume

    ; return true
    mov r0 1
    leave
    ret

:lexer_accept_false
    ; the token doesn't match. return false
    zero r0
    leave
    ret



; ==========================================================
; bool lexer_expect_char(char token_char);
; ==========================================================

=lexer_expect_char
    push r0
    mov r0 rsp
    call ^lexer_expect
    popd
    ret



; ==========================================================
; bool lexer_accept_char(char token_char);
; ==========================================================

=lexer_accept_char
    push r0
    mov r0 rsp
    call ^lexer_accept
    popd
    ret



; ==========================================================
; bool lexer_is(const char*);
; ==========================================================
; Returns true if the current token matches that given without consuming it.
;
; This always returns false if the current token is a string literal, a
; character literal or the end of the input.
;
; TODO possibly also make this false for numbers. maybe it should only return
; true for alphanumeric and punctuation.
; ==========================================================

=lexer_is
    enter

    ; get the token type into r1
    imw r1 ^lexer_type;
    ldw r1 rpp r1

    ; Check for a string, character or EOF. If it's any of these, lexer_is()
    ; always returns false.
    cmpu r9 r1 "s"            ; "s" == string literal
    jz r9 &lexer_is_false
    cmpu r9 r1 "c"            ; "c" == character literal
    jz r9 &lexer_is_false
    cmpu r9 r1 "e"            ; "e" == end-of-file
    jz r9 &lexer_is_false

    ; call strcmp
    imw r1 ^lexer_token
    ldw r1 rpp r1
    call ^strcmp

    ; convert to bool
    and r0 r0 1
    xor r0 r0 1

    leave
    ret

:lexer_is_false
    zero r0
    leave
    ret



; ==========================================================
; void lexer_dump_tokens(void);
; ==========================================================
; Causes the lexer to print all tokens to standard output.
;
; This is just used for debugging the lexer.
; ==========================================================

=lexer_dump_tokens
    enter

:lexer_dump_tokens_loop

    ; get the token type
    imw r0 ^lexer_type
    ldw r0 rpp r0

    ; if it's EOF, return
    cmpu r9 r0 "e"  ; "e" == end-of-file
    jne r9 &lexer_dump_tokens_not_eof
    leave
    ret
:lexer_dump_tokens_not_eof

    ; print the token type
    call ^putchar
    mov r0 " "
    call ^putchar
    mov r0 " "
    call ^putchar

    ; print the token
    imw r0 ^lexer_token
    ldw r0 rpp r0
    call ^puts

    ; consume it
    call ^lexer_consume

    ; loop
    jmp &lexer_dump_tokens_loop
