#!/bin/sh

# Copyright (c) 2023-2025 Fraser Heavy Software
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.



# This is an experimental pure POSIX shell implementation of the Onramp virtual
# machine. It's mostly just a proof of concept at this point, too slow to be
# useful. It does technically "work", if you're okay with waiting months to get
# anything done.
#
# I wrote this in part to prove that you really can implement the Onramp VM in
# "anything" (not particularly surprising), but also in part because the shell
# is one of the few programming languages available out-of-the-box on a POSIX
# system. So this could have in theory allowed you to bootstrap a modern C
# compiler on any old POSIX system that has nothing else besides a shell and
# coreutils.
#
# Of course if it's too slow then it doesn't actually work, so this is probably
# all pointless. Given the difficulty of getting this working, it was actually
# much easier to write a VM directly in x86_64 machine code.



# Implementation details:
#
# POSIX shell doesn't have arrays so we use dynamically named variables, like
# _0, _1, _2, etc. for memory and REGISTER_0, REGISTER_1, etc. for registers.
#
# The VM relies heavily on arithmetic expressions, i.e. `$(( ... ))`, which are
# specified by POSIX. Your shell must support these for this VM to work. If you
# have a POSIX-style shell that doesn't support these, it is in theory possible
# to port this to use `bc` or some other tool, but it would be a fair amount of
# work and it would make it much slower than it already is.
#
# POSIX specifies that arithmetic expressions only need to support the range of
# signed long, which on 32-bit platforms is 32 bits. To work around this, we
# `and` all values with 0xFFFFFFFF after doing any kind of arithmetic. On
# 32-bit shells, high values will be negative signed, and on 64-bit shells, all
# values will be non-negative. Almost all operations give exactly the same
# results, and those that don't are handled specially (see the implementation
# of "ltu" for example) so it works on both. As far as I can tell this is both
# the fastest and simplest way to make the shell independent of word size.



# Performance notes:
#
# When run in bash, dash, zsh and BusyBox, all non-syscall instructions
# *should* be done strictly with builtins (although I haven't really confirmed
# that.)
#
# Syscalls, especially file access syscalls, may use external commands (e.g.
# `ls`, `dd`, etc.)
#
# Shells have wildly different performance. This VM is something like 20x
# faster in BusyBox compared to Bash. The other shells I've tested are
# somewhere in between. Even BusyBox is way too slow to make this work though.
# Under BusyBox, this VM is roughly 200,000x slower than the C89 VM.
#
# I have some ideas for how to potentially improve performance but I doubt I
# could make up the >1000x improvement needed to make this useful. Still, I've
# written stuff below in case I ever want to revisit this.
#
# The main thing needed first is a proper benchmarking suite to test the speed
# of various instructions and syscalls individually so we can get an idea of
# what's slow.
#
# I assume the `eval` calls are a major problem so any way to avoid them could
# make a difference. If we can change functions like `parse_mix`,
# `set_register` and `cache_fetch` (on a hit) to not use `eval` (perhaps using
# large `case` statements instead), it might make a big difference. On the
# other hand large case statements might be even slower (as evidenced by the
# slow hex/sh-alt tools.)
#
# I have some ideas for how to improve cache line performance but none of them
# are good. Cache lines could potentially be decimal strings delimited by IFS,
# or each byte in each cache line could be its own variable. This probably
# wouldn't help though. Since the backing is in hexadecimal you don't
# necessarily save much if you need to convert entire cache lines to and from
# hexadecimal whenever you get a cache miss.
#
# File I/O is not properly buffered yet. Buffering read calls should be fairly
# easy and should make a difference. Buffering write calls I expect would be
# harder, not impossible but maybe not worth it.



#set -e

# Use line feed as the internal field separator. ANSI C quoting isn't in POSIX
# so we use printf instead.
IFS="$(printf '\012')"



################################################################
# Common Functions
################################################################

fatal() {
    echo ERROR: "$@" >&2
    exit 125
}


################################################################
# Memory Backing
################################################################

# The start of mapped VM memory. This is arbitrary.
# Words are stored in shell variables, named like so _$((address)).
# Uninitialized memory reads as 0 as per POSIX.
MEMORY_START=$((0x10000))
MEMORY_END=$(($MEMORY_START + 1024 * 1024 * 16)) # 16 MiB

