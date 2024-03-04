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
; int frame_size;
; ==========================================================
; The maximum extent of the stack frame of the current function being parsed.
; ==========================================================

@frame_size
    0



; ==========================================================
; int next_label;
; ==========================================================
; The label number to use for the next generated label.
; ==========================================================

@next_label
    0



; ==========================================================
; int while_start_label;
; ==========================================================
; The label number at the start of the current `while` loop (to jump to on a
; `continue` statement.)
; ==========================================================

@while_start_label
    0



; ==========================================================
; int while_end_label;
; ==========================================================
; The label number at the end of the current `while` loop (to jump to on a
; `break` statement.)
; ==========================================================

@while_end_label
    0



; ==========================================================
; int strings_emitted;
; ==========================================================
; The total number of strings that have been emitted so far.
; ==========================================================

@strings_emitted
    0



; ==========================================================
; char** strings;
; ==========================================================
; The array of strings.
; ==========================================================

@strings
    0



; ==========================================================
; size_t strings_capacity;
; ==========================================================
; The maximum number of strings per function.
; ==========================================================

@strings_capacity
    128



; ==========================================================
; size_t strings_count;
; ==========================================================
; The number of strings so far in this function.
; ==========================================================

@strings_count
    0



; ==========================================================
; void parse_init(void);
; ==========================================================

=parse_init

    ; allocate the strings array
    imw r0 ^strings_capacity
    ldw r0 rpp r0
    shl r0 r0 2
    call ^malloc
    jz r0 &parse_init_fail

    ; store it
    imw r1 ^strings
    stw r0 rpp r1

    ret

:parse_init_fail
    imw r0 ^error_out_of_memory
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; void parse_destroy(void);
; ==========================================================

=parse_destroy

    ; free strings
    imw r0 ^strings
    ldw r0 rpp r0
    call ^free

    ret



; ==========================================================
; static bool try_parse_type(char* out_type);
; ==========================================================
; Tries to parse a type. Returns true if successful, false otherwise.
;
; TODO this could just return 0 if it's not a type, would be way simpler, and
; no confusion with ldw/ldb
;
; vars:
; - out_type: rfp-4
; - has_const: rfp-8
; - type: rfp-12
; - "*": rfp-16
; ==========================================================

@try_parse_type
    enter
    push r0

    ; check for `const`. we don't actually implement const but if we find it, a
    ; type won't be optional.
    imw r0 ^str_const
    add r0 r0 rpp
    call ^lexer_accept
    push r0

    ; try to find the base type
    imw r0 ^lexer_token
    ldw r0 r0 rpp
    sub rsp rsp 4
    mov r1 rsp
    call ^type_find
    jz r0 &try_parse_type_not_found

    ; consume the token
    call ^lexer_consume

    ; put "*" on the stack to collect indirections
    push "*"

:try_parse_type_star_loop

    ; accept an indirection
    add r0 rfp -16
    call ^lexer_accept
    jz r0 &try_parse_type_star_done

    ; increment the indirection count of the type
    ldb r0 rfp -12
    inc r0
    and r1 r0 0xF
    jz r1 &try_parse_type_too_many_indirections
    stb r0 rfp -12

    jmp &try_parse_type_star_loop

:try_parse_type_star_done

    ; output the type
    ldw r0 rfp -4
    ldb r1 rfp -12
    stb r1 r0 0

    ; return true
    mov r0 1
    leave
    ret

:try_parse_type_not_found
    ; if `const` was not followed by a type, it's a fatal error
    ldw r0 rfp -8
    jz r0 &try_parse_type_not_found_ok
    imw r0 ^error_const_must_be_followed_by_type
    add r0 r0 rpp
    call ^fatal

:try_parse_type_not_found_ok
    ; otherwise it's not a type. return false.
    zero r0
    leave
    ret

:try_parse_type_too_many_indirections
    ; fatal, too many indirections
    imw r0 ^error_too_many_indirections
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; static char parse_type(void);
; ==========================================================

@parse_type
    enter

    ; try to parse a type
    sub rsp rsp 4
    mov r0 rsp
    call ^try_parse_type

    ; if it failed, fatal error
    jnz r0 &parse_type_ok
    imw r0 ^error_expected_type
    add r0 r0 rpp
    call ^fatal
:parse_type_ok

    ; return the type
    ldb r0 rsp 0
    leave
    ret



; ==========================================================
; static char* parse_alphanumeric(void);
; ==========================================================
; Expects the current token to be alphanumeric and returns a copy of it.
; ==========================================================

@parse_alphanumeric
    enter

    ; check if the token type is alphanumeric
    imw r0 ^lexer_type
    ldw r0 rpp r0
    cmpu r0 r0 "a"   ; "a" == alphanumeric
    je r0 &parse_alphanumeric_ok

    ; if it's not alphanumeric, fatal error
    imw r0 ^error_expected_alphanumeric
    add r0 r0 rpp
    call ^fatal
:parse_alphanumeric_ok

    ; return a copy
    call ^lexer_take
    leave
    ret

:parse_alphanumeric_oom
    imw r0 ^error_out_of_memory
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; static void parse_typedef(void);
; ==========================================================
; vars:
; - type: rfp-4
; - name: rfp-8
; ==========================================================

@parse_typedef
    enter

    ; parse a type
    call ^parse_type
    push r0

    ; parse a name
    call ^parse_alphanumeric
    push r0

    ; add the type
    ldw r0 rfp -4
    ldw r1 rfp -8
    call ^type_add

    ; require a closing semicolon
    mov r0 ";"
    call ^lexer_expect_char

    leave
    ret



