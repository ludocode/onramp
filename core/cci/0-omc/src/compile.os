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
; void compile_init(void);
; ==========================================================

=compile_init
    ret



; ==========================================================
; void compile_destroy(void);
; ==========================================================

=compile_destroy
    ret



; ==========================================================
; void compile_global_variable_definition(bool is_static, char* name);
; ==========================================================

=compile_global_variable_definition

    ; our symbol definition should be @ for static and = for extern
    jz r0 &compile_global_variable_extern
    mov r0 "@"
    jmp &compile_global_variable_static
:compile_global_variable_extern
    mov r0 "="
:compile_global_variable_static

    ; emit it
    call ^emit_label
    call ^emit_newline
    call ^emit_zero
    call ^emit_newline
    jmp ^emit_newline



; ==========================================================
; void compile_function_open(const char* name, int arg_count);
; ==========================================================
; Emits the function wrapper symbol, then emits code to move all function
; arguments into their proper positions in the stack frame.
;
; Note that we don't emit the real function prologue here. It's emitted at the
; end, in compile_function_close(). See the README for details.
;
; vars:
; - arg_count: rfp-4
; - index: rfp-8
; ==========================================================

=compile_function_open
    enter

    ; setup vars
    push r1
    push 0

    ; emit the linker directive with _F_ prefix
    ;     emit_prefixed_linker_string('@', "_F_", name);
    mov r2 r0
    mov r0 "@"
    imw r1 ^str_function_prefix
    add r1 rpp r1
    call ^emit_prefixed_linker_string
    call ^emit_newline

    ; the rest of this function emits instructions to move each argument into
    ; the stack frame.

:compile_function_open_loop

    ; if index == arg_count, we're done
    ldw r0 rfp -4
    ldw r1 rfp -8
    sub r0 r0 r1
    jz r0 &compile_function_open_done

    ; emit "stw"
    imw r0 ^str_stw
    add r0 rpp r0
    call ^emit_term

    ; emit the register number
    ldw r0 rfp -8
    call ^emit_register

    ; emit "rfp"
    imw r0 ^str_rfp
    add r0 rpp r0
    call ^emit_term

    ; emit address in frame
    ldw r0 rfp -8
    shl r0 r0 2
    sub r0 -4 r0
    call ^emit_int
    call ^emit_newline

    ; next
    ldw r0 rfp -8
    inc r0
    stw r0 rfp -8
    jmp &compile_function_open_loop

:compile_function_open_done
    leave
    ret