# Verifies that a memory address is valid. If it is not, the VM aborts.
check_address() {
    if [ $1 -lt $MEMORY_START ] || [ $1 -ge $MEMORY_END ]; then
        fatal "Invalid memory address: $1"
    fi
}

check_alignment() {
    if [ $(( $1 & 0x3 )) -ne 0 ]; then
        fatal "Invalid memory alignment: $1"
    fi
}

# Loads a byte at the given address and places it in LOAD_BYTE_RET.
load_byte() {
    # Check the memory address
    check_address $1
    LOAD_BYTE_IX=$(( $1 / 4 ))
    LOAD_BYTE_OFFSET=$(( ($1 & 0x3) * 8 ))

    # Extract the value from the appropriate shell variable and byte offset.
    LOAD_BYTE_RET=$(( (_${LOAD_BYTE_IX} >> LOAD_BYTE_OFFSET) & 0xFF ))
}

# Loads a word from the given address and places it in LOAD_WORD_RET.
load_word() {
    # Check the memory address
    check_address $1
    check_alignment $1
    LOAD_BYTE_IX=$(( $1 / 4 ))

    # The adress is aligned so we know it's not split across 2 shell variables
    LOAD_WORD_RET=$(( _${LOAD_BYTE_IX} & 0xFFFFFFFF ))
}

# Stores a byte at the given address.
store_byte() {
    # Check the memory address
    check_address $1

    LOAD_BYTE_IX=$(( $1 / 4 ))
    LOAD_BYTE_OFFSET=$(( ($1 & 0x3) * 8 ))
    # We clear the byte at the appropriate offset and then set it to the new value.
    LOAD_WORD_RET=$(( _${LOAD_BYTE_IX} & 0xFFFFFFFF ))
    LOAD_WORD_RET=$(( LOAD_WORD_RET & ~(0xFF << LOAD_BYTE_OFFSET) )) # clear the byte
    LOAD_WORD_RET=$(( LOAD_WORD_RET | (($2 & 0xFF) << LOAD_BYTE_OFFSET) )) # set the byte to the new value
    : $(( _${LOAD_BYTE_IX} = LOAD_WORD_RET ))
}

# Stores a word at the given address.
store_word() {
    # Check the memory address
    check_address $1
    check_alignment $1

    LOAD_BYTE_IX=$(( $1 / 4 ))
    # the adress is aligned so we know it's not split across 2 shell variables
    : $(( _${LOAD_BYTE_IX} = $2 & 0xFFFFFFFF ))
}



################################################################
# Registers
################################################################

# Registers are stored as 32-bit signed decimal strings.

registers_init() {
    I=0
    while [ $I -ne 16 ]; do
        : $(( REGISTER_$I = 0 ))
        I=$(( $I + 1 ))
    done
}

register_get() {
    REGISTER_GET_RET=$(( REGISTER_$1 ))
}

register_set() {
#    if [ $2 -gt 2147483647 ]; then
#        REGISTER_SET_VALUE=$(( $2 - 4294967296 ))
#        #echo a $2 $REGISTER_SET_VALUE >&2
#    elif [ $2 -lt -2147483648 ]; then
#        #echo b >&2
#        REGISTER_SET_VALUE=$(( $2 + 4294967296 ))
#    else
#        #echo c >&2
#        REGISTER_SET_VALUE=$2
#    fi
#    #echo register_set $1 $REGISTER_SET_VALUE $(printf %08X $REGISTER_SET_VALUE) >&2
#    : $(( REGISTER_$1 = $REGISTER_SET_VALUE ))

    #echo register_set $1 $2 $(printf %08X $2) >&2
    : $(( REGISTER_$1 = $2 ))
}

registers_print() {
    I=0
    while [ $I -ne 16 ]; do
        register_get $I
        echo REGISTER_$I=$REGISTER_GET_RET >&2
        I=$(( $I + 1 ))
    done
}



################################################################
# I/O
################################################################

