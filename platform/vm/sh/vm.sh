#!/bin/sh

# Copyright (c) 2023-2024 Fraser Heavy Software
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
# POSIX shell doesn't have arrays so we use dynamically named variables. They
# are accessed using `eval`. POSIX shell also doesn't have a straightforward
# way to store bytes so we store everything in hexadecimal.
#
# Memory is stored in a series of dynamically named variables containing long
# hexadecimal strings. A cache sits in front with smaller hexadecimal strings,
# and file I/O is (or could be) buffered in hexadecimal strings as well.
#
# Load and store operations are performed by string splicing of hexadecimal via
# `cut` and `printf %X`. All memory and cache strings are stored in reverse
# order so we don't have to re-order the bytes of a hexadecimal word. Our
# machine is effectively big-endian.
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
# of "cmpu" for example) so it works on both. As far as I can tell this is both
# the fastest and simplest way to make the shell independent of word size.



# Performance notes:
#
# When run in bash, dash, zsh and BusyBox, all non-syscall instructions
# *should* be done strictly with builtins (although I haven't really confirmed
# that.) Other shells may use external commands if things like `cut` and
# `printf` are not builtins.
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
# The `cut` calls are a major problem. It seems all shells fork and create
# pipes and such to do it even though they implement `cut` internally. I
# haven't tested the performance of shell variable substitutions but if there
# is a way to use them to eliminate `cut` calls in cache manipulation that
# might greatly improve performance.
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

