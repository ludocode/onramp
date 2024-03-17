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
;=__start
;    "~Onr~amp~   "
=main
imw r0 ^__process_info_table
ldw r0 rpp r0

    ; put process info table in r9
    mov r9 r0

    ; put queens array in r8
    imw r8 ^queens
    add r8 r8 rpp

    ; call place_queen(0)
    mov r0 0
    call ^place_queen

    ; jump to exit with status 0
    mov r0 0
    ldw rip r9 8     ; load exit address into rip



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
    ; don't bother with a stack frame

    ; iterate i from 0 to x
    zero r2
:is_queen_valid_loop

    ; if i==x, we've passed all tests, return true.
    cmpu r7 r2 r0
    jne r7 &is_queen_valid_not_done
    mov r0 1
    ret
:is_queen_valid_not_done

    ; calculate the difference in x for checking diagonals
    sub r3 r0 r2

    ; get the queen's position in column i
    ldb r4 r8 r2

    ; check if there's a queen in the same row
    cmpu r7 r4 r1
    je r7 &is_queen_valid_fail

    ; check if there's a queen diagonally up
    add r7 r1 r3
    cmpu r7 r4 r7 
    je r7 &is_queen_valid_fail

    ; check if there's a queen diagonally down
    sub r7 r1 r3
    cmpu r7 r4 r7 
    je r7 &is_queen_valid_fail

    ; next i
    inc r2
    jmp &is_queen_valid_loop

:is_queen_valid_fail
    ; return false
    zero r0
    ret



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
    cmpu r7 r0 8
    jne r7 &place_queen_not_print

        ; tail-call print_board()
        jmp ^print_board

:place_queen_not_print

    ; set up a stack frame
    enter

    ; load our vars into the frame
    sub rsp rsp 8
    stw r0 rfp -4   ; store x
    stw 0 rfp -8    ; store y=0

:place_queen_next_y

    ; check if a queen in this position is valid
    ldw r0 rfp -4    ; load x
    ldw r1 rfp -8    ; load y
    call ^is_queen_valid

    ; if not valid, don't recurse
    je r0 &place_queen_inc_y

        ; queen is valid. recurse place_queen(x+1)
        ldw r0 rfp -4    ; load x
        ldw r1 rfp -8    ; load y
        stb r1 r8 r0     ; queens[x] = y
        inc r0
        call ^place_queen

:place_queen_inc_y
    ldw r1 rfp -8    ; load y
    inc r1           ; inc y
    stw r1 rfp -8    ; store y

    ; next y
    cmpu r7 r1 8
    jne r7 &place_queen_next_y

    ; done
    leave
    ret



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
    ; don't bother with a stack frame

    ; push "Q. \n" to the stack
    ims r7 '20 '0A
    ims r7 "Q" "."
    push r7

    ; prepare syscall args
    ldw r0 r9 16  ; r0 = stdout
    mov r2 1      ; r2 = string length (1)

    ; iterate over y from 0 to 8
    zero r4
:print_board_next_y

        ; iterate over x from 0 to 8
        zero r3
        :print_board_next_x

            ; check whether there is a queen in this position. 0 for "Q", 1 for "."
            ldb r7 r8 r3
            cmpu r7 r7 r4
            and r7 r7 1

            ; print the character, syscall write(stdout, rsp+ra, 1)
            add r1 rsp r7    ; r1 = string address (rsp+ra)
            sys fwrite '00 '00

            ; increment x
            inc r3
            cmpu r7 r3 8
            je r7 &print_board_x_done

            ; print a space for alignment
            add r1 rsp 2    ; r1 = string address (rsp+2)
            sys fwrite '00 '00

            ; next
            jmp &print_board_next_x
        :print_board_x_done

        ; print newline
        add r1 rsp 3    ; r1 = string address (rsp+3)
        sys fwrite '00 '00

        ; next y
        inc r4
        cmpu r7 r4 8
        jne r7 &print_board_next_y

    ; print two newlines
    add r1 rsp 3    ; r1 = string address (rsp+3)
    sys fwrite '00 '00
    sys fwrite '00 '00

    ; return
    popd
    ret
