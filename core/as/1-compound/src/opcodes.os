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
; This file contains common opcode functions and data.



; ==========================================================
; static void** opcodes_table;
; ==========================================================
; The hashtable of opcodes.
;
; It's a simple fixed-size hashtable that uses open addressing with linear
; probing. It is initialized on startup.
; ==========================================================

@opcodes_table
    '00 '00 '00 '00



; ==========================================================
; static int opcodes_table_capacity;
; ==========================================================
; The number of bukcets in the hashtable of opcodes.
; ==========================================================

@opcodes_table_capacity
    '80 '00 '00 '00   ; 128



; ==========================================================
; static void** opcodes_list;
; ==========================================================
; The list of opcodes.
;
; The list is a null-terminated sequence of pairs. Each pair consists of a
; pointer to the null-terminated opcode name followed by the address of the
; function to handle it.
; ==========================================================

=opcodes_list

    ; arithmetic
    ^opcode_str_add ^opcode_add
    ^opcode_str_sub ^opcode_sub
    ^opcode_str_mul ^opcode_mul
    ^opcode_str_divu ^opcode_divu
    ^opcode_str_divs ^opcode_divs
    ^opcode_str_modu ^opcode_modu
    ^opcode_str_mods ^opcode_mods
    ^opcode_str_zero ^opcode_zero
    ^opcode_str_inc ^opcode_inc
    ^opcode_str_dec ^opcode_dec
    ^opcode_str_sxs ^opcode_sxs
    ^opcode_str_sxb ^opcode_sxb
    ^opcode_str_trs ^opcode_trs
    ^opcode_str_trb ^opcode_trb

    ; logic
    ^opcode_str_and ^opcode_and
    ^opcode_str_or ^opcode_or
    ^opcode_str_xor ^opcode_xor
    ^opcode_str_not ^opcode_not
    ^opcode_str_shl ^opcode_shl
    ^opcode_str_shru ^opcode_shru
    ^opcode_str_shrs ^opcode_shrs
    ;^opcode_str_rol ^opcode_rol
    ;^opcode_str_ror ^opcode_ror
    ^opcode_str_mov ^opcode_mov
    ^opcode_str_bool ^opcode_bool
    ^opcode_str_isz ^opcode_isz

    ; memory
    ^opcode_str_ldw ^opcode_ldw
    ^opcode_str_lds ^opcode_lds
    ^opcode_str_ldb ^opcode_ldb
    ^opcode_str_stw ^opcode_stw
    ^opcode_str_sts ^opcode_sts
    ^opcode_str_stb ^opcode_stb
    ^opcode_str_push ^opcode_push
    ^opcode_str_pop ^opcode_pop
    ^opcode_str_popd ^opcode_popd

    ; control
    ^opcode_str_ims ^opcode_ims
    ^opcode_str_imw ^opcode_imw
    ^opcode_str_cmps ^opcode_cmps
    ^opcode_str_cmpu ^opcode_cmpu
    ^opcode_str_lts ^opcode_lts
    ^opcode_str_ltu ^opcode_ltu
    ^opcode_str_jz ^opcode_jz
    ^opcode_str_jnz ^opcode_jnz
    ^opcode_str_je ^opcode_je
    ^opcode_str_jne ^opcode_jne
    ^opcode_str_jl ^opcode_jl
    ^opcode_str_jg ^opcode_jg
    ^opcode_str_jle ^opcode_jle
    ^opcode_str_jge ^opcode_jge
    ^opcode_str_jmp ^opcode_jmp
    ^opcode_str_call ^opcode_call
    ^opcode_str_ret ^opcode_ret
    ^opcode_str_enter ^opcode_enter
    ^opcode_str_leave ^opcode_leave
    ^opcode_str_sys ^opcode_sys
    '00 '00 '00 '00

; arithmetic
:opcode_str_add "add" '00
:opcode_str_sub "sub" '00
:opcode_str_mul "mul" '00
:opcode_str_divu "divu" '00
:opcode_str_divs "divs" '00
:opcode_str_modu "modu" '00
:opcode_str_mods "mods" '00
:opcode_str_zero "zero" '00
:opcode_str_inc "inc" '00
:opcode_str_dec "dec" '00
:opcode_str_sxs "sxs" '00
:opcode_str_sxb "sxb" '00
:opcode_str_trs "trs" '00
:opcode_str_trb "trb" '00