; ==========================================================
; void compile_function_close(const char* name, bool is_static,
;                             int arg_count, int frame_size);
; ==========================================================
; Compiles the epilogue of the current function, followed by a trampoline with
; the prologue of the function.
;
; We insert code at the end of every function to return 0. main() needs to
; return 0 if execution reaches the end of the function without a return
; statement (provided it's declared with a return type of int.) We do this for
; all functions for simplicity.
;
; We then emit the trampoline which sets up the function's stack frame. See the
; README for details.
;
; vars:
; - name: rfp-4
; - is_static: rfp-8
; - arg_count: rfp-12
; - frame_size: rfp-16
; ==========================================================

=compile_function_close
    enter

    push r0
    push r1
    push r2
    push r3

    ; emit "zero r0"
    imw r0 ^str_zero
    add r0 rpp r0
    call ^emit_term
    zero r0
    call ^emit_register
    call ^emit_newline

    ; emit "leave"
    imw r0 ^str_leave
    add r0 rpp r0
    call ^emit_term
    call ^emit_newline

    ; emit "ret"
    imw r0 ^str_ret
    add r0 rpp r0
    call ^emit_term
    call ^emit_newline
    call ^emit_newline

    ; get character for symbol linkage: "=" for extern and "@" for static
    ldw r0 rfp -8
    jz r0 &compile_function_close_extern
    mov r0 "@"
    jmp &compile_function_close_static
:compile_function_close_extern
    mov r0 "="
:compile_function_close_static

    ; emit the symbol definition
    ldw r1 rfp -4
    call ^emit_label
    call ^emit_newline

    ; emit "enter"
    imw r0 ^str_enter
    add r0 rpp r0
    call ^emit_term
    call ^emit_newline

    ; Set up the stack frame (now that we know its size)
    ; We use a temporary register in case it doesn't fit in a mix-type byte.
    ; Note that r0-r3 are used as function arguments.

    ; emit "imw r9 <framesize>"
    imw r0 ^str_imw
    add r0 rpp r0
    call ^emit_term
    mov r0 9
    call ^emit_register
    ldw r0 rfp -16
    call ^emit_int
    call ^emit_newline

    ; emit "sub rsp rsp r9"
    imw r0 ^str_sub
    add r0 rpp r0
    call ^emit_term
    imw r0 ^str_rsp
    add r0 rpp r0
    call ^emit_term
    imw r0 ^str_rsp
    add r0 rpp r0
    call ^emit_term
    mov r0 9
    call ^emit_register
    call ^emit_newline

    ; emit "jmp ^_F_name", a jump to the top of the function
    imw r0 ^str_jmp
    add r0 rpp r0
    call ^emit_term
    mov r0 "^"
    imw r1 ^str_function_prefix
    add r1 rpp r1
    ldw r2 rfp -4
    call ^emit_prefixed_linker_string
    call ^emit_newline
    call ^emit_newline

    leave
    ret



; ==========================================================
; void compile_function_call(const char* name, int arg_count)
; ==========================================================
; vars:
; - name: rfp-4
; - arg_count: rfp-8
; ==========================================================

=compile_function_call
    enter
    push r0
    push r1

    ; the arguments are on the stack. emit code to pop them into registers.
:compile_function_call_loop

    ; decrement arg_count
    ldw r1 rfp -8
    jz r1 &compile_function_call_break
    dec r1
    stw r1 rfp -8

    ; emit "pop r<arg_count>"
    imw r0 ^str_pop
    add r0 rpp r0
    call ^emit_term
    ldw r0 rfp -8
    call ^emit_register
    call ^emit_newline

    jmp &compile_function_call_loop
:compile_function_call_break

    ; emit "call ^name"
    imw r0 ^str_call
    add r0 rpp r0
    call ^emit_term
    mov r0 "^"
    ldw r1 rfp -4
    call ^emit_label
    call ^emit_newline

    leave
    ret



; ==========================================================
; void compile_label(int label);
; ==========================================================
; Compiles a label definition with a generated name.
; ==========================================================

=compile_label
    push r0

    ; emit the label definition character
    mov r0 ":"
    call ^emit_char

    ; emit the prefix "_Lx"
    imw r1 ^str_label_prefix
    add r0 rpp r1
    call ^emit_string

    ; emit the label number in hex
    pop r0
    call ^emit_hex_number

    ; done
    jmp ^emit_newline



; ==========================================================
; void compile_jump(int label);
; ==========================================================
; Compiles a jump to a label with a generated name.
; ==========================================================

=compile_jump
    push r0

    ; emit "jmp"
    imw r0 ^str_jmp
    add r0 rpp r0
    call ^emit_term

    ; emit "&_Lx#"
    mov r0 "&"
    imw r1 ^str_label_prefix
    add r1 rpp r1
    pop r2
    call ^emit_prefixed_linker_number

    ; done
    jmp ^emit_newline



; ==========================================================
; void compile_jump_if_zero(int label);
; ==========================================================
; Compiles a jump to a generated label if the value in r0 is zero.
; ==========================================================

=compile_jump_if_zero
    push r0

    ; emit "jz"
    imw r0 ^str_jz
    add r0 rpp r0
    call ^emit_term

    ; emit "r0"
    zero r0
    call ^emit_register

    ; emit "&_Lx#"
    mov r0 "&"
    imw r1 ^str_label_prefix
    add r1 rpp r1
    pop r2
    call ^emit_prefixed_linker_number

    ; done
    jmp ^emit_newline



; ==========================================================
; void compile_push_r0(void);
; ==========================================================
; Emits "push r0".
; ==========================================================

=compile_push_r0

    ; emit "push"
    imw r0 ^str_push
    add r0 rpp r0
    call ^emit_term

    ; emit r0
    zero r0
    call ^emit_register

    jmp ^emit_newline



; ==========================================================
; void compile_pop(int register);
; ==========================================================
; Emits "pop" and the given numbered register.
; ==========================================================

=compile_pop

    ; store the register
    push r0

    ; emit "pop"
    imw r0 ^str_pop
    add r0 rpp r0
    call ^emit_term

    ; emit the register
    pop r0
    call ^emit_register

    jmp ^emit_newline



; ==========================================================
; void compile_zero(void);
; ==========================================================
; Emits "zero r0", i.e. as though the current expression returned zero.
; ==========================================================

=compile_zero

    ; emit "zero"
    imw r0 ^str_zero
    add r0 rpp r0
    call ^emit_term

    ; emit r0
    zero r0
    call ^emit_register

    jmp ^emit_newline



; ==========================================================
; void compile_return(void);
; ==========================================================

=compile_return

    ; emit "leave"
    imw r0 ^str_leave
    add r0 rpp r0
    call ^emit_term
    call ^emit_newline

    ; emit "ret"
    imw r0 ^str_ret
    add r0 rpp r0
    call ^emit_term
    jmp ^emit_newline



; ==========================================================
; void compile_negate(void)
; ==========================================================
; Emits code to negate the value in r0.
; ==========================================================

=compile_negate

    ; emit "sub r0 0 r0"

    imw r0 ^str_sub
    add r0 rpp r0
    call ^emit_term

    zero r0
    call ^emit_register

    call ^emit_zero

    zero r0
    call ^emit_register

    jmp ^emit_newline



; ==========================================================
; char compile_bool(void)
; ==========================================================
; Emits code to convert the value in r0 to boolean. (If it is non-zero, it is
; replaced with 1.)
;
; Returns the type int.
; ==========================================================

=compile_bool

    ; emit "bool r0 r0"
    imw r0 ^str_bool
    add r0 rpp r0
    call ^emit_term
    zero r0
    call ^emit_register
    zero r0
    call ^emit_register
    call ^emit_newline

    ; return int
    mov r0 0x30  ; int
    ret



; ==========================================================
; char compile_boolean_not(void)
; ==========================================================
; Emits code to do a boolean inversion of the value in r0. (If the value is
; non-zero, it is replaced with zero; otherwise it is replaced with 1.)
;
; Returns the type int.
; ==========================================================

=compile_boolean_not

    ; emit "isz r0 r0"
    imw r0 ^str_isz
    add r0 rpp r0
    call ^emit_term
    zero r0
    call ^emit_register
    zero r0
    call ^emit_register
    call ^emit_newline

    ; return int
    mov r0 0x30  ; int
    ret



; ==========================================================
; char compile_bitwise_not(void)
; ==========================================================
; Emits code to do a bitwise inversion of the value in r0.
; ==========================================================

=compile_bitwise_not

    ; emit "not r0 r0"
    imw r0 ^str_not
    add r0 rpp r0
    call ^emit_term
    zero r0
    call ^emit_register
    zero r0
    call ^emit_register
    call ^emit_newline

    ; return int
    mov r0 0x30  ; int
    ret



; ==========================================================
; void compile_immediate(const char* number);
; ==========================================================
; Emits code to load the given number into register r0.
; ==========================================================

=compile_immediate
    push r0

    ; emit "imw"
    imw r0 ^str_imw
    add r0 rpp r0
    call ^emit_term

    ; emit r0
    zero r0
    call ^emit_register

    ; emit the number
    pop r0
    call ^emit_term

    jmp ^emit_newline



; ==========================================================
; void compile_character_literal(char character);
; ==========================================================
; Emits code to load the given character into register r0.
; ==========================================================

=compile_character_literal
    push r0

    ; emit "mov"
    imw r0 ^str_mov
    add r0 rpp r0
    call ^emit_term

    ; emit r0
    zero r0
    call ^emit_register

    ; emit the character
    pop r0
    call ^emit_character_literal

    jmp ^emit_newline



; ==========================================================
; void compile_string_literal_invocation(int index);
; ==========================================================
; Emits code to load the address of the string literal with the given index
; into register r0.
; ==========================================================

=compile_string_literal_invocation
    push r0

    ; emit "imw r0"
    imw r0 ^str_imw
    add r0 rpp r0
    call ^emit_term
    zero r0
    call ^emit_register

    ; emit string symbol name
    mov r0 "^"
    call ^emit_char
    imw r0 ^str_string_prefix
    add r0 rpp r0
    call ^emit_string
    pop r0
    call ^emit_hex_number
    call ^emit_newline

    ; emit "add r0 rpp r0"
    imw r0 ^str_add
    add r0 rpp r0
    call ^emit_term
    mov r0 0
    call ^emit_register
    imw r0 ^str_rpp
    add r0 rpp r0
    call ^emit_term
    mov r0 0
    call ^emit_register
    call ^emit_newline

    ret



; ==========================================================
; void compile_string_literal_definition(int index, const char* str);
; ==========================================================

=compile_string_literal_definition
    push r1
    push r0

    ; emit string symbol name
    mov r0 "@"
    call ^emit_char
    imw r0 ^str_string_prefix
    add r0 rpp r0
    call ^emit_string
    pop r0
    call ^emit_hex_number
    call ^emit_newline

    ; emit string text
    pop r0
    call ^emit_string_literal
    call ^emit_newline
    jmp ^emit_newline




; ==========================================================
; static char compile_load_global_variable(char type, const char* name);
; ==========================================================
; Emits code to load a global variable as an l-value into r0, returning its
; type.
;
; vars:
; - name: rfp-4
; - type: rfp-8
; ==========================================================

@compile_load_global_variable
    enter
    push r1
    push r0

    ; emit "imw r0 ^name"
    imw r0 ^str_imw
    add r0 rpp r0
    call ^emit_term
    mov r0 0
    call ^emit_register
    mov r0 "^"
    ldw r1 rfp -4
    call ^emit_label
    call ^emit_newline

    ; emit "add r0 rpp r0"
    imw r0 ^str_add
    add r0 rpp r0
    call ^emit_term
    mov r0 0
    call ^emit_register
    imw r0 ^str_rpp
    add r0 rpp r0
    call ^emit_term
    mov r0 0
    call ^emit_register
    call ^emit_newline

    ; return the type with the l-value flag set
    ldw r0 rfp -8
    or r0 r0 0x40   ; l-value flag
    leave
    ret



; ==========================================================
; static char compile_load_local_variable(char type, char offset);
; ==========================================================
; Emits code to load a local variable as an l-value into r0, returning its
; type.
;
; vars:
; - type: rfp-4
; - offset: rfp-8
; ==========================================================

@compile_load_local_variable
    enter
    push r0
    push r1

    ; We don't use a mix-type byte because that would limit us to 28 locals
    ; (and we don't want to bother with different paths for small vs. large
    ; frame offset.)

    ; emit "imw r0 -<offset>"
    imw r0 ^str_imw
    add r0 rpp r0
    call ^emit_term
    mov r0 0
    call ^emit_register
    ldw r0 rfp -8
    sub r0 0 r0
    call ^emit_int
    call ^emit_newline

    ; emit "add r0 rfp r0"
    imw r0 ^str_add
    add r0 rpp r0
    call ^emit_term
    mov r0 0
    call ^emit_register
    imw r0 ^str_rfp
    add r0 rpp r0
    call ^emit_term
    mov r0 0
    call ^emit_register
    call ^emit_newline

    ; return the type with the l-value flag set
    ldw r0 rfp -4
    or r0 r0 0x40   ; l-value flag
    leave
    ret



; ==========================================================
; char compile_load_variable(const char* name);
; ==========================================================
; Emits code to load the variable with the given name as an l-value into r0,
; returning its type.
; ==========================================================

=compile_load_variable
    enter
    push r0

    ; locals shadow globals so we check locals first.

    ; check for a local with the given name
    sub rsp rsp 4
    mov r1 rsp
    call ^locals_find
    jz r0 &compile_load_variable_not_local

    ; it's a local.
    pop r1
    leave
    jmp ^compile_load_local_variable
:compile_load_variable_not_local

    ; check for a global with the given name
    ldw r0 rfp -4
    call ^globals_find_variable
    jz r0 &compile_load_variable_not_global

    ; it's a global.
    ldw r1 rfp -4
    leave
    jmp ^compile_load_global_variable
:compile_load_variable_not_global

    ; fatal, no such variable
    imw r0 ^error_no_such_variable
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; void compile_dereference(char type, int register_num);
; ==========================================================
; Emits code to dereference the value of the given type stored in the given
; register.
;
; You must have already removed an indirection or cleared the lvalue flag
; before calling this.
; ==========================================================

=compile_dereference
    push r1

    ; check if we should emit "ldw" or "ldb"
    call ^type_size
    sub r0 r0 1
    jz r0 &compile_dereference_ldb

    ; "ldw"
    imw r0 ^str_ldw
    jmp &compile_dereference_ldw
:compile_dereference_ldb
    ; "ldb"
    imw r0 ^str_ldb
:compile_dereference_ldw

    ; emit "<opcode> <reg> 0 <reg>"
    add r0 rpp r0
    call ^emit_term
    ldw r0 rsp 0
    call ^emit_register
    call ^emit_zero
    pop r0
    call ^emit_register

    jmp ^emit_newline



; ==========================================================
; char compile_dereference_if_lvalue(char type, int register_num);
; ==========================================================
; Emits code to convert the value of the given type in the given register into
; an r-value.
;
; If it's an l-value, it will be dereferenced.
;
; The type is returned without the l-value flag.
; ==========================================================

=compile_dereference_if_lvalue

    ; check the l-value flag
    and r2 r0 0x40   ; l-value flag
    jnz r2 &compile_dereference_if_lvalue_true
    ret
:compile_dereference_if_lvalue_true

    ; it's an l-value. clear the l-value flag
    and r0 r0 0x3f

    ; dereference it
    push r0
    call ^compile_dereference
    pop r0

    ret



; ==========================================================
; char compile_binary_op(char* operator, char left_type, char right_type);
; ==========================================================
; Compiles a binary operator, returning the result type.
;
; The code just emitted must have placed the left hand side in r1 and the right
; hand side in r0. The result will be left in r0 and its type is returned.
;
; vars:
; - operator: rfp-4
; - left_type: rfp-8
; - right_type: rfp-12
; ==========================================================

=compile_binary_op
    enter
    push r0
    push r1
    push r2

;; i keep having to re-add this so i'm not taking it out anymore
;; this is what printf debugging looks like in onramp assembly
;mov r0 "B"
;call ^putchar
;mov r0 " "
;call ^putchar
;ldw r0 rfp -8
;call ^putd
;mov r0 " "
;call ^putchar
;ldw r0 rfp -12
;call ^putd
;mov r0 " "
;call ^putchar
;ldw r0 rfp -4
;call ^puts

    ; As with the parser we don't bother with a hashtable or anything. We just
    ; strcmp() each operator string.

    ; the right-hand side is always an r-value.
    ldw r0 rfp -12
    zero r1
    call ^compile_dereference_if_lvalue
    stw r0 rfp -12

    ; check =
    imw r0 ^str_op_assign ; "="
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    jne r0 &compile_binary_op_not_assign

    ; it's =
    ldw r0 rfp -8
    ldw r1 rfp -12
    leave
    jmp ^compile_assign
:compile_binary_op_not_assign

    ; for all other operations the left-hand side is an r-value as well.
    ldw r0 rfp -8
    mov r1 1
    call ^compile_dereference_if_lvalue
    stw r0 rfp -8

    ; for all other operations we also promote char to int.

    ; promote left to int
    ldw r0 rfp -8
    sub r9 r0 0x20
    jnz r9 &compile_add_sub_left_not_char
    mov r1 0x30
    mov r2 1
    call ^compile_cast
    stw r0 rfp -8
:compile_add_sub_left_not_char

    ; promote right to int
    ldw r0 rfp -12
    sub r9 r0 0x20
    jnz r9 &compile_add_sub_right_not_char
    mov r1 0x30
    zero r2
    call ^compile_cast
    stw r0 rfp -12
:compile_add_sub_right_not_char

    ; finally we just check all our operators to find the right one and output
    ; the right code.

    ; check arithmetic ops
    ldw r0 rfp -4
    ldw r1 rfp -8
    ldw r2 rfp -12
    call ^compile_binary_arithmetic
    jz r0 &compile_binary_op_not_arithmetic
    leave
    ret
:compile_binary_op_not_arithmetic

    ; check bitwise ops
    ldw r0 rfp -4
    ldw r1 rfp -8
    ldw r2 rfp -12
    call ^compile_binary_bitwise
    jz r0 &compile_binary_op_not_bitwise
    leave
    ret
:compile_binary_op_not_bitwise

    ; check comparison ops
    ldw r0 rfp -4
    ldw r1 rfp -8
    ldw r2 rfp -12
    call ^compile_binary_comparison
    jz r0 &compile_binary_op_not_comparison
    leave
    ret
:compile_binary_op_not_comparison

    ; this should be unreachable.
    imw r0 ^error_internal
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; static char compile_binary_arithmetic(char* operator, char left_type, char right_type);
; ==========================================================
; Compiles a binary arithmetic operator, returning the result type.
;
; If the given operator is not arithmetic, 0 is returned.
; ==========================================================

@compile_binary_arithmetic
    enter
    push r0
    push r1
    push r2

    ; check +
    imw r0 ^str_op_add ; "+"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    jne r0 &compile_binary_arithmetic_not_add

    ; it's +
    imw r0 ^str_add ; "+"
    add r0 r0 rpp
    ldw r1 rfp -8
    ldw r2 rfp -12
    leave
    jmp ^compile_add_sub
:compile_binary_arithmetic_not_add

    ; check -
    imw r0 ^str_op_sub ; "-"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    jne r0 &compile_binary_arithmetic_not_sub

    ; it's -
    imw r0 ^str_sub ; "-"
    add r0 r0 rpp
    ldw r1 rfp -8
    ldw r2 rfp -12
    leave
    jmp ^compile_add_sub
:compile_binary_arithmetic_not_sub

    ; check *
    imw r0 ^str_op_mul ; "*"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    jne r0 &compile_binary_arithmetic_not_mul

    ; it's *
    imw r0 ^str_mul
    add r0 rpp r0
    ldw r1 rfp -8
    ldw r2 rfp -12
    leave
    jmp ^compile_basic_op
:compile_binary_arithmetic_not_mul

    ; check /
    imw r0 ^str_op_div ; "/"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    jne r0 &compile_binary_arithmetic_not_div

    ; it's /
    imw r0 ^str_divs
    add r0 rpp r0
    ldw r1 rfp -8
    ldw r2 rfp -12
    leave
    jmp ^compile_basic_op
:compile_binary_arithmetic_not_div

    ; check %
    imw r0 ^str_op_mod ; "%"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    jne r0 &compile_binary_arithmetic_not_mod

    ; it's %
    imw r0 ^str_mods
    add r0 rpp r0
    ldw r1 rfp -8
    ldw r2 rfp -12
    leave
    jmp ^compile_basic_op
:compile_binary_arithmetic_not_mod

    ; none of the above
    zero r0
    leave
    ret



; ==========================================================
; static char compile_binary_bitwise(char* operator, char left_type, char right_type);
; ==========================================================
; Compiles a binary bitwise operator.
;
; If the given operator is not bitwise, 0 is returned.
; ==========================================================

@compile_binary_bitwise
    enter
    push r0
    push r1
    push r2

    ; check |
    imw r0 ^str_op_or ; "|"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    jne r0 &compile_binary_bitwise_not_or

    ; it's |
    imw r0 ^str_or
    add r0 rpp r0
    ldw r1 rfp -8
    ldw r2 rfp -12
    leave
    jmp ^compile_basic_op
:compile_binary_bitwise_not_or

    ; check &
    imw r0 ^str_op_and ; "&"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    jne r0 &compile_binary_bitwise_not_and

    ; it's &
    imw r0 ^str_and
    add r0 rpp r0
    ldw r1 rfp -8
    ldw r2 rfp -12
    leave
    jmp ^compile_basic_op
:compile_binary_bitwise_not_and

    ; check ^
    imw r0 ^str_op_xor ; "^"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    jne r0 &compile_binary_bitwise_not_xor

    ; it's ^
    imw r0 ^str_xor
    add r0 rpp r0
    ldw r1 rfp -8
    ldw r2 rfp -12
    leave
    jmp ^compile_basic_op
:compile_binary_bitwise_not_xor

    ; check <<
    imw r0 ^str_op_shl ; "<<"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    jne r0 &compile_binary_bitwise_not_shl

    ; it's <<
    imw r0 ^str_shl
    add r0 rpp r0
    ldw r1 rfp -8
    ldw r2 rfp -12
    leave
    jmp ^compile_basic_op
:compile_binary_bitwise_not_shl

    ; check >>
    imw r0 ^str_op_shr ; ">>"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    jne r0 &compile_binary_bitwise_not_shr

    ; it's %
    imw r0 ^str_shrs  ; we only support signed
    add r0 rpp r0
    ldw r1 rfp -8
    ldw r2 rfp -12
    leave
    jmp ^compile_basic_op
:compile_binary_bitwise_not_shr

    ; none of the above
    zero r0
    leave
    ret



; ==========================================================
; static char compile_binary_comparison(char* operator, char left_type, char right_type);
; ==========================================================
; Compiles a binary comparison operator.
;
; If the given operator is not comparison, 0 is returned.
; ==========================================================

@compile_binary_comparison
    enter
    push r0
    push r1
    push r2

    ; check ==
    imw r0 ^str_op_eq ; "=="
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    jne r0 &compile_binary_comparison_not_eq

    ; it's ==
    leave
    jmp ^compile_eq
:compile_binary_comparison_not_eq

    ; check !=
    imw r0 ^str_op_ne ; "!="
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    jne r0 &compile_binary_comparison_not_ne

    ; it's !=
    leave
    jmp ^compile_ne
:compile_binary_comparison_not_ne

    ; check <
    imw r0 ^str_op_lt ; "<"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    jne r0 &compile_binary_comparison_not_lt

    ; it's <
    leave
    jmp ^compile_lt
:compile_binary_comparison_not_lt

    ; check >
    imw r0 ^str_op_gt ; ">"
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    jne r0 &compile_binary_comparison_not_gt

    ; it's >
    leave
    jmp ^compile_gt
:compile_binary_comparison_not_gt

    ; check <=
    imw r0 ^str_op_le ; "<="
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    jne r0 &compile_binary_comparison_not_le

    ; it's <=
    leave
    jmp ^compile_le
:compile_binary_comparison_not_le

    ; check >=
    imw r0 ^str_op_ge ; ">="
    add r0 r0 rpp
    ldw r1 rfp -4
    call ^strcmp
    jne r0 &compile_binary_comparison_not_ge

    ; it's >=
    leave
    jmp ^compile_ge
:compile_binary_comparison_not_ge

    ; none of the above
    zero r0
    leave
    ret



; ==========================================================
; static void compile_assign(char left_type, char right_type);
; ==========================================================
; vars:
; - left_type: rfp-4
; - right_type: rfp-8
; ==========================================================

@compile_assign
    enter
    sub rsp rsp 8
    stw r1 rfp -8

    ; the left type must be an l-value
    and r2 r0 0x40   ; l-value flag
    jz r2 &compile_binary_assign_not_lvalue

    ; strip the l-value flag
    and r0 r0 0x3f
    stw r0 rfp -4

    ; the right type should be cast to the left type.
    ; (we only need to do this if we're storing a word but it's simpler to do
    ; it all the time)
    ldw r0 rfp -8
    ldw r1 rfp -4
    zero r2
    call ^compile_cast

    ; figure out whether we're storing a byte or a word
    ldw r0 rfp -4
    call ^type_size
    sub r0 r0 1
    jz r0 &compile_binary_assign_byte
    imw r0 ^str_stw
    jmp &compile_binary_assign_word
:compile_binary_assign_byte
    imw r0 ^str_stb
:compile_binary_assign_word

    ; emit "stw"/"stb"
    add r0 rpp r0
    call ^emit_term

    ; emit "r0 0 r1"
    zero r0
    call ^emit_register
    call ^emit_zero
    mov r0 1
    call ^emit_register
    call ^emit_newline

    ; return the assigned type without l-value flag
    ldw r0 rfp -4
    leave
    ret

:compile_binary_assign_not_lvalue
    ; fatal, not l-value
    imw r0 ^error_assign_not_lvalue
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; static char compile_add_sub(const char* opcode, char left_type, char right_type);
; ==========================================================
; Compiles a binary addition or subtraction, performing the appropriate pointer
; arithmetic where necessary and returning the result type.
;
; vars:
; - opcode: rfp-4
; - left_type: rfp-8
; - right_type: rfp-12
; - both_pointers: rfp-16
; ==========================================================

@compile_add_sub

    ; If both sides are pointers, their types must match. We do the arithmetic
    ; first, then divide by the arithmetic factor.
    ;
    ; If only one side is a pointer, we multiply the other by its arithmetic
    ; factor first, then do the arithmetic. The result is the same pointer
    ; again.
    ;
    ; Non-pointer values have already been promoted from char to int. If
    ; neither side is a pointer, we return int.

    ; TODO Addition and subtraction actually have fairly different rules.
    ; Addition is symmetric and only one side is allowed to be a pointer.
    ; Subtraction is not symmetric, both sides can be pointers but if the left
    ; is an integer the right cannot be a pointer. This is not implemented
    ; correctly but this code does work, it's just more lax than what we're
    ; supposed to allow so it's fine for bootstrapping. See 6.5.6 in the C11
    ; spec.

    ; setup our stack frame
    enter
    push r0
    push r1
    push r2
    push 0

    ; check if both sides are pointers
    ldw r0 rfp -8
    ldw r1 rfp -12
    and r2 r0 0x0F
    jz r2 &compile_add_sub_not_both_pointers
    and r2 r1 0x0F
    jz r2 &compile_add_sub_not_both_pointers

    ; both are pointers. the types must match.
    sub r2 r0 r1
    jnz r2 &compile_add_sub_different_pointers
    stw 1 rfp -16
    jmp &compile_add_sub_skip_pre_factors
:compile_add_sub_not_both_pointers

    ; at most one side is a pointer.
    ; multiply the right side by the arithmetic factor of the left
    ldw r0 rfp -8
    mov r1 0
    mov r2 1
    call ^compile_arithmetic_factor

    ; multiply the left side by the arithmetic factor of the right
    ldw r0 rfp -12
    mov r1 1
    mov r2 1
    call ^compile_arithmetic_factor

:compile_add_sub_skip_pre_factors

    ; emit the actual add/sub
    ldw r0 rfp -4
    ldw r1 rfp -8
    ldw r2 rfp -12
    call ^compile_basic_op

    ; we do a post factor only if both are pointers.
    ldw r0 rfp -16
    jz r0 &compile_add_sub_skip_post_factor

    ; both are pointers. emit the divide
    ldw r0 rfp -8
    mov r1 0
    mov r2 0
    call ^compile_arithmetic_factor

    ; the difference between pointers is int
    mov r0 0x30 ; int
    leave
    ret

:compile_add_sub_skip_post_factor

    ; if the left type is a pointer, return it
    ldw r0 rfp -8
    and r1 r0 0x0F
    jz r1 &compile_add_sub_left_not_pointer
    leave
    ret
:compile_add_sub_left_not_pointer

    ; if the right type is a pointer, return it
    ldw r0 rfp -12
    and r1 r0 0x0F
    jz r2 &compile_add_sub_right_not_pointer
    leave
    ret
:compile_add_sub_right_not_pointer

    ; both types are scalar. promote to int
    mov r0 0x30 ; int
    leave
    ret

:compile_add_sub_different_pointers
    ; fatal, pointers must match
    imw r0 ^error_arithmetic_different_pointers
    add r0 r0 rpp
    call ^fatal



; ==========================================================
; static void compile_arithmetic_factor(char type, int register, bool multiply);
; ==========================================================
; Calculates the arithmetic factor of the given type and emits it for the given
; register.
;
; Pointers of word size (e.g. `int*`) have an arithmetic factor of four, which
; means the other operand in an addition or multiplication must be multiplied
; by four.
;
; For pointers this is essentially the sizeof() of the pointed-to type. We
; handle a few extra special cases here.
;
; vars:
; - register: rfp-4
; ==========================================================

@compile_arithmetic_factor
    enter
    push r1

    ; we can't perform arithmetic on void
    sub r3 r0 0x10
    jnz r3 &compile_arithmetic_factor_not_void
    imw r0 ^error_arithmetic_void
    add r0 r0 rpp
    call ^fatal
:compile_arithmetic_factor_not_void

    ; all other scalar types have an arithmetic factor of 1
    and r3 r0 0x0f
    jnz r3 &compile_arithmetic_factor_not_scaler
    leave
    ret
:compile_arithmetic_factor_not_scaler

    ; we can't perform arithmetic on void*
    sub r3 r0 0x11
    jnz r3 &compile_arithmetic_factor_not_voidp
    imw r0 ^error_arithmetic_voidp
    add r0 r0 rpp
    call ^fatal
:compile_arithmetic_factor_not_voidp

    ; char* has an arithmetic factor of 1
    sub r3 r0 0x21
    jnz r3 &compile_arithmetic_factor_not_charp
    leave
    ret
:compile_arithmetic_factor_not_charp

    ; all other pointer types have an arithmetic factor of 4, so we shift (signed) by 2.

    ; if we're multiplying, emit shl; if we're diving, emit shrs.
    jz r2 &compile_arithmetic_factor_shrs
    imw r0 ^str_shl
    jmp &compile_arithmetic_factor_shl
:compile_arithmetic_factor_shrs
    imw r0 ^str_shrs
:compile_arithmetic_factor_shl
    add r0 rpp r0
    call ^emit_term

    ; emit "<reg> <reg> 2"
    ldw r0 rfp-4
    call ^emit_register
    ldw r0 rfp-4
    call ^emit_register
    mov r0 2
    call ^emit_int
    call ^emit_newline

    leave
    ret



; ==========================================================
; static void compile_basic_op(const char* opcode, char left_type, char right_type);
; ==========================================================
; Compiles a basic opcode.
;
; vars:
; - opcode: rfp-4
; - left_type: rfp-8
; - right_type: rfp-12
; ==========================================================

@compile_basic_op
    enter
    push r0
    push r1
    push r2

    ; emit the opcode
    ldw r0 rfp -4
    call ^emit_term

    ; emit "r0 r1 r0"
    zero r0
    call ^emit_register
    mov r0 1
    call ^emit_register
    zero r0
    call ^emit_register
    call ^emit_newline

    ; TODO for bitwise operations on pointer types we should return the correct
    ; type. for now we just return int. this will compile incorrectly if e.g.
    ; the result of a bitwise operation on an int* is used with +/-, the
    ; arithmetic factor won't be performed.
    mov r0 0x30 ; int
    leave
    ret



; ==========================================================
; static char compile_eq(void);
; ==========================================================
; Compiles `==`, returning type int.
; ==========================================================

@compile_eq

    ; emit "sub r0 r1 r0"
    imw r0 ^str_sub
    add r0 rpp r0
    call ^emit_term
    zero r0
    call ^emit_register
    mov r0 1
    call ^emit_register
    zero r0
    call ^emit_register
    call ^emit_newline

    jmp ^compile_boolean_not



; ==========================================================
; static char compile_ne(void);
; ==========================================================
; Compiles `!=`, returning type int.
; ==========================================================

@compile_ne

    ; emit "sub r0 r1 r0"
    imw r0 ^str_sub
    add r0 rpp r0
    call ^emit_term
    zero r0
    call ^emit_register
    mov r0 1
    call ^emit_register
    zero r0
    call ^emit_register
    call ^emit_newline

    jmp ^compile_bool



; ==========================================================
; static char compile_lt(void);
; ==========================================================
; Compiles `<`, returning type int.
; ==========================================================

@compile_lt

    ; emit "cmps r0 r1 r0"
    imw r0 ^str_cmps
    add r0 rpp r0
    call ^emit_term
    zero r0
    call ^emit_register
    mov r0 1
    call ^emit_register
    zero r0
    call ^emit_register
    call ^emit_newline

    ; emit "sub r0 r0 -1"
    imw r0 ^str_sub
    add r0 rpp r0
    call ^emit_term
    zero r0
    call ^emit_register
    zero r0
    call ^emit_register
    mov r0 -1
    call ^emit_int
    call ^emit_newline

    jmp ^compile_boolean_not



; ==========================================================
; static char compile_gt(void);
; ==========================================================
; Compiles `>`, returning type int.
; ==========================================================

@compile_gt

    ; emit "cmps r0 r1 r0"
    imw r0 ^str_cmps
    add r0 rpp r0
    call ^emit_term
    zero r0
    call ^emit_register
    mov r0 1
    call ^emit_register
    zero r0
    call ^emit_register
    call ^emit_newline

    ; emit "sub r0 r0 1"
    imw r0 ^str_sub
    add r0 rpp r0
    call ^emit_term
    zero r0
    call ^emit_register
    zero r0
    call ^emit_register
    mov r0 1
    call ^emit_int
    call ^emit_newline

    jmp ^compile_boolean_not



; ==========================================================
; static char compile_le(void);
; ==========================================================
; Compiles `<=`, returning type int.
; ==========================================================

@compile_le

    ; emit "cmps r0 r1 r0"
    imw r0 ^str_cmps
    add r0 rpp r0
    call ^emit_term
    zero r0
    call ^emit_register
    mov r0 1
    call ^emit_register
    zero r0
    call ^emit_register
    call ^emit_newline

    ; emit "sub r0 r0 1"
    imw r0 ^str_sub
    add r0 rpp r0
    call ^emit_term
    zero r0
    call ^emit_register
    zero r0
    call ^emit_register
    mov r0 1
    call ^emit_int
    call ^emit_newline

    jmp ^compile_bool



; ==========================================================
; static char compile_ge(void);
; ==========================================================
; Compiles `>=`, returning type int.
; ==========================================================

@compile_ge

    ; emit "cmps r0 r1 r0"
    imw r0 ^str_cmps
    add r0 rpp r0
    call ^emit_term
    zero r0
    call ^emit_register
    mov r0 1
    call ^emit_register
    zero r0
    call ^emit_register
    call ^emit_newline

    ; emit "sub r0 r0 -1"
    imw r0 ^str_sub
    add r0 rpp r0
    call ^emit_term
    zero r0
    call ^emit_register
    zero r0
    call ^emit_register
    mov r0 -1
    call ^emit_int
    call ^emit_newline

    jmp ^compile_bool



; ==========================================================
; char compile_cast(char current_type, char desired_type, int register);
; ==========================================================
; Compiles a cast of the value in the given register from its current type to
; the desired type.
;
; Both types must be r-values.
;
; vars:
; - current_type: rfp-4
; - desired_type: rfp-8
; - register: rfp-12
; ==========================================================

=compile_cast
    enter
    push r0
    push r1
    push r2

    ; cast from char or to char does sign extension.
    ; TODO probably we shouldn't bother if both types are char though?
    sub r3 r0 0x20
    jz r3 &compile_cast_sxb
    sub r3 r1 0x20
    jz r3 &compile_cast_sxb

    ; cast to anything else does nothing.
    jmp &compile_cast_done

:compile_cast_sxb

    ; emit "sxb <reg> <reg>"
    imw r0 ^str_sxb
    add r0 rpp r0
    call ^emit_term
    ldw r0 rfp -12
    call ^emit_register
    ldw r0 rfp -12
    call ^emit_register
    call ^emit_newline

:compile_cast_done
    ; return the desired type
    ldw r0 rfp -8
    leave
    ret