files_init() {
    I=0
    while [ $I -ne 16 ]; do
        #echo "$I" >&2
        eval FILE_${I}_NAME=
        eval FILE_${I}_SIZE=-1
        eval FILE_${I}_BUFFER=x
        eval FILE_${I}_OFFSET=-1
        eval FILE_${I}_MODE=0
        I=$(( $I + 1 ))
    done

    FILE_0_NAME=/dev/stdin
    FILE_0_MODE=0

    FILE_1_NAME=/dev/stdout
    FILE_1_MODE=1

    FILE_2_NAME=/dev/stderr
    FILE_2_MODE=1
}

file_print() {
    echo "file $1:"
    eval echo "  name" \$FILE_${1}_NAME
    eval echo "  size" \$FILE_${1}_SIZE
    eval echo "  offset" \$FILE_${1}_OFFSET
    eval echo "  buffer" \$FILE_${1}_BUFFER
    eval echo "  mode" \$FILE_${1}_MODE
}

file_find_free() {
    I=0
    while [ $I -ne 16 ]; do
        if [ "x$(eval echo \$FILE_${I}_NAME)" = "x" ]; then
            FILE_FIND_FREE_RET=$I
            break
        fi
        I=$(( $I + 1 ))
    done

    if [ $I -eq 16 ]; then
        fatal "Too many open files."
    fi

    FILE_FIND_FREE_RET=$I
}

file_open() {
    file_find_free
    I=$FILE_FIND_FREE_RET

    # Open for reading
    if [ $2 -eq 0 ]; then
        if ! [ -e "$1" ]; then
            # TODO for now if a file doesn't exist we abort. Eventually we'll
            # need to return an error to the syscall.
            fatal "File not found: $1"
        fi
        eval FILE_${I}_SIZE=$(wc -c < $1)
        eval FILE_${I}_OFFSET=0

    # Open for writing
    elif [ $2 -eq 1 ]; then
        rm -f "$1"
        touch "$1"
        eval FILE_${I}_SIZE=0
        # TODO for now we don't support append. We always overwrite. We also
        # assume it's a regular file, not a stream.
        eval FILE_${I}_OFFSET=0

    else
        fatal "Invalid file mode"
    fi

    eval FILE_${I}_NAME="\$1"
    eval FILE_${I}_MODE="\$2"
    eval FILE_${I}_BUFFER=x

    FILE_OPEN_RET=$I
}

file_close() {
    if [ "x$(eval echo \$FILE_${1}_NAME)" = "x" ]; then
        fatal "Handle is already closed."
    fi
    eval FILE_${1}_NAME=
    eval FILE_${1}_BUFFER=x
}

# Reads one byte from the given file, returning it as unsigned decimal in
# FILE_READ_RET, or an empty string if the end of the file has been reached.
#
# TODO this could probably be sped up a lot by converting directly to hex with
# `od` and avoiding the conversions to/from decimal when reading to memory
# (which is always.)
#
# TODO buffering is not implemented yet. We call out to dd for each byte.
file_read() {
    FILE_READ_NAME="$(eval echo \$FILE_${1}_NAME)"
    if [ "x$FILE_READ_NAME" = "x" ]; then
        fatal "Handle is not open."
    fi

    FILE_READ_MODE="$(eval echo \$FILE_${1}_MODE)"
    if [ $FILE_READ_MODE -ne 0 ]; then
        fatal "Handle is not open for reading."
    fi

    FILE_READ_OFFSET="$(eval echo \$FILE_${1}_OFFSET)"
    FILE_READ_SIZE="$(eval echo \$FILE_${1}_SIZE)"

    if [ $FILE_READ_OFFSET -eq $FILE_READ_SIZE ]; then
        # end-of-file
        FILE_READ_RET=
    else
        FILE_READ_RET=$(dd if="$FILE_READ_NAME" bs=1 count=1 skip=$FILE_READ_OFFSET 2>/dev/null | od -A n -N 1 -t u1 | tr -d " ")
        eval FILE_${1}_OFFSET=$(( $FILE_READ_OFFSET + 1 ))
    fi
}