; logic
:opcode_str_and "and" '00
:opcode_str_or "or" '00
:opcode_str_xor "xor" '00
;:opcode_str_ror "ror" '00
:opcode_str_mov "mov" '00
:opcode_str_not "not" '00
;:opcode_str_rol "rol" '00
:opcode_str_shrs "shrs" '00
:opcode_str_shru "shru" '00
:opcode_str_shl "shl" '00
:opcode_str_bool "bool" '00
:opcode_str_isz "isz" '00

; memory
:opcode_str_ldw "ldw" '00
:opcode_str_lds "lds" '00
:opcode_str_ldb "ldb" '00
:opcode_str_stw "stw" '00
:opcode_str_sts "sts" '00
:opcode_str_stb "stb" '00
:opcode_str_push "push" '00
:opcode_str_pop "pop" '00
:opcode_str_pushn "pushn" '00
:opcode_str_popn "popn" '00
:opcode_str_popd "popd" '00

; control
:opcode_str_ims "ims" '00
:opcode_str_jz "jz" '00
:opcode_str_sys "sys" '00
:opcode_str_imw "imw" '00
:opcode_str_cmpu "cmpu" '00
:opcode_str_cmps "cmps" '00
:opcode_str_ltu "ltu" '00
:opcode_str_lts "lts" '00
:opcode_str_jnz "jnz" '00
:opcode_str_jmp "jmp" '00
:opcode_str_je "je" '00
:opcode_str_jne "jne" '00
:opcode_str_jg "jg" '00
:opcode_str_jge "jge" '00
:opcode_str_jl "jl" '00
:opcode_str_jle "jle" '00
:opcode_str_enter "enter" '00
:opcode_str_leave "leave" '00
:opcode_str_call "call" '00
:opcode_str_ret "ret" '00



; ==========================================================
; void opcodes_init(void);
; ==========================================================
; Creates the opcodes hashtable.
;
; vars:
; - list_ptr: rfp-4
; - bucket_index: rfp-8
; - mask: rfp-12
; ==========================================================

=opcodes_init

    ; set up a stack frame
    sub rsp rsp '04     ; push rfp       ; enter
    stw rfp '00 rsp     ; ...
    add rfp rsp '00     ; mov rfp rsp

    ; make space for local vars
    sub rsp rsp '0C

    ; get the table size and bucket size
    ims ra <opcodes_table_capacity
    ims ra >opcodes_table_capacity
    ldw r0 rpp ra
    add r1 '00 '08   ; two words per bucket

    ; get the mask
    sub r2 r0 '01
    stw r2 rfp 'F4

    ; call calloc()
    ims ra <calloc
    ims ra >calloc
    sub rsp rsp '04     ; push return address
    add rb rip '08      ; ...
    stw rb '00 rsp      ; ...
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; store it
    ims ra <opcodes_table
    ims ra >opcodes_table
    stw r0 rpp ra

    ; get the start of the opcode list
    ims ra <opcodes_list
    ims ra >opcodes_list
    add r0 rpp ra
    stw r0 rfp 'FC

:opcodes_init_list_loop

    ; get the current name from the list
    ldw r1 rfp 'FC
    ldw r0 r1 '00
    jz r0 &opcodes_init_done
    add r0 rpp r0

    ; call fnv1a_cstr()
    ims ra <fnv1a_cstr
    ims ra >fnv1a_cstr
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; store the starting bucket index
    ldw r2 rfp 'F4
    and r0 r0 r2
    stw r0 rfp 'F8

    :opcodes_init_bucket_loop

        ; This could be optimized, there are no function calls here so we don't
        ; need all the loads and stores

        ; get the opcode table
        ims ra <opcodes_table
        ims ra >opcodes_table
        ldw r0 rpp ra

        ; check if the bucket is empty
        ldw r1 rfp 'F8
        mul r3 r1 '08    ; TODO shl
        ldw ra r0 r3
        jz ra &opcodes_init_found

        ; next bucket
        add r1 r1 '01
        ldw r2 rfp 'F4
        and r1 r1 r2
        stw r1 rfp 'F8
        jz '00 &opcodes_init_bucket_loop

    :opcodes_init_found

    ; copy the pair into the bucket, converting program-relative addresses to
    ; absolute as we go
    ldw r5 rfp 'FC
    ldw r4 r5 '00
    add r4 rpp r4
    stw r4 r0 r3
    add r3 r3 '04
    ldw r4 r5 '04
    add r4 rpp r4
    stw r4 r0 r3

    ; next list item
    ldw r0 rfp 'FC
    add r0 r0 '08
    stw r0 rfp 'FC
    jz '00 &opcodes_init_list_loop