; ==========================================================
; static void parse_global_variable(char type, char* name,
;                                   bool is_extern, bool is_static);
; ==========================================================

@parse_global_variable
    enter
    push r0
    push r1
    push r2
    push r3

    ; add the variable
    call ^globals_add_variable

    ; check if the variable is extern. if it is, we won't emit a definition.
    ldw r0 rfp -12    ; extern
    jnz r0 &parse_global_variable_not_extern

    ; emit a definition
    ldw r1 rfp -8    ; name
    ldw r2 rfp -16   ; is_static
    call ^compile_global_variable_definition
    call ^compile_global_divider

:parse_global_variable_not_extern
    leave
    ret



; ==========================================================
; static void parse_local_variable_definition(char type);
; ==========================================================
; Parses a local variable definition.
;
; vars:
; - type: rfp-4
; - name: rfp-8
; - expr_type: rfp-12
; ==========================================================

@parse_local_variable_definition
    enter
    sub rsp rsp 12
    stw r0 rfp -4

    ; parse a name
    call ^parse_alphanumeric
    stw r0 rfp -8

    ; add the variable
    ; (this takes ownership of the string but we'll still use it for a bit)
    mov r1 r0
    ldw r0 rfp -4
    call ^locals_add

    ; check for =, in which case it's an initializer
    mov r0 "="
    call ^lexer_accept_char
    jz r0 &parse_local_variable_done

    ; it's an initializer. compile the variable and push it
    ldw r0 rfp -8
    call ^compile_load_variable
    stw r0 rfp -4
    call ^compile_push_r0

    ; parse a unary expression (not binary, we don't support precedence!)
    call ^parse_unary_expression
    stw r0 rfp -12

    ; pop the variable to r1
    mov r0 1
    call ^compile_pop

    ; compile an assignment
    imw r0 ^str_op_assign ; "="
    add r0 r0 rpp
    ldw r1 rfp -4   ; type of left-hand side (the variable)
    ldw r2 rfp -12  ; type of right-hand side (the expression)
    call ^compile_binary_op

:parse_local_variable_done

    ; require a trailing semicolon
    mov r0 ";"
    call ^lexer_expect_char

    leave
    ret



; ==========================================================
; void parse_condition(int label);
; ==========================================================
; Parses the condition of an `if` or `while` statement.
;
; This emits code to check the condition and, if false, jump to the given label.
; ==========================================================

=parse_condition

    ; store the label
    push r0

    ; open paren
    mov r0 "("
    call ^lexer_expect_char

    ; parse the expression, store its type
    call ^parse_expression
    push r0

    ; close paren
    mov r0 ")"
    call ^lexer_expect_char

    ; if the type is an l-value, dereference it
    pop r0
    zero r1
    call ^compile_dereference_if_lvalue

    ; emit the conditional jump
    pop r0
    jmp ^compile_jump_if_zero



; ==========================================================
; void parse_if(void);
; ==========================================================

=parse_if

    ; generate an end label
    imw r1 ^next_label
    ldw r0 rpp r1
    push r0
    inc r0
    stw r0 rpp r1

    ; emit the condition check (to jump to the end label if false)
    ldw r0 rsp 0
    call ^parse_condition

    ; parse the block
    mov r0 "{"
    call ^lexer_expect_char
    call ^parse_block

    ; emit the end label
    pop r0
    call ^compile_label

    ; done
    ret



; ==========================================================
; void parse_while(void);
; ==========================================================
; Parses a while loop.
;
; vars:
; - old_start_label: rfp-4
; - old_end_label: rfp-8
; ==========================================================

=parse_while
    enter

    ; get the value of next_label
    imw r3 ^next_label
    ldw r2 rpp r3

    ; We generate new start and end labels, and we also preserve the old ones
    ; and restore them afterwards. This gives us correct break and continue
    ; behaviour for nested while loops.

    ; generate a new start label (preserving the old one)
    imw r1 ^while_start_label
    ldw r0 rpp r1
    push r0
    stw r2 rpp r1
    inc r2

    ; generate a new end label (preserving the old one)
    imw r1 ^while_end_label
    ldw r0 rpp r1
    push r0
    stw r2 rpp r1
    inc r2

    ; store the new next_label
    stw r2 rpp r3

    ; emit the start label
    imw r1 ^while_start_label
    ldw r0 rpp r1
    call ^compile_label

    ; emit the condition check (to jump to the end label if false)
    imw r1 ^while_end_label
    ldw r0 rpp r1
    call ^parse_condition

    ; parse the block
    mov r0 "{"
    call ^lexer_expect_char
    call ^parse_block

    ; emit a jump back to the start
    imw r1 ^while_start_label
    ldw r0 rpp r1
    call ^compile_jump

    ; emit the end label
    imw r1 ^while_end_label
    ldw r0 rpp r1
    call ^compile_label

    ; restore old labels
    ldw r0 rfp -4
    imw r1 ^while_start_label
    stw r0 rpp r1
    ldw r0 rfp -8
    imw r1 ^while_end_label
    stw r0 rpp r1

    ; done
    leave
    ret



; ==========================================================
; void parse_break(void);
; ==========================================================

=parse_break

    ; break must be followed by a semicolon
    mov r0 ";"
    call ^lexer_expect_char

    ; get the end label for the current while loop
    imw r0 ^while_end_label
    ldw r0 rpp r0

    ; if the label is -1, we're not in a while loop; fatal error
    sub r1 r0 -1
    jz r1 &parse_break_fatal

    ; emit a jump to the label
    jmp ^compile_jump

:parse_break_fatal
    imw r0 ^error_break_continue
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; void parse_continue(void);
; ==========================================================

=parse_continue

    ; continue must be followed by a semicolon
    mov r0 ";"
    call ^lexer_expect_char

    ; get the start label for the current while loop
    imw r0 ^while_start_label
    ldw r0 rpp r0

    ; if the label is -1, we're not in a while loop; fatal error
    sub r1 r0 -1
    jz r1 &parse_continue_fatal

    ; emit a jump to the label
    jmp ^compile_jump

:parse_continue_fatal
    imw r0 ^error_break_continue
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; void parse_return(void);
; ==========================================================

=parse_return

    ; check for a semicolon
    mov r0 ";"
    call ^lexer_accept_char
    jz r0 &parse_return_argument

    ; we have no argument. we still need to return 0 in case this is main().
    call ^compile_zero
    jmp &parse_return_ok

:parse_return_argument

    ; we have an argument. parse it and dereference it
    call ^parse_expression
    zero r1
    call ^compile_dereference_if_lvalue

    ; TODO if the function returns char we need to truncate / sign extend! do
    ; we do this here? or in callers? or in expression parsing when we need to
    ; promote?

    ; argument must be followed by a semicolon
    mov r0 ";"
    call ^lexer_expect_char

:parse_return_ok
    jmp ^compile_return



; ==========================================================
; char parse_sizeof(void);
; ==========================================================
; Parses a sizeof(type) expression.
;
; The `sizeof` keyword and opening parenthesis have already been consumed.
; ==========================================================

=parse_sizeof
    enter

    ; parse (type)
    call ^parse_type
    push r0
    mov r0 ")"
    call ^lexer_expect_char
    pop r0

    ; emit code to put the size of the type in r0
    call ^type_size
    sub rsp rsp 12   ; buffer for itoa_d()
    mov r1 rsp
    call ^itoa_d
    mov r0 rsp
    call ^compile_immediate

    ; return type int (we don't support unsigned)
    mov r0 0x30   ; int
    leave
    ret



; ==========================================================
; int parse_function_call_arguments(void);
; ==========================================================
; Parses all arguments to a function call, emitting code to push them onto the
; stack.
;
; TODO: We should take the function argcount and argtypes here and check them.
; It should be pretty easy.
;
; vars:
; - arg_count: rfp-4
; ==========================================================

=parse_function_call_arguments
    enter
    push 0

:parse_function_call_arguments_loop

    ; check if we have a closing parenthesis
    mov r0 ")"
    call ^lexer_accept_char
    jnz r0 &parse_function_call_arguments_done

    ; we need a comma to delimit arguments (if this isn't the first argument.)
    ldw r0 rfp -4
    jz r0 &parse_function_call_arguments_no_comma
    mov r0 ","
    call ^lexer_expect_char
:parse_function_call_arguments_no_comma

    ; we support at most four arguments.
    ldw r0 rfp -4
    sub r1 r0 4
    jz r1 &parse_function_call_arguments_too_many

    ; increment the arg count
    inc r0
    stw r0 rfp -4

    ; parse the arg. we want the value so dereference it if needed.
    call ^parse_binary_expression
    ; TODO for now we're ignoring the type except to dereference l-values. We
    ; actually don't need to care about the type, the called function will
    ; truncate int to char where needed, but it would still be good to do it
    ; for error checking
    zero r1
    call ^compile_dereference_if_lvalue

    ; put the argument on the stack. we'll pop them into the correct registers
    ; later.
    call ^compile_push_r0

    jmp &parse_function_call_arguments_loop

:parse_function_call_arguments_done
    ; return arg_count
    ldw r0 rfp -4
    leave
    ret

:parse_function_call_arguments_too_many
    ; fatal, too many args
    imw r0 ^error_too_many_args
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; char parse_function_call(const char* name);
; ==========================================================
; vars:
; - name: rfp-4
; - return_type: rfp-8
; - arg_count: rfp-12
; - param_count: rfp-16
; ==========================================================

=parse_function_call
    enter
    sub rsp rsp 16
    stw r0 rfp -4

    ; first we check if the name is sizeof. if so, it's not really a function
    ; call and we handle it differently. (technically sizeof is a unary
    ; operator but we only support it with parens and a type so we parse it
    ; here instead.)
    imw r1 ^str_sizeof
    add r1 r1 rpp
    call ^strcmp
    jz r0 &parse_function_call_sizeof

    ; find the function, make sure it exists
    ldw r0 rfp -4
    add r1 rfp -16
    call ^globals_find_function
    jz r0 &parse_function_call_not_found
    stw r0 rfp -8

    ; TODO: We're not currently bothering to check function argument types yet.
    ; We don't actually need to do any casts ourselves because the function
    ; preamble moves the arguments into local variables, so it will correctly
    ; use ldb/stb for any arguments of type char. We should eventually do this
    ; though.

    ; parse the arguments
    call ^parse_function_call_arguments
    stw r0 rfp -12

    ; make sure the argument count matches
    ldw r0 rfp -12
    ldw r1 rfp -16
    sub r0 r0 r1
    jnz r0 &parse_function_call_wrong_number_of_args

    ; compile it
    ldw r0 rfp -4
    ldw r1 rfp -12
    call ^compile_function_call

    ; return the return type
    ldw r0 rfp -8
    leave
    ret

:parse_function_call_sizeof
    ; it's sizeof. clean up and jump out.
    ldw r0 rfp -4
    call ^free
    leave
    jmp ^parse_sizeof

:parse_function_call_wrong_number_of_args
    ; fatal, wrong number of args
    imw r0 ^error_wrong_number_of_args
    add r0 r0 rpp
    call ^fatal

:parse_function_call_not_found
    ; fatal, no such function
    imw r0 ^error_no_such_function
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; char parse_string_literal(void);
; ==========================================================

=parse_string_literal

    ; check if we have room
    imw r0 ^strings_count
    ldw r0 rpp r0
    imw r1 ^strings_capacity
    ldw r1 rpp r1
    sub r1 r0 r1
    jz r1 &parse_string_literal_too_many_strings

    ; store it in the strings array
    push r0
    call ^lexer_take
    pop r1
    imw r9 ^strings
    ldw r9 rpp r9
    shl r2 r1 2
    stw r0 r9 r2

    ; compile it
    imw r0 ^strings_emitted
    ldw r0 rpp r0
    add r0 r0 r1
    call ^compile_string_literal_invocation

    ; bump the string count
    imw r1 ^strings_count
    ldw r0 rpp r1
    inc r0
    stw r0 rpp r1

    ; return the type
    mov r0 0x21  ; char*
    ret

:parse_string_literal_too_many_strings
    ; fatal, too many strings
    imw r0 ^error_too_many_strings
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; char parse_primary_expression(void);
; ==========================================================

=parse_primary_expression

    ; check for a number
    imw r0 ^lexer_type
    ldw r0 rpp r0
    cmpu r0 r0 "n"   ; "n" == number
    jne r0 &parse_primary_expression_not_number

    ; it's a number. emit code to load it into r0.
    imw r0 ^lexer_token
    ldw r0 r0 rpp
    call ^compile_immediate
    call ^lexer_consume
    mov r0 0x30  ; int
    ret
:parse_primary_expression_not_number

    ; check for a character literal
    imw r0 ^lexer_type
    ldw r0 rpp r0
    cmpu r0 r0 "c"   ; "c" == character
    jne r0 &parse_primary_expression_not_character

    ; it's a character. emit code to load it into r0.
    imw r0 ^lexer_token
    ldw r0 r0 rpp
    ldb r0 r0 0
    call ^compile_character_literal
    call ^lexer_consume
    mov r0 0x30  ; int
    ret
:parse_primary_expression_not_character

    ; check for a string
    imw r0 ^lexer_type
    ldw r0 rpp r0
    cmpu r0 r0 "s"   ; "s" == string
    jne r0 &parse_primary_expression_not_string

    ; it's a string.
    jmp ^parse_string_literal
:parse_primary_expression_not_string

    ; check for parentheses
    mov r0 "("
    call ^lexer_accept_char
    jz r0 &parse_primary_expression_not_parens

    ; it's parens. check for a type; if so it's a cast.
    push 0
    mov r0 rsp
    call ^try_parse_type
    pop r1
    jz r0 &parse_primary_expression_not_cast

    ; we have a cast. parse the closing paren and the expression it's casting
    push r1
    mov r0 ")"
    call ^lexer_expect_char
    call ^parse_unary_expression

    ; dereference if needed and then cast it.
    zero r1
    call ^compile_dereference_if_lvalue
    ldb r1 rsp 0
    popd
    jmp ^compile_cast

:parse_primary_expression_not_cast
    ; otherwise it's a nested expression.
    call ^parse_expression
    push r0
    mov r0 ")"  ; closing parenthesis
    call ^lexer_expect_char
    pop r0
    ret

:parse_primary_expression_not_parens
    ; fatal, expected expression
    imw r0 ^error_expected_expression
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; char parse_postfix_expression(void);
; ==========================================================
; vars:
; - identifier: rfp-4
; - expression type: rfp-8
; ==========================================================

=parse_postfix_expression
    enter
    sub rsp rsp 8

    ; check if the token type is an identifier
    imw r0 ^lexer_type
    ldw r0 rpp r0
    sub r0 r0 "a"   ; "a" == alphanumeric
    je r0 &parse_postfix_expression_identifier

    ; if it's not an identifier, it's a primary-expression
    leave
    jmp ^parse_primary_expression

:parse_postfix_expression_identifier

    ; store the identifier
    call ^lexer_take
    stw r0 rfp -4

    ; check if we have an open parenthesis
    mov r0 "("
    call ^lexer_accept_char
    jz r0 &parse_postfix_expression_variable

    ; we have an open parenthesis. it's a function call (or builtin.)
    ldw r0 rfp -4
    call ^parse_function_call
    jmp &parse_postfix_expression_identifier_done

:parse_postfix_expression_variable
    ; no open parenthesis; it's a variable. emit code to load it (as an
    ; l-value) into r0.
    ldw r0 rfp -4
    call ^compile_load_variable

:parse_postfix_expression_identifier_done
    ; clean up
    stw r0 rfp -8
    ldw r0 rfp -4
    call ^free
    ldw r0 rfp -8
    leave
    ret


; ==========================================================
; char parse_unary_minus(void);
; ==========================================================
; Parses the unary `-` operator.
; ==========================================================

=parse_unary_minus

    ; parse the following expression, store the type
    call ^parse_unary_expression
    zero r1
    call ^compile_dereference_if_lvalue
    push r0

    ; emit negation of the value
    call ^compile_negate

    ; return the type
    ; TODO we should promote char to int
    ; TODO isn't this already happening with type_promote?
    pop r0
    jmp ^type_promote



; ==========================================================
; char parse_unary_boolean_not(void);
; ==========================================================
; Parses the unary `!` operator.
; ==========================================================

=parse_unary_boolean_not

    ; parse the following expression, store the type
    call ^parse_unary_expression
    zero r1
    call ^compile_dereference_if_lvalue
    push r0

    ; emit boolean not of the value
    call ^compile_boolean_not

    ; return the type
    ; TODO we should promote char to int
    ; TODO isn't this already happening with type_promote?
    ; TODO why do we even need to promote, compile_boolean_not returns int
    pop r0
    jmp ^type_promote



; ==========================================================
; char parse_unary_bitwise_not(void);
; ==========================================================
; Parses the unary `!` operator.
; ==========================================================

=parse_unary_bitwise_not

    ; parse the following expression, store the type
    call ^parse_unary_expression
    zero r1
    call ^compile_dereference_if_lvalue
    push r0

    ; emit boolean not of the value
    call ^compile_bitwise_not

    ; return the type
    ; TODO we should promote char to int
    pop r0
    jmp ^type_promote



; ==========================================================
; char parse_unary_dereference(void);
; ==========================================================
; Parses the unary `*` operator.
; ==========================================================

=parse_unary_dereference

    ; parse the following expression
    call ^parse_unary_expression

    ; make sure the type is a pointer (non-zero indirection count)
    and r1 r0 0xF
    jz r1 &parse_unary_dereference_not_pointer

    ; check the l-value flag
    and r1 r0 0x40   ; l-value flag
    jz r1 &parse_unary_dereference_not_lvalue

    ; it's already an l-value. emit code to dereference it now.
    push r0
    and r0 r0 0x3F
    zero r1
    call ^compile_dereference
    pop r0
    dec r0
    ret

:parse_unary_dereference_not_lvalue
    ; it's not an l-value. make it an l-value and decrement the indirection
    ; count. (l-values are dereferenced as needed.)
    or r0 r0 0x40   ; l-value flag
    dec r0
    ret

:parse_unary_dereference_not_pointer
    ; fatal, type to dereference is not a pointer
    imw r0 ^error_type_to_dereference_not_pointer
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; char parse_unary_reference(void);
; ==========================================================
; Parses the unary `&` operator.
; ==========================================================

=parse_unary_reference

    ; parse the following expression
    ; (there's no possible unary operator that could follow & but we parse a
    ; unary expression anyway so that we get a nice error message ("cannot
    ; take address of r-value") rather than a confusing parse error.)
    call ^parse_unary_expression

    ; the type must be an l-value.
    and r1 r0 0x40   ; l-value flag
    jz r1 &parse_unary_reference_not_lvalue

    ; it also cannot have too many indirections
    and r1 r0 0xF
    sub r1 r1 0xF
    jz r1 &parse_unary_reference_too_many_indirections

    ; remove the l-value flag and increment the indirection count
    and r0 r0 0x3F
    inc r0
    ret

:parse_unary_reference_not_lvalue
    ; fatal, cannot take address of r-value
    imw r0 ^error_cannot_take_address_of_rvalue
    add r0 r0 rpp
    call ^fatal

:parse_unary_reference_too_many_indirections
    ; fatal, too many indirections
    imw r0 ^error_too_many_indirections
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; char parse_unary_expression(void);
; ==========================================================
; Parses a unary expression.
; ==========================================================

=parse_unary_expression

    ; TODO all of these functions recurse into parse_unary_expression first
    ; since all unary operators are right-associative. we could probably
    ; simplify this a lot by doing parse_unary_expression here.

    ; check for unary +. it does nothing.
    ; TODO actually it shouldn't do nothing, it should remove l-value and promote char to int
    mov r0 "+"
    call ^lexer_accept_char
    jz r0 &parse_unary_expression_not_plus
    jmp ^parse_unary_expression
:parse_unary_expression_not_plus

    ; check for unary -, negate
    mov r0 "-"
    call ^lexer_accept_char
    jz r0 &parse_unary_expression_not_minus
    jmp ^parse_unary_minus
:parse_unary_expression_not_minus

    ; check for unary !, boolean not
    mov r0 "!"
    call ^lexer_accept_char
    jz r0 &parse_unary_expression_not_boolean_not
    jmp ^parse_unary_boolean_not
:parse_unary_expression_not_boolean_not

    ; check for unary ~, bitwise not
    mov r0 "~"
    call ^lexer_accept_char
    jz r0 &parse_unary_expression_not_bitwise_not
    jmp ^parse_unary_bitwise_not
:parse_unary_expression_not_bitwise_not

    ; check for unary *, dereference
    mov r0 "*"
    call ^lexer_accept_char
    jz r0 &parse_unary_expression_not_dereference
    jmp ^parse_unary_dereference
:parse_unary_expression_not_dereference

    ; check for unary &, reference
    mov r0 "&"
    call ^lexer_accept_char
    jz r0 &parse_unary_expression_not_reference
    jmp ^parse_unary_reference
:parse_unary_expression_not_reference

    ; we don't check sizeof here, but rather in postfix expression; see note
    ; about sizeof above.

    ; no unary operator
    jmp ^parse_postfix_expression



; ==========================================================
; bool parse_token_is_binary_op(void);
; ==========================================================
; Returns true if the current token is a binary operator, false otherwise.
;
; vars:
; - lexer_token: rfp-4
; ==========================================================

=parse_token_is_binary_op
    enter

    ; the token type must be punctuation
    imw r0 ^lexer_type
    ldw r0 rpp r0
    sub r0 r0 "p"  ; "p" == punctuation
    jnz r0 &parse_token_is_binary_op_false

    ; For now we don't bother with a hashtable or anything. We just strcmp()
    ; each operator string.

    ; get the current token
    imw r0 ^lexer_token
    ldw r0 r0 rpp
    push r0

    ; check =
    imw r0 ^str_op_assign ; "="
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    je r0 &parse_token_is_binary_op_true

    ; check ==
    imw r0 ^str_op_eq ; "=="
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    je r0 &parse_token_is_binary_op_true

    ; check !=
    imw r0 ^str_op_ne ; "!="
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    je r0 &parse_token_is_binary_op_true

    ; check <
    imw r0 ^str_op_lt ; "<"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    je r0 &parse_token_is_binary_op_true

    ; check <=
    imw r0 ^str_op_le ; "<="
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    je r0 &parse_token_is_binary_op_true

    ; check >
    imw r0 ^str_op_gt ; ">"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    je r0 &parse_token_is_binary_op_true

    ; check >=
    imw r0 ^str_op_ge ; ">="
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    je r0 &parse_token_is_binary_op_true

    ; check +
    imw r0 ^str_op_add ; "+"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    je r0 &parse_token_is_binary_op_true

    ; check -
    imw r0 ^str_op_sub ; "-"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    je r0 &parse_token_is_binary_op_true

    ; check *
    imw r0 ^str_op_mul ; "*"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    je r0 &parse_token_is_binary_op_true

    ; check /
    imw r0 ^str_op_div ; "/"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    je r0 &parse_token_is_binary_op_true

    ; check %
    imw r0 ^str_op_mod ; "%"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    je r0 &parse_token_is_binary_op_true

    ; check &
    imw r0 ^str_op_and ; "&"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    je r0 &parse_token_is_binary_op_true

    ; check |
    imw r0 ^str_op_or ; "|"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    je r0 &parse_token_is_binary_op_true

    ; check ^
    imw r0 ^str_op_xor ; "^"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    je r0 &parse_token_is_binary_op_true

    ; check <<
    imw r0 ^str_op_shl ; "<<"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    je r0 &parse_token_is_binary_op_true

    ; check >>
    imw r0 ^str_op_shr ; ">>"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    je r0 &parse_token_is_binary_op_true

:parse_token_is_binary_op_false
    ; return false
    zero r0
    leave
    ret

:parse_token_is_binary_op_true
    ; return true
    mov r0 1
    leave
    ret



; ==========================================================
; static void parse_binary_expression(void);
; ==========================================================
; Parses a binary expression.
; ==========================================================

@parse_binary_expression

    ; parse the left-hand side, store the type
    call ^parse_unary_expression
    push r0

    ; check if we have a binary operator
    call ^parse_token_is_binary_op
    jnz r0 &parse_binary_expression_found

    ; not a binary expression; just return the left-hand side
    pop r0
    ret
:parse_binary_expression_found

    ; store the binary operator
    call ^lexer_take
    push r0

    ; emit a push of the left-hand value
    call ^compile_push_r0

    ; parse the right-hand side, store the type
    call ^parse_unary_expression
    push r0

    ; emit a pop of the left-hand value into r1
    mov r0 1
    call ^compile_pop

    ; compile the binary operator
    pop r2   ; type of right-hand side
    pop r0   ; binary operator
    pop r1   ; type of left-hand side
    call ^compile_binary_op
    push r0

    ; Check for another binary operator. (We don't support this but we want a
    ; nice error message when we forget all the required parens.)
    call ^parse_token_is_binary_op
    jnz r0 &parse_binary_expression_multiple

    ; done
    pop r0
    ret

:parse_binary_expression_multiple
    ; fatal, multiple binary operators not allowed
    imw r0 ^error_multiple_binary_ops
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; void parse_expression(void);
; ==========================================================
; Parses an expression.
; ==========================================================

=parse_expression

    ; In the C grammar, `expression` is actually a comma operator expression.
    ; It's separate from a binary expression because it can't appear as the
    ; argument to a function. We don't support comma expressions so we do
    ; nothing here, but we keep them separate for a bit of consistency with the
    ; C grammar.

    jmp ^parse_binary_expression



; ==========================================================
; void parse_statement(void);
; ==========================================================

=parse_statement

    ; check for a type. if so, it's a local variable definition.
    sub rsp rsp 4
    mov r0 rsp
    call ^try_parse_type
    jz r0 &parse_statement_not_type
    pop r0
    jmp ^parse_local_variable_definition
:parse_statement_not_type
    popd

    ; check for a semicolon. if so, it's an empty statement.
    mov r0 ";"
    call ^lexer_accept_char
    jz r0 &parse_statement_not_empty
    ret
:parse_statement_not_empty

    ; check for an opening brace. if so, it's a nested block.
    mov r0 "{"
    call ^lexer_accept_char
    jz r0 &parse_statement_not_block
    jmp ^parse_block
:parse_statement_not_block

    ; check for `if`
    imw r0 ^str_if
    add r0 r0 rpp
    call ^lexer_accept
    jz r0 &parse_statement_not_if
    jmp ^parse_if
:parse_statement_not_if

    ; check for `while`
    imw r0 ^str_while
    add r0 r0 rpp
    call ^lexer_accept
    jz r0 &parse_statement_not_while
    jmp ^parse_while
:parse_statement_not_while

    ; check for `break`
    imw r0 ^str_break
    add r0 r0 rpp
    call ^lexer_accept
    jz r0 &parse_statement_not_break
    jmp ^parse_break
:parse_statement_not_break

    ; check for `continue`
    imw r0 ^str_continue
    add r0 r0 rpp
    call ^lexer_accept
    jz r0 &parse_statement_not_continue
    jmp ^parse_continue
:parse_statement_not_continue

    ; check for `return`
    imw r0 ^str_return
    add r0 r0 rpp
    call ^lexer_accept
    jz r0 &parse_statement_not_return
    jmp ^parse_return
:parse_statement_not_return

    ; None of the above. It must be an expression followed by a semicolon. We
    ; discard the result of the expression.
    call ^parse_expression
    mov r0 ";"
    jmp ^lexer_expect_char



; ==========================================================
; void parse_block(void);
; ==========================================================
; Parses a block (called a "compound statement" in the C spec.)
;
; The opening brace has already been consumed.
;
; vars:
; - previous_locals_count: rfp-4
; ==========================================================

@parse_block
    enter

    ; store the local variable count at the start of this block so we can
    ; restore it later
    imw r0 ^locals_count
    ldw r0 rpp r0
    push r0

:parse_block_loop

    ; check for a closing brace
    mov r0 "}"
    call ^lexer_accept_char
    jnz r0 &parse_block_loop_done

    ; parse one statement
    call ^parse_statement

    ; loop
    jmp &parse_block_loop

:parse_block_loop_done

    ; check if the frame size has grown, and if so, update it
    imw r0 ^locals_count
    ldw r0 rpp r0
    shl r0 r0 2
    imw r2 ^frame_size
    ldw r1 rpp r2
    cmpu r1 r0 r1
    jle r1 &parse_block_frame_size
    stw r0 rpp r2
:parse_block_frame_size

    ; destroy any local variables defined in this block
    ldw r0 rfp -4
    call ^locals_pop

    leave
    ret



; ==========================================================
; void parse_compile_function_strings(void);
; ==========================================================
; Emits all strings collected during the parsing of a function.
;
; vars:
; - index: rfp-4
; - string: rfp-8
; ==========================================================

=parse_compile_function_strings
    enter
    push 0
    push 0

:parse_function_strings_loop

    ; check if we're done
    ldw r0 rfp -4
    imw r1 ^strings_count
    ldw r1 rpp r1
    sub r1 r0 r1
    jz r1 &parse_function_strings_done

    ; get a string
    imw r1 ^strings
    ldw r1 rpp r1
    shl r2 r0 2
    ldw r1 r1 r2
    stw r1 rfp -8

    ; emit it
    imw r9 ^strings_emitted
    ldw r2 rpp r9
    add r0 r0 r2
    call ^compile_string_literal_definition

    ; free it
    ldw r1 rfp -8
    call ^free

    ; next
    ldw r0 rfp -4
    inc r0
    stw r0 rfp -4
    jmp &parse_function_strings_loop

:parse_function_strings_done

    ; clear strings
    imw r1 ^strings_count
    ldw r0 rpp r1
    stw 0 rpp r1

    ; increment strings_emitted
    imw r9 ^strings_emitted
    ldw r1 rpp r9
    add r0 r0 r1
    stw r0 rpp r9

    leave
    ret



; ==========================================================
; static void parse_function_definition(char return_type, char* name,
;                                       bool is_static, int arg_count);
; ==========================================================
; Parses a function definition.
;
; The opening brace has already been consumed.
;
; vars:
; - return_type: rfp-4
; - name: rfp-8
; - is_static: rfp-12
; - arg_count: rfp-16
; ==========================================================

@parse_function_definition
    enter

    ; store args and initial vars
    push r0
    push r1
    push r2
    push r3

    ; open the function (emitting the preamble)
    ldw r0 rfp -8
    ldw r1 rfp -16
    call ^compile_function_open

    ; parse and compile the contents
    call ^parse_block

    ; get the frame size and clear it for the next function
    imw r9 ^frame_size
    ldw r3 rpp r9
    stw 0 rpp r9

    ; close the function (emitting the epilogue and trampoline)
    ldw r0 rfp -8
    ldw r1 rfp -12
    ldw r2 rfp -16
    call ^compile_function_close

    ; compile the string literals collected in the function
    call ^parse_compile_function_strings
    call ^compile_global_divider

    ; done
    leave
    ret



; ==========================================================
; static void parse_function_declaration(char return_type, char* name, bool is_static);
; ==========================================================
; Parses a function declaration.
;
; This parses the argument list and adds the variables to locals. If the
; declaration is then followed by a definition, we hand control over to
; parse_function_definition().
;
; (Although adding the variables to locals is unnecessary if this is just a
; declaration, adding them as we go means we don't need to store them.)
;
; The opening parenthesis has already been consumed.
;
; vars:
; - return_type: rfp-4
; - name: rfp-8
; - is_static: rfp-12
; - argument_count: rfp-16
; ==========================================================

@parse_function_declaration
    enter

    ; store args and initial vars
    push r0
    push r1
    push r2
    push 0

:parse_function_declaration_loop

    ; parse the first argument type, store it
    call ^parse_type
    push r0

    ; check if the type is void and this is the first argument. (we're looking
    ; for a `(void)` argument list.)
    cmpu r0 r0 0x10    ; 0x10 == void
    jne r0 &parse_function_declaration_not_void
    ldw r0 rfp -16     ; argument_count
    jnz r0 &parse_function_declaration_not_void

    ; the first argument is void. check again for a closing parenthesis.
    mov r0 ")"
    call ^lexer_accept_char
    jnz r0 &parse_function_declaration_args_done

    ; if we don't have `)`, we just keep going; we let locals_add() complain
    ; about the void variable.

:parse_function_declaration_not_void

    ; names are optional. check if we have a name
    imw r0 ^lexer_type
    ldw r0 rpp r0
    cmpu r0 r0 "a"
    jne r0 &parse_function_declaration_empty_name

    ; we have a name; take it
    call ^lexer_take
    jmp &parse_function_declaration_have_name

:parse_function_declaration_empty_name
    ; no name; make an empty string
    push 0
    mov r0 rsp
    call ^strdup
    popd

:parse_function_declaration_have_name

    ; add the variable
    mov r1 r0
    pop r0
    call ^locals_add

    ; increment the argument count
    ldw r0 rfp -16
    inc r0
    stw r0 rfp -16

    ; a closing parenthesis means the end of the argument list
    mov r0 ")"
    call ^lexer_accept_char
    jnz r0 &parse_function_declaration_args_done

    ; otherwise we need a comma to separate arguments
    mov r0 ","
    call ^lexer_expect_char

    ; we only support up to four arguments
    ldw r0 rfp -16
    sub r0 r0 4
    jz r0 &parse_function_declaration_too_many_params

    ; next loop
    jmp &parse_function_declaration_loop

:parse_function_declaration_args_done

    ; add the function
    ldw r0 rfp -4
    ldw r1 rfp -8
    ldw r2 rfp -16
    call ^globals_add_function

    ; check for a semicolon. if so, we're done.
    mov r0 ";"
    call ^lexer_accept_char
    jnz r0 &parse_function_declaration_done

    ; otherwise it must be an open brace.
    mov r0 "{"
    call ^lexer_accept_char
    jz r0 &parse_function_declaration_end_error

    ; we have a function definition. parse it
    ldw r0 rfp -4
    ldw r1 rfp -8
    ldw r2 rfp -12
    ldw r3 rfp -16
    call ^parse_function_definition

:parse_function_declaration_done

    ; free all local variables
    zero r0
    call ^locals_pop

    ; done
    leave
    ret

:parse_function_declaration_end_error
    ; fatal, expected ; or (
    imw r0 ^error_expected_semicolon_or_open_paren
    add r0 r0 rpp
    call ^fatal

:parse_function_declaration_too_many_params
    ; fatal, too many parameters
    imw r0 ^error_too_many_params
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; static void parse_global(void);
; ==========================================================
; vars:
; - is_extern: rfp-4
; - is_static: rfp-8
; - type: rfp-12
; - identifier: rfp-16
; ==========================================================

@parse_global
    enter

    ; in omC, qualifiers (typedef, extern, static) have to come first. we check
    ; for them before parsing any types.

    ; check for typedef. if so we tail-call parse_typedef() and skip the rest
    ; of this function.
    imw r0 ^str_typedef
    add r0 r0 rpp
    call ^lexer_accept
    jz r0 &parse_global_not_typedef
    leave
    jmp ^parse_typedef
:parse_global_not_typedef

    ; otherwise we have a declaration or definition of a variable or function.

    ; check for extern
    imw r0 ^str_extern
    add r0 r0 rpp
    call ^lexer_accept
    push r0

    ; check for static
    imw r0 ^str_static
    add r0 r0 rpp
    call ^lexer_accept
    push r0

    ; parse a type
    call ^parse_type
    push r0

    ; parse a name
    call ^parse_alphanumeric
    push r0

    ; check for a semicolon. (we don't consume it right away so our line
    ; numbers for emitted variables are correct.)
    push ";"
    mov r0 rsp
    call ^lexer_is
    popd
    jz r0 &parse_global_not_variable

    ; a semicolon means it's a variable: call parse_global_variable()
    ldw r0 rfp -12   ; type
    ldw r1 rfp -16   ; name
    ldw r2 rfp -4    ; is_extern
    ldw r3 rfp -8    ; is_static
    call ^parse_global_variable

    ; consume the semicolon, we're done
    call ^lexer_consume
    leave
    ret
:parse_global_not_variable

    ; check for an open parenthesis
    mov r0 "("
    call ^lexer_accept_char
    jz r0 &parse_global_not_function

    ; an open parenthesis means it's a function declaration.
    ; tail-call parse_function_declaration()
    ldw r0 rfp -12   ; type
    ldw r1 rfp -16   ; name
    ldw r2 rfp -8    ; is_static
    leave
    jmp ^parse_function_declaration

:parse_global_not_function
    ; fatal, expected ; or (
    imw r0 ^error_expected_semicolon_or_open_paren
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; void parse(void);
; ==========================================================

=parse
    enter

:parse_loop

    ; if the token type is "e" (end-of-file), return
    imw r0 ^lexer_type
    ldw r0 rpp r0
    cmpu r0 r0 "e"   ; "e" == end-of-file
    je r0 &parse_ret

    ; parse one global declaration or definition
    call ^parse_global

    ; loop
    jmp &parse_loop

:parse_ret
    leave
    ret
