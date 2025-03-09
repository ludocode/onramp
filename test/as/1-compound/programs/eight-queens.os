; The MIT License (MIT)
;
; Copyright (c) 2023-2025 Fraser Heavy Software
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



;==========================================
; int main(void);
;==========================================

=main
    enter

    ; call place_queen(0)
    zero r0
    call ^place_queen

    ; return 0
    zero r0
    leave
    ret



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
; Returns true if a queen in the given row and column would be valid given
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
    sub r7 r2 r0
    jnz r7 &is_queen_valid_not_done
    mov r0 1
    ret
:is_queen_valid_not_done

    ; calculate the difference in x for checking diagonals
    sub r3 r0 r2

    ; get the queen's position in column i
    imw r8 ^queens
    add r8 rpp r8
    ldb r4 r8 r2

    ; check if there's a queen in the same row
    sub r7 r4 r1
    jz r7 &is_queen_valid_fail

    ; check if there's a queen diagonally up
    add r7 r1 r3
    sub r7 r4 r7 
    jz r7 &is_queen_valid_fail

    ; check if there's a queen diagonally down
    sub r7 r1 r3
    sub r7 r4 r7 
    jz r7 &is_queen_valid_fail

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
    enter
    sub rsp rsp 8

    ; if x is 8, we print the board instead of placing queens.
    sub r7 r0 8
    jnz r7 &place_queen_not_print

    ; tail-call print_board()
    leave
    jmp ^print_board

:place_queen_not_print

    ; initialize vars
    stw r0 rfp -4   ; store x
    stw 0 rfp -8    ; store y=0

:place_queen_next_y

    ; check if a queen in this position is valid, call is_queen_valid(x,y)
    ldw r0 rfp -4    ; load x
    ldw r1 rfp -8    ; load y
    call ^is_queen_valid

    ; if not valid, don't recurse
    jz r0 &place_queen_inc_y

    ; queen is valid. store it
    ldw r0 rfp -4    ; load x
    ldw r1 rfp -8    ; load y
    imw r8 ^queens
    add r8 rpp r8
    stb r1 r8 r0     ; queens[x] = y

    ; recurse place_queen(x+1)
    inc r0
    call ^place_queen

:place_queen_inc_y
    ldw r1 rfp -8    ; load y
    inc r1           ; inc y
    stw r1 rfp -8    ; store y

    ; next y
    sub r7 r1 8
    jnz r7 &place_queen_next_y

    ; done
    leave
    ret



; ======================================
; void print_board(void)
; ======================================
; vars:
; - x: r3, rfp-4
; - y: r4, rfp-8
; ======================================

=print_board
    enter
    sub rsp rsp 8

    ; iterate over y from 0 to 8
    stw 0 rfp -8      ; y=0
:print_board_next_y

        ; iterate over x from 0 to 8
        stw 0 rfp -4     ; x=0
        :print_board_next_x

            ; check whether there is a queen in this position.
            ldw r3 rfp -4
            imw r8 ^queens
            add r8 rpp r8
            ldb r7 r8 r3   ; r7 = queens[x]
            ldw r4 rfp -8
            sub r7 r7 r4   ; if r7 == y

            ; emit the appropriate character, call putchar() with 'Q' or '.'
            jz r7 &print_board_queen
            mov r0 "."    ; it's not a queen, print '.'
            jmp &print_board_char
        :print_board_queen
            mov r0 "Q"    ; it's a queen, print 'Q'
        :print_board_char
            call ^putchar

            ; increment x
            ldw r3 rfp -4
            inc r3
            stw r3 rfp -4

            ; break if x == 8
            sub r7 r3 8
            jz r7 &print_board_x_done

            ; print a space for alignment
            mov r0 " "
            call ^putchar

            ; next
            jmp &print_board_next_x
        :print_board_x_done

        ; print newline, call putchar('\n')
        mov r0 '0A  ; \n
        call ^putchar

        ; next y, break if y == 8
        ldw r4 rfp -8
        inc r4
        stw r4 rfp -8
        sub r7 r4 8
        jnz r7 &print_board_next_y

    ; print two newlines
    mov r0 '0A  ; \n
    call ^putchar
    mov r0 '0A  ; \n
    call ^putchar

    ; return
    popd
    leave
    ret
