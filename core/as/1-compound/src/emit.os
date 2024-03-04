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
; This file contains functions to emit data to the output file.



; ==========================================================
; void emit_byte(char c);
; ==========================================================
; Writes the given byte to the output file.
; ==========================================================

=emit_byte
    ; no stack frame

    ; push the byte to the stack
    sub rsp rsp '04     ; push r0
    stw r0 '00 rsp      ; ...

    ; prepare args for fwrite
    add r0 '00 rsp  ; mov r0 rsp    ; r0 = &c
    add r1 '00 '01    ; mov r1 1      ; r1 = 1
    add r2 '00 '01    ; mov r2 1      ; r2 = 1
    ims ra <output_file
    ims ra >output_file
    ldw r3 rpp ra                 ; r3 = output_file

    ; call fwrite
    ims ra <fwrite
    ims ra >fwrite
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; done
    add rsp rsp '04     ; popd
    ldw rip '00 rsp     ; ret



; ==========================================================
; void emit_string(const char* cstr);
; ==========================================================
; Emits the given string to the output.
; ==========================================================

=emit_string

    ; stash the string
    sub rsp rsp '04
    stw r0 rsp '00

:emit_string_loop

    ; get the current byte
    ldw r1 rsp '00
    ldb r0 r1 '00

    ; if the byte is zero, we're done
    jz r0 &emit_string_done

    ; increment the current byte
    add r1 r1 '01   ; inc r1
    stw r1 rsp '00

    ; emit it, call emit_byte()
    ims ra <emit_byte
    ims ra >emit_byte
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; loop
    jz '00 &emit_string_loop

:emit_string_done
    add rsp rsp '04     ; popd
    ldw rip '00 rsp     ; ret



; ==========================================================
; void emit_hex_char(char c);
; ==========================================================
; Emits the given four bits as one hexadecimal character.
; ==========================================================

@emit_hex_char
    ; don't bother to set up a stack frame

    ; see if we need a letter or a number
    cmpu r1 r0 '09
    cmpu r1 r1 '01
    jz r1 &emit_hex_bits_alpha

    ; number
    add r0 r0 "0"
    jz '00 &emit_hex_bits_done

    ; letter
:emit_hex_bits_alpha
    add r0 r0 '37  ; 'A' - 10

    ; tail-call emit_byte
:emit_hex_bits_done
    ims ra <emit_byte
    ims ra >emit_byte
    add rip rpp ra



; ==========================================================
; void emit_byte_as_hex(char c);
; ==========================================================
; Writes the hexadecimal representation of the given byte to the output file.
; ==========================================================

=emit_byte_as_hex

    ; set up a stack frame
    sub rsp rsp '04     ; push rfp
    stw rfp '00 rsp     ; ...
    add rfp rsp '00     ; mov rfp rsp

    ; collect the low bits and push them to the stack
    and r1 r0 '0F
    sub rsp rsp '04     ; push r1
    stw r1 '00 rsp

    ; collect the high bits and call emit_hex_char
    ror r0 r0 '04
    and r0 r0 '0F
    ims ra <emit_hex_char
    ims ra >emit_hex_char
    sub rsp rsp '04          ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra         ; jump
    add rsp rsp '04          ; pop return address

    ; pop the low bits and call emit_hex_char
    ldw r0 '00 rsp      ; pop r0
    add rsp rsp '04
    ims ra <emit_hex_char
    ims ra >emit_hex_char
    sub rsp rsp '04          ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra         ; jump
    add rsp rsp '04          ; pop return address

    ; return
    add rsp rfp '00     ; mov rsp rfp    ; leave
    ldw rfp '00 rsp     ; pop rfp
    add rsp rsp '04     ; ...
    ldw rip '00 rsp     ; ret



; ==========================================================
; void emit_bytes_as_hex(const char* p, size_t count);
; ==========================================================
; Writes the hexadecimal representation of the given bytes to the output file.
;
; vars:
; - p: rfp-4
; - count: rfp-8
; - i: rfp-12
; ==========================================================

=emit_bytes_as_hex

    ; set up a stack frame
    sub rsp rsp '04     ; push rfp
    stw rfp '00 rsp     ; ...
    add rfp rsp '00     ; mov rfp rsp

    ; initialize our local vars
    sub rsp rsp '0C
    stw r0 rfp 'FC
    stw r1 rfp 'F8
    stw '00 rfp 'F4

:emit_bytes_as_hex_loop

    ; if i matches count, we're done
    ldw r2 rfp 'F4
    ldw r1 rfp 'F8
    cmpu r0 r1 r2
    jz r0 &emit_bytes_as_hex_done

    ; get p[i]
    ldw r0 rfp 'FC
    ldw r2 rfp 'F4
    ldb r0 r0 r2

    ; call emit_byte_as_hex()
    ims ra <emit_byte_as_hex
    ims ra >emit_byte_as_hex
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; next i
    ldw r2 rfp 'F4
    add r2 r2 '01
    stw r2 rfp 'F4
    jz '00 &emit_bytes_as_hex_loop

:emit_bytes_as_hex_done

    ; return
    add rsp rfp '00     ; mov rsp rfp    ; leave
    ldw rfp '00 rsp     ; pop rfp
    add rsp rsp '04     ; ...
    ldw rip '00 rsp     ; ret



; ==========================================================
; void emit_linker(char control_character);
; ==========================================================
; Emits a linker directive with the given control character and the name stored
; in `identifier`.
; ==========================================================

=emit_linker

    ; emit the control character, call emit_byte()
    ims ra <emit_byte
    ims ra >emit_byte
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; get the identifier
    ims ra <identifier
    ims ra >identifier
    add r0 rpp ra

    ; emit it, call emit_string()
    ims ra <emit_string
    ims ra >emit_string
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; emit a space, call emit_byte(' ')
    add r0 '00 " "
    ims ra <emit_byte
    ims ra >emit_byte
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; done
    ldw rip '00 rsp        ; ret