:opcodes_init_done

    ; return
    add rsp rfp '00     ; mov rsp rfp    ; leave
    ldw rfp '00 rsp     ; pop rfp
    add rsp rsp '04     ; ...
    ldw rip '00 rsp     ; ret



; ==========================================================
; void opcodes_destroy(void);
; ==========================================================

=opcodes_destroy

    ; get the table
    ims ra <opcodes_table
    ims ra >opcodes_table
    ldw r0 rpp ra

    ; call free()
    ims ra <free
    ims ra >free
    sub rsp rsp '04     ; push return address
    add rb rip '08      ; ...
    stw rb '00 rsp      ; ...
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ldw rip '00 rsp        ; ret



; ==========================================================
; void opcodes_dispatch(void);
; ==========================================================
; Calls the appropriate opcode handler for the opcode in `identifier`.
;
; vars:
; - bucket_index: rfp-4
; - mask: rfp-8
; ==========================================================

=opcodes_dispatch

    ; set up a stack frame
    sub rsp rsp '04     ; push rfp       ; enter
    stw rfp '00 rsp     ; ...
    add rfp rsp '00     ; mov rfp rsp

    ; make space for local vars
    sub rsp rsp '08

    ; get the identifier
    ims ra <identifier
    ims ra >identifier
    add r0 rpp ra

    ; call fnv1a_cstr()
    ims ra <fnv1a_cstr
    ims ra >fnv1a_cstr
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the mask
    ims ra <opcodes_table_capacity
    ims ra >opcodes_table_capacity
    ldw r1 rpp ra
    sub r1 r1 '01
    stw r1 rfp 'F8

    ; get the starting bucket index
    and r0 r0 r1
    stw r0 rfp 'FC

:opcodes_dispatch_loop

    ; get current bucket index
    ldw r0 rfp 'FC
    mul r1 r0 '08  ; TODO shl

    ; get name in this bucket. if it's null, opcode is not found.
    ims ra <opcodes_table
    ims ra >opcodes_table
    ldw r2 rpp ra
    ldw r0 r2 r1
    jz r0 &opcodes_dispatch_not_found

    ; get current identifier
    ims ra <identifier
    ims ra >identifier
    add r1 rpp ra

    ; check if opcode matches, call strcmp()
    ims ra <strcmp
    ims ra >strcmp
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address
    jz r0 &opcodes_dispatch_found

    ; no match. next bucket
    ldw r0 rfp 'FC
    add r0 r0 '01
    ldw r1 rfp 'F8
    and r0 r0 r1
    stw r0 rfp 'FC
    jz '00 &opcodes_dispatch_loop

:opcodes_dispatch_found

    ; opcode found. get the function
    ims ra <opcodes_table
    ims ra >opcodes_table
    ldw r2 rpp ra
    ldw r0 rfp 'FC
    mul r1 r0 '08  ; TODO shl
    add r1 r1 '04
    ldw r0 r2 r1

    ; tail-call it
    add rsp rfp '00     ; mov rsp rfp   ; leave
    ldw rfp '00 rsp     ; pop rfp       ; ^^^
    add rsp rsp '04     ; ^^^           ; ^^^
    add rip '00 r0    ; jump

:opcodes_dispatch_not_found
    ; not an opcode. fatal error
    ims r0 <error_unrecognized_opcode
    ims r0 >error_unrecognized_opcode
    add r0 rpp r0
    ims ra <fatal
    ims ra >fatal
    add rip rpp ra



; ==========================================================
; void opcode_standard(uint8_t opcode_byte)
; ==========================================================
; Implements a standard register-mix-mix opcode.
;
; Most primitive instructions forward to this.
; ==========================================================

=opcode_standard

    ; prepare our template on the stack
    ;    <opcode_byte> <dest> <src1> <src2>
    sub rsp rsp '04

    ; opcode byte goes first
    stb r0 rsp '00

    ; call parse_register_mix_mix(rsp+1)
    add r0 rsp '01
    ims ra <parse_register_mix_mix
    ims ra >parse_register_mix_mix
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; output the template
    add r0 rsp '00
    add r1 '00 '04
    ims ra <emit_bytes_as_hex
    ims ra >emit_bytes_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; done
    add rsp rsp '04   ; popd (template)
    ldw rip '00 rsp        ; ret