# Writes to the given file the given unsigned decimal byte.
#
# TODO also no buffering yet
file_write() {
    #echo file_write $1 $2 >&2
    #echo filename: "$(eval echo \$FILE_${1}_NAME)" >&2
    #echo filename: $FILE_1_NAME >&2

    FILE_WRITE_NAME="$(eval echo \$FILE_${1}_NAME)"
    if [ "x$FILE_WRITE_NAME" = "x" ]; then
        fatal "Handle is not open."
    fi

    FILE_WRITE_MODE="$(eval echo \$FILE_${1}_MODE)"
    if [ $FILE_WRITE_MODE -ne 1 ]; then
        fatal "Handle is not open for writing."
    fi

    FILE_WRITE_OFFSET="$(eval echo \$FILE_${1}_OFFSET)"
    if [ $FILE_WRITE_OFFSET -eq -1 ]; then
        # For streams we don't seek or increment the offset but we still need
        # to tell dd to append in case it isn't really a stream (for example if
        # the VM output is redirected to a file.)
        # We have to break up these options because of our IFS.
        FILE_WRITE_APPEND="oflag=append"
        FILE_WRITE_NOTRUNC="conv=notrunc"
        :
    else
        FILE_WRITE_SEEK="seek=$FILE_WRITE_OFFSET"
        eval FILE_${1}_OFFSET=$(( $FILE_WRITE_OFFSET + 1 ))
    fi

    # For some reason I couldn't get the dd command to work with our special
    # streams in every shell (zsh...) so we just special case it.
    if [ "$FILE_WRITE_NAME" = /dev/stdout ]; then
        printf \\$(printf %o $2)
    elif [ "$FILE_WRITE_NAME" = /dev/stderr ]; then
        printf \\$(printf %o $2) >&2
    else
        printf \\$(printf %o $2) | dd if=/dev/stdin of=$FILE_WRITE_NAME bs=1 count=1 \
            $FILE_WRITE_SEEK $FILE_WRITE_APPEND $FILE_WRITE_NOTRUNC 2>/dev/null
    fi
}

file_seek() {
    FILE_SEEK_FILENAME="$(eval echo \$FILE_${1}_NAME)"
    if [ "x$FILE_SEEK_FILENAME" = "x" ]; then
        fatal "Handle is not open."
    fi

    eval FILE_${1}_OFFSET=$2
}



################################################################
# Initialization Process
################################################################

# Returns the number of strings in STRING_COUNT_RET.
string_count() {
    STRING_COUNT_RET=$#
}