# Replaces a substring of a string, returning it in REPLACE_SUBSTRING_RET.
replace_substring() {
    SUBSTRING_PAGE=$1
    SUBSTRING_START=$2
    SUBSTRING_VALUE=$3

    SUBSTRING_END=$(($SUBSTRING_START + ${#SUBSTRING_VALUE}))

    #echo "replace $SUBSTRING_PAGE $SUBSTRING_START $SUBSTRING_VALUE" >&2

    # cut does not allow empty ranges. We have to check.
    SUBSTRING_BEFORE=
    if [ $SUBSTRING_START -ne 0 ]; then
        SUBSTRING_BEFORE=$(echo $SUBSTRING_PAGE | cut -c-$SUBSTRING_START)
    fi
    SUBSTRING_AFTER=
    if [ $SUBSTRING_END -ne ${#SUBSTRING_PAGE} ]; then
        SUBSTRING_AFTER=$(echo $SUBSTRING_PAGE | cut -c$(($SUBSTRING_END + 1))-)
    fi

    REPLACE_SUBSTRING_RET=$SUBSTRING_BEFORE$SUBSTRING_VALUE$SUBSTRING_AFTER
}



################################################################
# Memory Backing
################################################################

# The memory backing contains all of the VM memory. It does not contain
# registers, files or other metadata.
#
# Memory is stored in pages. Each page contains a string of uppercase
# hexadecimal characters of length 2 * $MEM_PAGE_SIZE, or 'x' if the page
# is zero.
#
# Pages are stored in a series of variables of the form $MEM_{n} where {n} is
# the page index. The page index ranges from 0 to $MEM_COUNT - 1.
#
# Note that there is a cache that sits in front of the memory. The memory
# backing functions should only be used by the cache.
#
# All bytes are stored in reverse order. This allows us to do all memory access
# as big-endian and convert between hexadecimal and decimal without having to
# do expensive byte swapping. From the VM's perspective, all memory is
# little-endian. The address inversion is done in the cache, not here.
#

# The size of an individual page in bytes. This must be a power of two. The
# page will be stored in hexadecimal so the string length will be 2x this. The
# string is passed to `cut` so if `cut` is not a shell builtin, the string must
# be less than ARG_MAX. The shell may have string length limits of its own as
# well. We keep it at 8kB (so 16k string length) to keep it conservative. The
# higher this number is, the more expensive cache flushes will be as the entire
# string must be recreated.
# TODO convert this to bits, like cache TODO why?
BACKING_PAGE_SIZE=4096

# The number of pages. This times the page size gives the total amount of
# address space available to the program. Most shells seem to work fine even if
# this is 100000 or more although it adds a few seconds of startup time to
# assign all the variables.
BACKING_PAGE_COUNT=4096

# The start of mapped VM memory. This is arbitrary.
MEMORY_START=$((0x10000))

# The end of memory is the address just after the last byte. (This is the
# initial value of the stack pointer.)
MEMORY_END=$(($MEMORY_START + $BACKING_PAGE_SIZE * $BACKING_PAGE_COUNT))
#echo "START $MEMORY_START END $MEMORY_END SIZE $(( $MEMORY_END - $MEMORY_START )) ALTSIZE $(( $BACKING_PAGE_SIZE * $BACKING_PAGE_COUNT ))" >&2

# Initializes the backing.
backing_init() {

    # We initially assign all pages to 'x'. We only fill pages when storing a
    # non-zero value.
    BACKING_INDEX=0
    while [ $BACKING_INDEX -ne $BACKING_PAGE_COUNT ]; do
        eval BACKING_$BACKING_INDEX=x
        BACKING_INDEX=$(($BACKING_INDEX + 1))
    done

    # Generate an empty page.
    BACKING_INDEX=16
    BACKING_EMPTY_PAGE=00000000000000000000000000000000
    while [ $BACKING_INDEX -ne $BACKING_PAGE_SIZE ]; do
        BACKING_EMPTY_PAGE=${BACKING_EMPTY_PAGE}${BACKING_EMPTY_PAGE}
        BACKING_INDEX=$(($BACKING_INDEX * 2))
    done
}

# Gets the memory page index of an address, returning it in BACKING_PAGE_INDEX.
# The memory address must be valid.
# TODO inline this function, no longer needed
backing_page_index() {
    BACKING_PAGE_INDEX_ADDR=$1
#    if [ $BACKING_PAGE_INDEX_ADDR -lt $MEMORY_START ] || [ $BACKING_PAGE_INDEX_ADDR -ge $MEMORY_END ]; then
#        fatal "Address $BACKING_PAGE_INDEX_ADDR is out of range."
#    fi
#    BACKING_PAGE_INDEX=$(( ($BACKING_PAGE_INDEX_ADDR - $MEMORY_START) / $BACKING_PAGE_SIZE ))
    BACKING_PAGE_INDEX=$(( $BACKING_PAGE_INDEX_ADDR / $BACKING_PAGE_SIZE ))
}

# Loads a cache line from the backing, returning it in BACKING_LOAD_LINE.
# The given address must be properly aligned.
backing_load() {
    BACKING_LOAD_ADDR=$1

    backing_page_index $BACKING_LOAD_ADDR
    BACKING_LOAD_INDEX=$BACKING_PAGE_INDEX
    BACKING_LOAD_OFFSET=$((($BACKING_LOAD_ADDR % $BACKING_PAGE_SIZE) * 2))
    BACKING_LOAD_PAGE=$(eval echo \$BACKING_$BACKING_LOAD_INDEX)

    #echo "backing_load(): BACKING_LOAD_ADDR: $BACKING_LOAD_ADDR" >&2
    #echo "backing_load(): BACKING_LOAD_INDEX: $BACKING_LOAD_INDEX" >&2
    #echo "backing_load(): BACKING_LOAD_OFFSET: $BACKING_LOAD_OFFSET" >&2
    #echo "backing_load(): BACKING_LOAD_PAGE: $BACKING_LOAD_PAGE" >&2

    if [ "$BACKING_LOAD_PAGE" = "x" ]; then
        #echo "backing_load(): Page is empty. Returning empty cache line: $CACHE_EMPTY_LINE" >&2
        BACKING_LOAD_LINE=$CACHE_EMPTY_LINE
    else
        #echo "backing_load(): Returning cache line: $(echo $BACKING_LOAD_PAGE | cut -c$(($BACKING_LOAD_OFFSET +  1))-$(($BACKING_LOAD_OFFSET + 2 * $CACHE_LINE_SIZE)))" >&2
        BACKING_LOAD_LINE="$($BACKING_LOAD_PAGE | cut -c$(($BACKING_LOAD_OFFSET +  1))-$(($BACKING_LOAD_OFFSET + 2 * $CACHE_LINE_SIZE)))"
    fi
}

# Stores a cache line into the backing.
# The given address must be properly aligned.
backing_store() {
    BACKING_STORE_ADDR=$1
    BACKING_STORE_VALUE=$2

    backing_page_index $BACKING_STORE_ADDR
    BACKING_STORE_INDEX=$BACKING_PAGE_INDEX
    BACKING_STORE_OFFSET=$((($BACKING_STORE_ADDR % $BACKING_PAGE_SIZE) * 2))
    BACKING_STORE_PAGE=$(eval echo \$BACKING_$BACKING_STORE_INDEX)

    #echo "backing_store(): BACKING_STORE_ADDR: $BACKING_STORE_ADDR" >&2
    #echo "backing_store(): BACKING_STORE_VALUE: $BACKING_STORE_VALUE" >&2
    #echo "backing_store(): BACKING_STORE_INDEX: $BACKING_STORE_INDEX" >&2
    #echo "backing_store(): BACKING_STORE_OFFSET: $BACKING_STORE_OFFSET" >&2
    #echo "backing_store(): BACKING_STORE_PAGE: $BACKING_STORE_PAGE" >&2

    BACKING_STORE_NOTHING=0
    if [ "$BACKING_STORE_PAGE" = "x" ]; then
        if [ "$BACKING_STORE_VALUE" = "$CACHE_EMPTY_LINE" ]; then
            #echo "backing_store(): Page is empty but we're storing a zero cache line. Doing nothing." >&2
            BACKING_STORE_NOTHING=1
        else
            #echo "backing_store(): Page is empty. Generating a new zero page." >&2
            BACKING_STORE_PAGE=$BACKING_EMPTY_PAGE
        fi
    fi

    if [ $BACKING_STORE_NOTHING -eq 0 ]; then
        replace_substring $BACKING_STORE_PAGE $BACKING_STORE_OFFSET $BACKING_STORE_VALUE
        #echo "backing_store(): Replaced substring. New page: $REPLACE_SUBSTRING_RET" >&2
        eval BACKING_$BACKING_STORE_INDEX="\$REPLACE_SUBSTRING_RET"
    fi
}



################################################################
# Memory Cache
################################################################

# The memory cache sits in front of the backing. Memory pages are huge strings
# for which manipulation is expensive so the cache uses smaller strings to
# provide faster memory operations.
#
# The cache is implemented as a hash table. Each bucket contains a number of
# cache lines in most-recently-used order. Cache lines are fetched from the
# backing on a cache miss and flushed to the backing when evicted.
#
# The number of cache lines per bucket is currently hardcoded to two to
# simplify the fetch and eviction code. See cache_fetch().
#

# The number of bits used for the size in bytes of a cache line. This must be
# smaller than the backing page size.
CACHE_LINE_BITS=6   # 2^6 == 64 bytes

# The size of a cache line in bytes. The cache line will be stored in
# hexadecimal so the string length will be 2x this.
CACHE_LINE_SIZE=$(( 1 << $CACHE_LINE_BITS ))

# The length of a cache line in hex chars.
CACHE_LINE_LENGTH=$(( $CACHE_LINE_SIZE << 1 ))

# Mask used to grab the lower bits of an address.
CACHE_LINE_MASK=$(( $CACHE_LINE_SIZE - 1 ))

if [ $CACHE_LINE_SIZE -ge $BACKING_PAGE_SIZE ]; then
    fatal "Invalid configuration. Cache line size $CACHE_LINE_SIZE must be less than page size $BACKING_PAGE_SIZE"
fi

# The number of bits used for hashing. The bucket count is two to the power of
# this.
CACHE_BUCKET_BITS=6  # 2^6 == 64 buckets

# The total number of buckets.
CACHE_BUCKET_COUNT=$(( 1 << $CACHE_BUCKET_BITS ))

# Initializes the cache.
cache_init() {

    # All cache lines are set to address 0 and value 'x' to mark them as empty.
    CACHE_BUCKET=0
    while [ $CACHE_BUCKET -ne $CACHE_BUCKET_COUNT ]; do
        # Currently using two lines per bucket; see cache_fetch().
        eval CACHE_${CACHE_BUCKET}_0_ADDR=-1
        eval CACHE_${CACHE_BUCKET}_0_VALUE=x
        eval CACHE_${CACHE_BUCKET}_1_ADDR=-1
        eval CACHE_${CACHE_BUCKET}_1_VALUE=x
        CACHE_BUCKET=$(($CACHE_BUCKET + 1))
    done

    # Generate an empty cache line.
    CACHE_INDEX=8
    CACHE_EMPTY_LINE=0000000000000000
    while [ $CACHE_INDEX -ne $CACHE_LINE_SIZE ]; do
        CACHE_EMPTY_LINE=${CACHE_EMPTY_LINE}${CACHE_EMPTY_LINE}
        CACHE_INDEX=$(($CACHE_INDEX * 2))
    done
}

# Places the cache line for the given address into the front of its
# corresponding bucket, returning the index in CACHE_FETCH_BUCKET.
#
# The cache line will be at $CACHE_{b}_0_VALUE where {b} is the returned index.
#
# If the cache line is not found in the cache, it will be fetched from the
# backing, and another cache line will be evicted and flushed to make room.
#CACHE_FETCH_DEBUG=1
cache_fetch() {
    # Invert the address and align it to a cache line
    CACHE_FETCH_ADDR=$(( ($1 >> $CACHE_LINE_BITS) << $CACHE_LINE_BITS ))

    # Hash the address to choose a bucket. We use a 31-bit Knuth multiplicative
    # hash and trim the top bit to avoid any sign issues.
    CACHE_FETCH_BUCKET=$(( (($CACHE_FETCH_ADDR * 1327217867) & 0x7FFFFFFF) >> (31 - $CACHE_BUCKET_BITS) ))

    #[ $CACHE_FETCH_DEBUG -eq 0 ] || echo "cache_fetch(): !!! CACHE_FETCH_ADDR $CACHE_FETCH_ADDR CACHE_FETCH_BUCKET $CACHE_FETCH_BUCKET" >&2
    #[ $CACHE_FETCH_DEBUG -eq 0 ] || echo "cache_fetch(): CACHE_${CACHE_FETCH_BUCKET}_0_ADDR=$(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_0_ADDR)" >&2
    #[ $CACHE_FETCH_DEBUG -eq 0 ] || echo "cache_fetch(): CACHE_${CACHE_FETCH_BUCKET}_1_ADDR=$(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_1_ADDR)" >&2

    # The number of lines per bucket is currently fixed at 2 to simplify the
    # below code. It's probably faster than using a bunch of loops anyway. Also
    # see the initialization code in cache_init() which assumes two lines.
    if [ $CACHE_FETCH_ADDR -eq $(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_0_ADDR) ]; then
        # It's already at the front. nothing to do.
        #[ $CACHE_FETCH_DEBUG -eq 0 ] || echo "cache_fetch(): Cache line already at front." >&2
        :

    elif [ $CACHE_FETCH_ADDR -eq $(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_1_ADDR) ]; then
        # It's at index 1. Swap it with index 0.
        #[ $CACHE_FETCH_DEBUG -eq 0 ] || echo "cache_fetch(): Cache line at index 1. Swapping cache lines" >&2
        CACHE_FETCH_LINE_ADDR=$(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_1_ADDR)
        CACHE_FETCH_LINE_VALUE=$(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_1_VALUE)
        eval CACHE_${CACHE_FETCH_BUCKET}_1_ADDR="$(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_0_ADDR)"
        eval CACHE_${CACHE_FETCH_BUCKET}_1_VALUE="$(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_0_VALUE)"
        eval CACHE_${CACHE_FETCH_BUCKET}_0_ADDR="$CACHE_FETCH_LINE_ADDR"
        eval CACHE_${CACHE_FETCH_BUCKET}_0_VALUE="$CACHE_FETCH_LINE_VALUE"

    else
        # It's not in the cache.
        #[ $CACHE_FETCH_DEBUG -eq 0 ] || echo "cache_fetch(): Cache line not in cache." >&2

        # Evict the line at position 1
        CACHE_FETCH_LINE_ADDR=$(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_1_ADDR)
        CACHE_FETCH_LINE_VALUE=$(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_1_VALUE)
        #echo "cache_fetch(): Evicting line $CACHE_FETCH_LINE_ADDR" >&2
        if [ "$CACHE_FETCH_LINE_VALUE" != "x" ]; then
            #[ $CACHE_FETCH_DEBUG -eq 0 ] || echo "cache_fetch(): Evicting line $CACHE_FETCH_LINE_ADDR"
            backing_store $CACHE_FETCH_LINE_ADDR $CACHE_FETCH_LINE_VALUE
        else
            #[ $CACHE_FETCH_DEBUG -eq 0 ] || echo "cache_fetch(): Line to evict is uninitialized. Nothing to do." >&2
            :
        fi

        # Move line at 0 to 1
        #[ $CACHE_FETCH_DEBUG -eq 0 ] || echo "cache_fetch(): Moving line 0 to 1" >&2
        eval CACHE_${CACHE_FETCH_BUCKET}_1_ADDR="$(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_0_ADDR)"
        eval CACHE_${CACHE_FETCH_BUCKET}_1_VALUE="$(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_0_VALUE)"

        # Fetch the line into position 0
        #[ $CACHE_FETCH_DEBUG -eq 0 ] || echo "cache_fetch(): Fetching line into 0" >&2
        backing_load $CACHE_FETCH_ADDR
        eval CACHE_${CACHE_FETCH_BUCKET}_0_ADDR="$CACHE_FETCH_ADDR"
        eval CACHE_${CACHE_FETCH_BUCKET}_0_VALUE="$BACKING_LOAD_LINE"
    fi

    #[ $CACHE_FETCH_DEBUG -eq 0 ] || echo "cache_fetch(): Returning bucket $CACHE_FETCH_BUCKET containing $(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_0_VALUE)" >&2
}

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

#
# The load and store functions invert all memory addresses. In other words, all
# memory strings, i.e. the backing pages and cache lines, are stored backwards.
#
# The reason for this is so that we can do big-endian hexadecimal loads and
# stores. This removes the need for expensive byte-swapping conversions.
#

# Loads a byte at the given address, converts it to decimal, and places it in
# LOAD_BYTE_RET.
load_byte() {
    # Check and invert the memory address
    check_address $1
    LOAD_BYTE_ADDR=$(( $MEMORY_END - $1 - 1))

    #echo "load_byte(): LOAD_BYTE_ADDR: $LOAD_BYTE_ADDR" >&2

    # Fetch the cache line
    cache_fetch $LOAD_BYTE_ADDR
    LOAD_BYTE_LINE=$(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_0_VALUE)
    [ ${#LOAD_BYTE_LINE} -eq $CACHE_LINE_LENGTH ] || fatal "Invalid cache line: $LOAD_BYTE_LINE"

    # Extract the value
    LOAD_BYTE_OFFSET=$(( ($LOAD_BYTE_ADDR & $CACHE_LINE_MASK) * 2 ))
    LOAD_BYTE_RET=$(( 0x$(echo $LOAD_BYTE_LINE | cut -c$(($LOAD_BYTE_OFFSET +  1))-$(($LOAD_BYTE_OFFSET + 2))) ))

    #echo "load_byte(): CACHE_FETCH_BUCKET: $CACHE_FETCH_BUCKET" >&2
    #echo "load_byte(): LOAD_BYTE_OFFSET: $LOAD_BYTE_OFFSET" >&2
    #echo "load_byte(): LOAD_BYTE_LINE: $LOAD_BYTE_LINE" >&2
    #echo "load_byte(): LOAD_BYTE_RET: $LOAD_BYTE_VALUE" >&2
}

# Loads a word from the given address, converts it to decimal, and places it
# in LOAD_WORD_RET.
load_word() {
    # Check and invert the memory address
    check_address $1
    check_alignment $1
    LOAD_WORD_ADDR=$(( $MEMORY_END - $1 - 4))

    #echo "load_word(): loading at 0x$(printf %08X $1) (flipped addr $LOAD_WORD_ADDR)" >&2

    # Fetch the cache line
    cache_fetch $LOAD_WORD_ADDR
    LOAD_WORD_LINE=$(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_0_VALUE)
    [ ${#LOAD_WORD_LINE} -eq $CACHE_LINE_LENGTH ] || fatal "Invalid cache line: $LOAD_WORD_LINE"

    # Extract the value
    LOAD_WORD_OFFSET=$(( ($LOAD_WORD_ADDR & $CACHE_LINE_MASK) * 2 ))
    LOAD_WORD_RET=$(( 0x$(echo $LOAD_WORD_LINE | cut -c$(($LOAD_WORD_OFFSET + 1))-$(($LOAD_WORD_OFFSET + 8))) ))

    #echo "load_word(): CACHE_FETCH_BUCKET: $CACHE_FETCH_BUCKET" >&2
    #echo "load_word(): LOAD_WORD_OFFSET: $LOAD_WORD_OFFSET" >&2
    #echo "load_word(): LOAD_WORD_LINE: $LOAD_WORD_LINE" >&2
    #echo "load_word(): LOAD_WORD_RET: $LOAD_WORD_RET" >&2
}

# Stores a byte at the given address.
store_byte() {
    # Check and invert the memory address
    check_address $1
    STORE_BYTE_ADDR=$(( $MEMORY_END - $1 - 1))

    # Convert to hex
    STORE_BYTE_VALUE=$(printf %02X $2)
    #echo "store_byte(): storing $2 as 0x$STORE_BYTE_VALUE at 0x$(printf %02X $1)"

    #CACHE_FETCH_DEBUG=0
    # Fetch the cache line
    cache_fetch $STORE_BYTE_ADDR
    STORE_BYTE_LINE=$(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_0_VALUE)
    [ ${#STORE_BYTE_LINE} -eq $CACHE_LINE_LENGTH ] || fatal "Invalid cache line: $STORE_BYTE_LINE"

    # Mutate it
    STORE_BYTE_OFFSET=$(( ($STORE_BYTE_ADDR & $CACHE_LINE_MASK) * 2 ))
    replace_substring $STORE_BYTE_LINE $STORE_BYTE_OFFSET $STORE_BYTE_VALUE
    [ ${#REPLACE_SUBSTRING_RET} -eq $CACHE_LINE_LENGTH ] || fatal "Cache line mutation failed"
    eval CACHE_${CACHE_FETCH_BUCKET}_0_VALUE="\$REPLACE_SUBSTRING_RET"
    #CACHE_FETCH_DEBUG=1

    #echo "store_byte(): MEMORY_START: $MEMORY_START" >&2
    #echo "store_byte(): MEMORY_END: $MEMORY_END" >&2
    #echo "store_byte(): STORE_BYTE_ADDR: $STORE_BYTE_ADDR" >&2
    #echo "store_byte(): CACHE_FETCH_BUCKET: $CACHE_FETCH_BUCKET" >&2
    #echo "store_byte(): STORE_BYTE_OFFSET: $STORE_BYTE_OFFSET" >&2
    #echo "store_byte(): STORE_BYTE_LINE (before): $STORE_BYTE_LINE" >&2
    #echo "store_byte(): CACHE_${CACHE_FETCH_BUCKET}_0_VALUE (after): $(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_0_VALUE)" >&2
}

# Stores a word at the given address.
store_word() {
    # Check and invert the memory address
    check_address $1
    check_alignment $1
    STORE_WORD_ADDR=$(( $MEMORY_END - $1 - 4))

    # Convert to hex
    STORE_WORD_VALUE=$(printf %08X $2)
    #echo "store_word(): storing $2 as 0x$STORE_WORD_VALUE at 0x$(printf %08X $1) (flipped addr $STORE_WORD_ADDR)" >&2

    # Fetch the cache line
    cache_fetch $STORE_WORD_ADDR
    STORE_WORD_LINE=$(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_0_VALUE)
    [ ${#STORE_WORD_LINE} -eq $CACHE_LINE_LENGTH ] || fatal "Invalid cache line: $STORE_WORD_LINE"

    #echo "store_word(): MEMORY_START: $MEMORY_START" >&2
    #echo "store_word(): MEMORY_END: $MEMORY_END" >&2
    #echo "store_word(): STORE_WORD_ADDR: $STORE_WORD_ADDR" >&2
    #echo "store_word(): CACHE_FETCH_BUCKET: $CACHE_FETCH_BUCKET" >&2
    #echo "store_word(): STORE_WORD_OFFSET: $STORE_WORD_OFFSET" >&2
    #echo "store_word(): STORE_WORD_LINE: $STORE_WORD_LINE" >&2
    #echo "store_word(): CACHE_${CACHE_FETCH_BUCKET}_0_VALUE: $(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_0_VALUE)" >&2

    # Mutate it
    STORE_WORD_OFFSET=$(( ($STORE_WORD_ADDR & $CACHE_LINE_MASK) * 2 ))
    replace_substring $STORE_WORD_LINE $STORE_WORD_OFFSET $STORE_WORD_VALUE
    [ ${#REPLACE_SUBSTRING_RET} -eq $CACHE_LINE_LENGTH ] || fatal "Cache line mutation failed"
    eval CACHE_${CACHE_FETCH_BUCKET}_0_VALUE="\$REPLACE_SUBSTRING_RET"
    #echo "store_word(): mutated line: $(eval echo \$CACHE_${CACHE_FETCH_BUCKET}_0_VALUE)"
}



################################################################
# Registers
################################################################

# Registers are stored as 32-bit signed decimal strings.

registers_init() {
    I=0
    while [ $I -ne 16 ]; do
        eval REGISTER_$I=0
        I=$(( $I + 1 ))
    done
}

register_get() {
    REGISTER_GET_RET=$(eval echo \$REGISTER_$1)
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
#    eval REGISTER_$1="\$REGISTER_SET_VALUE"

    #echo register_set $1 $2 $(printf %08X $2) >&2
    eval REGISTER_$1="\$2"
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
        LEN=${#1}

        # Copy the string to the heap
        J=0
        while [ $J -ne $LEN ]; do
            CUT_POS=$(( $J + 1 ))
            BYTE=$(printf %d "\"$(echo "$1" | cut -c$CUT_POS-$CUT_POS)")
            store_byte $(( $CURRENT_ADDRESS + $J )) "$BYTE"
            J=$(($J + 1))
        done
        store_byte $(( $CURRENT_ADDRESS + $LEN )) 0

        # Write the string's address to the array
        store_word $(( $COPY_STRINGS_RET + $I * 4 )) $CURRENT_ADDRESS
        CURRENT_ADDRESS=$(( $CURRENT_ADDRESS + $LEN + 1 ))

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
    CURRENT_ADDRESS=$(( $CURRENT_ADDRESS + 36 ))

    # Generate the halt code
    EXIT_ADDRESS=$CURRENT_ADDRESS
    CURRENT_ADDRESS=$(( $CURRENT_ADDRESS + 4 ))
    store_word $EXIT_ADDRESS 127   # 0x0000007E == sys halt 0 0

    # Fill the process info table
    store_word $PROCESS_INFO_TABLE 0             # version
    # break is set in program_init()
    store_word $(( $PROCESS_INFO_TABLE +  8 )) $EXIT_ADDRESS   # exit address
    store_word $(( $PROCESS_INFO_TABLE + 12 )) 0               # stdin
    store_word $(( $PROCESS_INFO_TABLE + 16 )) 1               # stdout
    store_word $(( $PROCESS_INFO_TABLE + 20 )) 2               # stderr
    store_word $(( $PROCESS_INFO_TABLE + 32 )) 1               # wrapper style (TODO support it)
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
}

syscall() {
    case $1 in
        00)
            exit $(( $REGISTER_0 & 0xFF ))
            ;;
        10) syscall_open ;;
        11) syscall_close ;;
        12) syscall_read ;;
        13) syscall_write ;;
        *)
            fatal "Unhandled syscall: $1"
            ;;
    esac
}



################################################################
# Instruction Execution
################################################################

parse_register() {
    PARSE_REGISTER_RET=$(( 0x$1 - 0x80 ))
    if [ $PARSE_REGISTER_RET -lt 0 -o $PARSE_REGISTER_RET -ge 16 ]; then
        fatal "A register-type argument is invalid."
    fi
}

parse_mix() {
    PARSE_MIX_RET=$(( 0x$1 ))
    if [ $PARSE_MIX_RET -ge 144 ]; then
        PARSE_MIX_RET=$(( ($PARSE_MIX_RET - 256) & 0xFFFFFFFF ))
    elif [ $PARSE_MIX_RET -ge 128 ]; then
        register_get $(( $PARSE_MIX_RET - 0x80 ))
        PARSE_MIX_RET=$REGISTER_GET_RET
    elif [ $PARSE_MIX_RET -ge 112 ]; then
        fatal "A mix-type byte cannot contain an opcode."
    fi
}

run() {
    while true; do
        #echo "about to load instruction word at $REGISTER_15"
        load_word $REGISTER_15
        HEX_INSTRUCTION=$(printf %08X $LOAD_WORD_RET)
        OPCODE=$(echo $HEX_INSTRUCTION|cut -c 7-8)
        ARG1=$(echo $HEX_INSTRUCTION|cut -c 5-6)
        ARG2=$(echo $HEX_INSTRUCTION|cut -c 3-4)
        ARG3=$(echo $HEX_INSTRUCTION|cut -c 1-2)
        #echo >&2
        #echo "instruction $OPCODE $ARG1 $ARG2 $ARG3" >&2
#        if [ $OPCODE = "00" ]; then
#            registers_print
#        fi
        REGISTER_15=$(( $REGISTER_15 + 4 ))

        case $OPCODE in
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
                #echo xor >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                register_set $PARSE_REGISTER_RET $(( ($ARG2 ^ $PARSE_MIX_RET) & 0xFFFFFFFF ))
                ;;
            77)
                #echo ror >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                ARG3=$(( $PARSE_MIX_RET & 0x1F ))
                # We only have a signed shift. We shift down by 1, mask out the
                # high bit, then shift the rest.
                if [ $ARG3 -eq 0 ]; then
                    register_set $PARSE_REGISTER_RET $ARG2
                else
                    #echo $ARG2 $ARG3 >&2
                    register_set $PARSE_REGISTER_RET \
                            $(( ((($ARG2 >> 1) & 0x7FFFFFFF) >> ($ARG3 - 1)) | \
                                (($ARG2 << (32 - $ARG3)) & 0xFFFFFFFF) ))
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
                register_set $PARSE_REGISTER_RET $(( ($REGISTER_GET_RET << 16) & 0xFFFFFFFF | 0x$ARG3$ARG2))
                ;;
            7D)
                #echo cmpu >&2
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
                    CMPU=$(( 0xFFFFFFFF ))
                elif [ $ARG2 -lt 0 -a $ARG3 -ge 0 ]; then
                    CMPU=1
                elif [ $ARG2 -lt $ARG3 ]; then
                    CMPU=$(( 0xFFFFFFFF ))
                elif [ $ARG2 -gt $ARG3 ]; then
                    CMPU=1
                else
                    CMPU=0
                fi

                register_set $PARSE_REGISTER_RET $CMPU
                ;;

            7E)
                #echo jz >&2
                parse_mix $ARG1
                if [ 0 -eq $PARSE_MIX_RET ]; then
                    OFFSET=$(( 0x$ARG3$ARG2 ))
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
                if [ $(( 0x$ARG2 )) -ne 0 -o $(( 0x$ARG3 )) -ne 0 ]; then
                    fatal "The additional arguments to the sys instruction must be zero."
                fi
                syscall $ARG1
                ;;
            *)
                fatal "Invalid opcode: $OPCODE"
        esac
    done
}

registers_init
backing_init
cache_init
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
