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



; Generates all solutions to the 8 queens problem, printing them to standard
; output.



; global vars:
; - process_info_table: r9
; - queens: r8



;==========================================
; [[noreturn]] void __start(uint32_t* process_info_table);
;==========================================

; TODO this was written before we were linking against libc, hack to workaround it
;=_start
;    ; format indicator
;    "~Onr~amp~   "
=main
ims r0 <__process_info_table
ims r0 >__process_info_table
ldw r0 rpp r0

    ; put process info table in r9
    add r9 '00 r0

    ; put queens array in r8
    ims r8 <queens
    ims r8 >queens
    add r8 r8 rpp

    ; call place_queen(0)
    add r0 '00 '00
    ims ra <place_queen
    ims ra >place_queen
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; jump to exit with status 0
    add r0 '00 '00
    ldw rip r9 '08     ; load exit address into rip



; ======================================
; char queens[8];
; ======================================
; An array of queen y positions. (The index is x.)
; ======================================

=queens
    'FF 'FF 'FF 'FF 'FF 'FF 'FF 'FF



; ======================================
; bool is_queen_valid(int x, int y);
; ======================================
; Returns true if the queen in the given row and column would be valid given
; the queens in the preceding columns, or false otherwise
;
; vars:
; - x: 0
; - y: 1
; - i: r2      ; iterating over columns < x
; - diff: r3   ; difference x-i (for checking diagonals)
; - q: r4      ; queen position in column i
; ======================================

=is_queen_valid
    ; no stack frame

    ; iterate i from 0 to x
    add r2 '00 '00
:is_queen_valid_next_i

    ; if i==x, we've passed all tests, return true.
    cmpu ra r2 r0
    jz ra &is_queen_valid_done
    jz '00 &is_queen_valid_not_done
:is_queen_valid_done
    add r0 '00 '01
    ldw rip '00 rsp     ; ret
:is_queen_valid_not_done

    ; calculate the difference in x for checking diagonals
    sub r3 r0 r2

    ; get the queen's position in column i
    ldb r4 r8 r2

    ; check if there's a queen in the same row
    cmpu ra r4 r1
    jz ra &is_queen_valid_fail

    ; check if there's a queen diagonally up
    add ra r1 r3
    cmpu ra r4 ra
    jz ra &is_queen_valid_fail

    ; check if there's a queen diagonally down
    sub ra r1 r3
    cmpu ra r4 ra
    jz ra &is_queen_valid_fail

    ; next i
    add r2 r2 '01    ; inc r2
    jz '00 &is_queen_valid_next_i

:is_queen_valid_fail
    ; return false
    add r0 '00 '00
    ldw rip '00 rsp     ; ret



; ======================================
; void place_queen(int x)
; ======================================
; Places a queen at all valid positions in the given column, recursing for each
; to place the next column.
;
; If the column number is 8, this instead prints the board.
;
; vars:
; - x: r0, rfp-4
; - y: r1, rfp-8
; ======================================

=place_queen

    ; if x is 8, we print the board instead of placing queens.
    cmpu ra r0 '08
    jz ra &place_queen_print
    jz '00 &place_queen_not_print
    
:place_queen_print

        ; call print_board()
        ims ra <print_board
        ims ra >print_board
        sub rsp rsp '04     ; push return address
        add rb rip '08
        stw rb '00 rsp
        add rip rpp ra    ; jump
        add rsp rsp '04     ; pop return address

        ; return
        ldw rip '00 rsp     ; ret

:place_queen_not_print

    ; set up a stack frame
    sub rsp rsp '04     ; push rfp        ; enter
    stw rfp '00 rsp     ; ...
    add rfp rsp '00     ; mov rfp rsp

    ; load our vars into the frame
    sub rsp rsp '08
    stw r0 rfp 'FC   ; store x
    stw '00 rfp 'F8    ; store y=0

:place_queen_next_y

    ; check if a queen in this position is valid
    ldw r0 rfp 'FC    ; load x
    ldw r1 rfp 'F8    ; load y
    ims ra <is_queen_valid
    ims ra >is_queen_valid
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if not valid, don't recurse
    jz r0 &place_queen_inc_y

    ; queen is valid. recurse place_queen(x+1)
    ldw r0 rfp 'FC    ; load x
    ldw r1 rfp 'F8    ; load y
    stb r1 r8 r0     ; queens[x] = y
    add r0 r0 '01      ; inc r0
    ims ra <place_queen
    ims ra >place_queen
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

:place_queen_inc_y
    ; inc y
    ldw r1 rfp 'F8    ; load y
    add r1 r1 '01      ; inc r1
    stw r1 rfp 'F8    ; store y

    ; next y
    cmpu ra r1 '08
    jz ra &place_queen_done
    jz '00 &place_queen_next_y

    ; done
:place_queen_done
    add rsp rfp '00     ; mov rsp rfp    ; leave
    ldw rfp '00 rsp     ; pop rfp
    add rsp rsp '04     ; ^^^
    ldw rip '00 rsp     ; ret



; ======================================
; void print_board(void)
; ======================================
; vars:
; - x: r3
; - y: r4
; "Q": rsp
; ".": rsp+1
; " ": rsp+2
; "\n": rsp+3
; ======================================

=print_board

    ; push "Q. \n" to the stack
    sub rsp rsp '04
    ims ra '20 '0A
    ims ra "Q" "."
    stw ra rsp '00

    ; prepare syscall args
    ldw r0 r9 '10     ; r0 = stdout
    add r2 '00 '01    ; r2 = string length (1)

    ; iterate over y from 0 to 8
    add r4 '00 '00
:print_board_next_y

        ; iterate over x from 0 to 8
        add r3 '00 '00
        :print_board_next_x

            ; check whether there is a queen in this position. 0 for "Q", 1 for "."
            ldb ra r8 r3
            cmpu ra ra r4
            and ra ra '01

            ; print the character, syscall fwrite(stdout, rsp+ra, 1)
            add r1 rsp ra    ; r1 = string address (rsp+ra)
            sys fwrite '00 '00

            ; increment x
            add r3 r3 '01   ; inc r3
            cmpu ra r3 '08
            jz ra &print_board_done_x

            ; print a space for alignment
            add r1 rsp '02    ; r1 = string address (rsp+2)
            sys fwrite '00 '00

            ; next
            jz '00 &print_board_next_x
        :print_board_done_x

        ; print newline
        add r1 rsp '03    ; r1 = string address (rsp+3)
        sys fwrite '00 '00

        ; next y
        add r4 r4 '01   ; inc r4
        cmpu ra r4 '08
        jz ra &print_board_done_y
        jz '00 &print_board_next_y
:print_board_done_y

    ; print two newlines
    add r1 rsp '03    ; r1 = string address (rsp+3)
    sys fwrite '00 '00
    sys fwrite '00 '00

    ; return
    add rsp rsp '04
    ldw rip '00 rsp     ; ret