# Copies a list of strings into the VM at CURRENT_ADDRESS, returning the
# address of the array in COPY_STRINGS_RET.
copy_strings() {
    string_count "$@"

    # Reserve space for the array
    COPY_STRINGS_RET=$CURRENT_ADDRESS
    CURRENT_ADDRESS=$(( $CURRENT_ADDRESS + ($STRING_COUNT_RET + 1) * 4 ))

    # For each string...
    I=0
    while [ $I -ne $STRING_COUNT_RET ]; do
        STR="$1"

        # Copy the string to the heap
        J=0
        while [ ! -z "$STR" ]; do
            CHR=${STR%"${STR#?}"}       # Get the first character
            STR=${STR#?}                # Remove the first character
            BYTE=$(printf %d "'$CHR")   # Get the byte value of the character
            store_byte $(( $CURRENT_ADDRESS + $J )) "$BYTE"
            J=$(($J + 1))
        done
        store_byte $(( $CURRENT_ADDRESS + $J )) 0

        # Write the string's address to the array
        store_word $(( $COPY_STRINGS_RET + $I * 4 )) $CURRENT_ADDRESS
        CURRENT_ADDRESS=$(( $CURRENT_ADDRESS + $J + 1 ))

        shift
        I=$(( $I + 1 ))
    done

    # Write the null terminator
    store_word $(( $COPY_STRINGS_RET + $STRING_COUNT_RET * 4 )) 0

    # Align the current address
    CURRENT_ADDRESS=$(( ($CURRENT_ADDRESS + 3) & (0xFFFFFFFC) ))
}

# Loads the process info vector and its contents (command-line arguments,
# environment variables, halt code, etc.)
process_init() {
    CURRENT_ADDRESS=$MEMORY_START
    PROCESS_INFO_TABLE=$CURRENT_ADDRESS
    CURRENT_ADDRESS=$(( $CURRENT_ADDRESS + 40 ))

    # Generate the halt code
    EXIT_ADDRESS=$CURRENT_ADDRESS
    CURRENT_ADDRESS=$(( $CURRENT_ADDRESS + 4 ))
    store_word $EXIT_ADDRESS 127   # 0x0000007E == sys halt 0 0

    # Fill the process info table
    store_word $PROCESS_INFO_TABLE 1             # version
    # break is set in program_init()
    store_word $(( $PROCESS_INFO_TABLE +  8 )) $EXIT_ADDRESS   # exit address
    store_word $(( $PROCESS_INFO_TABLE + 12 )) 0               # stdin
    store_word $(( $PROCESS_INFO_TABLE + 16 )) 1               # stdout
    store_word $(( $PROCESS_INFO_TABLE + 20 )) 2               # stderr
    store_word $(( $PROCESS_INFO_TABLE + 32 )) 1               # wrapper style (TODO support it)
    store_word $(( $PROCESS_INFO_TABLE + 36 )) 7               # capabilities = echo | blocking | line-oriented
}

args_init() {
    copy_strings "$@"
    store_word $(( $PROCESS_INFO_TABLE + 24 )) $COPY_STRINGS_RET   # argv
}

env_init() {
    copy_strings $(env)
    store_word $(( $PROCESS_INFO_TABLE + 28 )) $COPY_STRINGS_RET   # environ
}

# Loads the program into memory and sets the program break.
program_init() {
    PROGRAM_FILENAME=$1
    if [ "x$PROGRAM_FILENAME" = "x" ]; then
        fatal "A program name is required."
    fi
    if ! [ -e "$PROGRAM_FILENAME" ]; then
        fatal "The program file doesn't exist."
    fi

    # Get file size with `ls -l`, the format of which is specified in POSIX.
    PROGRAM_SIZE=$(wc -c < $PROGRAM_FILENAME)

    # Load each byte from disk and store it in memory (TODO do this in blocks, this is really slow)
    #echo "Loading program $PROGRAM_FILENAME" >&2
    file_open $PROGRAM_FILENAME 0
    HANDLE=$FILE_OPEN_RET
    I=0
    while [ $I -ne $PROGRAM_SIZE ]; do
        file_read $HANDLE
        BYTE=$FILE_READ_RET
#        BYTE_HEX=$(od -A n -N 1 -j $I -t x1 $PROGRAM_FILENAME)
#        if [ $? -ne 0 ]; then
#            fatal "Failed to load a byte from the program."
#        fi
#        BYTE=$(( 0x$(echo $BYTE_HEX | tr -d " ") ))
#        #echo "I: $I   BYTE: $BYTE"
        store_byte $(( $CURRENT_ADDRESS + $I )) $BYTE
        I=$(( $I + 1 ))
    done
    file_close $HANDLE
    #echo "Done loading program" >&2

    # Store the program break in the process info table
    store_word $(( $PROCESS_INFO_TABLE + 4 )) $(( $CURRENT_ADDRESS + $PROGRAM_SIZE ))   # break

    # Set initial registers
    REGISTER_0=$PROCESS_INFO_TABLE  # r0
    REGISTER_12=$MEMORY_END         # rsp
    REGISTER_14=$CURRENT_ADDRESS    # rpp
    REGISTER_15=$CURRENT_ADDRESS    # rip
}



################################################################
# Syscalls
################################################################

syscall_open() {
    # Read the filename from memory
    FILENAME=
    I=$REGISTER_0
    while true; do
        load_byte $I
        if [ $LOAD_BYTE_RET -eq 0 ]; then
            break
        fi
        FILENAME="$FILENAME$(printf \\$(printf %o $LOAD_BYTE_RET))"
        I=$(( $I + 1 ))
    done

    # Call open
    MODE=$REGISTER_1
    file_open "$FILENAME" $MODE

    # Put the result in r0
    register_set 0 $FILE_OPEN_RET
}

syscall_close() {
    file_close $REGISTER_0
}

syscall_read() {
    HANDLE=$REGISTER_0
    ADDRESS=$REGISTER_1
    COUNT=$REGISTER_2

    # Safety for 32-bit shells
    if [ $COUNT -lt 0 ]; then
        fatal "Invalid read size."
    fi

    I=0
    while [ $I -ne $COUNT ]; do
        file_read $HANDLE
        if [ "x$FILE_READ_RET" = "x" ]; then
            break
        fi
        store_byte $(( ($ADDRESS + $I) & 0xFFFFFFFF )) $FILE_READ_RET
        I=$(( $I + 1 ))
    done

    register_set 0 $I
}

syscall_write() {
    HANDLE=$REGISTER_0
    ADDRESS=$REGISTER_1
    COUNT=$REGISTER_2

    # Safety for 32-bit shells
    if [ $COUNT -lt 0 ]; then
        fatal "Invalid write count."
    fi

    I=0
    while [ $I -ne $COUNT ]; do
        load_byte $(( ($ADDRESS + $I) & 0xFFFFFFFF ))
        file_write $HANDLE $LOAD_BYTE_RET
        I=$(( $I + 1 ))
    done

    register_set 0 $I
}

syscall() {
    case $1 in
        0)  # halt
            exit $(( $REGISTER_0 & 0xFF ))
            ;;
        3) syscall_open ;;
        4) syscall_close ;;
        5) syscall_read ;;
        6) syscall_write ;;
        *)
            fatal "Unhandled syscall: $1"
            ;;
    esac
}



