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



; Files

=input_file
    0

=output_file
    0



; Error messages

=error_usage
    "Invalid arguments." '0A
    "Usage: <cci> <input> -o <output>" '00

=error_input
    "Failed to read from input file." '00

=error_output
    "Failed to read from output file." '00

;=error_not_yet_implemented
;    "Not yet implemented." '00

=error_unexpected_token
    "Unexpected token." '00

=error_unexpected_character
    "Unexpected character." '00

=error_identifier_too_long
    "Identifier is too long." '00

=error_number_too_long
    "Number is too long." '00

=error_char_literal_count
    "Character literal must contain exactly one character." '00

=error_literal_too_long
    "String literal is too long." '00

=error_literal_truncated
    "String or character literal is truncated." '00

=error_unsupported_escape
    "Unrecognized or unsupported escape sequence." '00

=error_expected_semicolon_or_open_paren
    "Expected `;` or `(`." '00

=error_expected_type
    "Expected type." '00

=error_expected_identifier
    "Expected identifier." '00

=error_expected_alphanumeric
    "Expected an identifier or keyword." '00

=error_expected_expression
    "Expected expression." '00

=error_too_many_indirections
    "Too many indirections." '00

=error_too_many_typedefs
    "Too many typedefs." '00

=error_duplicate_typedef
    "Duplicate typedef." '00

=error_duplicate_global
    "Duplicate definition at file scope." '00

=error_type_redefined
    "Type name redefined as a different type." '00

=error_const_must_be_followed_by_type
    "`const` must be followed by a type name." '00

=error_void_variable
    "`void` type variables are not allowed." '00

=error_too_many_params
    "Too many parameters. Only 4 function parameters are supported." '00

=error_too_many_args
    "Too many arguments. Only 4 function arguments are supported." '00

=error_break_continue
    "`break` and `continue` can only be used in a `while` loop." '00

=error_multiple_binary_ops
    "Multiple binary operators are not allowed in an expression. Add parentheses." '00

=error_cannot_take_address_of_rvalue
    "Cannot take the address of an r-value" '00

=error_type_to_dereference_not_pointer
    "Type to dereference is not a pointer" '00

=error_preprocessor_directive
    "Unrecognized preprocessor directive." '00

=error_wrong_number_of_args
    "Wrong number of arguments to function call." '00

=error_no_such_function
    "Called function was not declared." '00

=error_no_such_variable
    "Variable not found." '00

=error_too_many_strings
    "Too many strings in this function" '00

=error_assign_not_lvalue
    "Assignment location is not an lvalue." '00

=error_internal
    "Internal compiler error." '00

=error_arithmetic_void
    "Cannot perform arithmetic on value of `void` type." '00

=error_arithmetic_voidp
    "Cannot perform arithmetic on value of `void*` type." '00

=error_arithmetic_different_pointers
    "Cannot perform arithmetic on two pointers of different types." '00

=error_cast_void
    "Casting to `void` is not allowed." '00



; Keyword strings

=str_typedef "typedef" '00
=str_static "static" '00
=str_extern "extern" '00
=str_const "const" '00
=str_int "int" '00
=str_char "char" '00
=str_void "void" '00
=str_if "if" '00
=str_while "while" '00
=str_break "break" '00
=str_continue "continue" '00
=str_return "return" '00
=str_sizeof "sizeof" '00



; Preprocessor directive strings

=str_line "line" '00
=str_pragma "pragma" '00



; Operator strings

=str_op_assign "=" '00
=str_op_eq "==" '00
=str_op_ne "!=" '00
=str_op_lt "<" '00
=str_op_le "<=" '00
=str_op_gt ">" '00
=str_op_ge ">=" '00
=str_op_add "+" '00
=str_op_sub "-" '00
=str_op_mul "*" '00
=str_op_div "/" '00
=str_op_mod "%" '00
=str_op_and "&" '00
=str_op_or "|" '00
=str_op_xor "^" '00
=str_op_shl "<<" '00
=str_op_shr ">>" '00



; Instruction strings

=str_add "add" '00
=str_sub "sub" '00
=str_mul "mul" '00
=str_divs "divs" '00
=str_mods "mods" '00
=str_zero "zero" '00
=str_inc "inc" '00
=str_dec "dec" '00
=str_mov "mov" '00

=str_and "and" '00
=str_or "or" '00
=str_shl "shl" '00
=str_shru "shru" '00
=str_shrs "shrs" '00
=str_xor "xor" '00
=str_ror "ror" '00
=str_rol "ror" '00
=str_not "not" '00

=str_ldw "ldw" '00
=str_stw "stw" '00
=str_ldb "ldb" '00
=str_stb "stb" '00

=str_ims "ims" '00
=str_imw "imw" '00

=str_cmpu "cmpu" '00
=str_cmps "cmps" '00
=str_jz "jz" '00
=str_jnz "jnz" '00
=str_je "je" '00
=str_jne "jne" '00
=str_jl "jl" '00
=str_jle "jle" '00
=str_jg "jg" '00
=str_jge "jge" '00
=str_jmp "jmp" '00

=str_enter "enter" '00
=str_leave "leave" '00
=str_call "call" '00
=str_ret "ret" '00

=str_push "push" '00
=str_pop "pop" '00
=str_popd "popd" '00



; register strings

=str_rsp "rsp" '00
=str_rfp "rfp" '00
=str_rpp "rpp" '00
=str_rip "rip" '00


; misc strings

=str_function_prefix "_F_" '00
=str_string_prefix "_Sx" '00
=str_label_prefix "_Lx" '00
=str_hex_prefix "0x" '00