################################################################
# Instruction Execution
################################################################

parse_register() {
    PARSE_REGISTER_RET=$(( $1 - 0x80 ))
    if [ $PARSE_REGISTER_RET -lt 0 -o $PARSE_REGISTER_RET -ge 16 ]; then
        fatal "A register-type argument is invalid."
    fi
}

parse_mix() {
    PARSE_MIX_RET=$(( $1 ))
    if [ $PARSE_MIX_RET -ge 144 ]; then
        PARSE_MIX_RET=$(( ($PARSE_MIX_RET - 256) & 0xFFFFFFFF ))
    elif [ $PARSE_MIX_RET -ge 128 ]; then
        register_get $(( $PARSE_MIX_RET - 0x80 ))
        PARSE_MIX_RET=$REGISTER_GET_RET
    fi
}

run() {
    while true; do
        #echo "about to load instruction word at $REGISTER_15"
        load_word $REGISTER_15
        : $(( OPCODE = ($LOAD_WORD_RET >> 0)  & 0xFF ))
        : $(( ARG1   = ($LOAD_WORD_RET >> 8)  & 0xFF ))
        : $(( ARG2   = ($LOAD_WORD_RET >> 16) & 0xFF ))
        : $(( ARG3   = ($LOAD_WORD_RET >> 24) & 0xFF ))
        # for easier debugging, but should be removed for performance
        OPCODE_HEX=$(printf %02X $OPCODE)
        #echo >&2
        #echo "instruction $OPCODE $ARG1 $ARG2 $ARG3" >&2
#        if [ $OPCODE = "00" ]; then
#            registers_print
#        fi
        REGISTER_15=$(( $REGISTER_15 + 4 ))

        case $OPCODE_HEX in
            70)
                #echo add >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                register_set $PARSE_REGISTER_RET $(( ($ARG2 + $PARSE_MIX_RET) & 0xFFFFFFFF ))
                ;;
            71)
                #echo sub >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                register_set $PARSE_REGISTER_RET $(( ($ARG2 - $PARSE_MIX_RET) & 0xFFFFFFFF ))
                ;;
            72)
                #echo mul >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                register_set $PARSE_REGISTER_RET $(( ($ARG2 * $PARSE_MIX_RET) & 0xFFFFFFFF ))
                ;;
            73)
                #echo div >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                register_set $PARSE_REGISTER_RET $(( ($ARG2 / $PARSE_MIX_RET) & 0xFFFFFFFF ))
                ;;
            74)
                #echo and >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                register_set $PARSE_REGISTER_RET $(( ($ARG2 & $PARSE_MIX_RET) & 0xFFFFFFFF ))
                ;;
            75)
                #echo or >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                register_set $PARSE_REGISTER_RET $(( ($ARG2 | $PARSE_MIX_RET) & 0xFFFFFFFF ))
                ;;
            76)
                #echo shl >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                register_set $PARSE_REGISTER_RET $(( ($ARG2 << $PARSE_MIX_RET) & 0xFFFFFFFF ))
                ;;
            77)
                #echo shru >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                ARG3=$PARSE_MIX_RET
                # We only have a signed shift. We shift down by 1, mask out the
                # high bit, then shift the rest.
                if [ $ARG3 -eq 0 ]; then
                    register_set $PARSE_REGISTER_RET $ARG2
                else
                    register_set $PARSE_REGISTER_RET $(( ((($ARG2 >> 1) & 0x7FFFFFFF) >> ($ARG3 - 1)) ))
                fi
                ;;
            78)
                #echo ldw >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                load_word $(( ($ARG2 + $PARSE_MIX_RET) & 0xFFFFFFFF ))
                register_set $PARSE_REGISTER_RET $LOAD_WORD_RET
                ;;
            79)
                #echo stw >&2
                parse_mix $ARG1
                ARG1=$PARSE_MIX_RET
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                store_word $(( ($ARG2 + $PARSE_MIX_RET) & 0xFFFFFFFF )) $ARG1
                ;;
            7A)
                #echo ldb >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                load_byte $(( ($ARG2 + $PARSE_MIX_RET) & 0xFFFFFFFF ))
                register_set $PARSE_REGISTER_RET $LOAD_BYTE_RET
                ;;
            7B)
                #echo stb >&2
                parse_mix $ARG1
                ARG1=$PARSE_MIX_RET
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                store_byte $(( ($ARG2 + $PARSE_MIX_RET) & 0xFFFFFFFF )) $(( $ARG1 & 0xFF ))
                ;;
            7C)
                #echo ims >&2
                parse_register $ARG1
                register_get $PARSE_REGISTER_RET
                register_set $PARSE_REGISTER_RET $(( ($REGISTER_GET_RET << 16) & 0xFFFFFFFF | (ARG3 << 8) | ARG2 ))
                ;;
            7D)
                #echo ltu >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                ARG3=$PARSE_MIX_RET

                # Our comparison is supposed to be unsigned but a POSIX shell
                # only needs to support signed long arithmetic which will be
                # 32 bits on a 32-bit platform. We need to check for signedness
                # manually. (These first two cases never happen on a 64-bit
                # shell because values are 64-bit unsigned.)
                #echo $ARG2 $ARG3 >&2
                if [ $ARG2 -ge 0 -a $ARG3 -lt 0 ]; then
                    CMPU=0
                elif [ $ARG2 -lt 0 -a $ARG3 -ge 0 ]; then
                    CMPU=1
                else
                    CMPU=$(( $ARG2 < $ARG3 ))
                fi

                register_set $PARSE_REGISTER_RET $CMPU
                ;;
            7E)
                #echo jz >&2
                parse_mix $ARG1
                if [ 0 -eq $PARSE_MIX_RET ]; then
                    OFFSET=$(( (($ARG3 << 8) | ARG2) & 0xFFFF ))
                    if [ $OFFSET -ge 32768 ]; then
                        OFFSET=$(( $OFFSET - 65536 ))
                    fi
                    #echo "jump $OFFSET words" >&2
                    REGISTER_15=$(( $REGISTER_15 + ($OFFSET << 2) ))
                else
                    #echo "no jump" >&2
                    :
                fi
                ;;
            7F)
                if [ $(( ARG2 )) -ne 0 -o $(( ARG3 )) -ne 0 ]; then
                    fatal "The additional arguments to the sys instruction must be zero."
                fi
                syscall $ARG1
                ;;
            *)
                fatal "Invalid opcode: $OPCODE_HEX"
        esac
    done
}

registers_init
files_init
process_init
args_init "$@"
#env_init#TODO
program_init "$1"
#        store_word $((0x0100FFFC)) 1  #debugging
run



#
# Directory syscalls
#

# TODO on a diropen syscall, do `ls -1` and store the results in a string. To
# read a directory entry, pass the whole string as arguments to a function, the
# function should `shift` off one entry and store the remaining elements back
# to the string.
