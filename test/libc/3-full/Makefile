# The MIT License (MIT)
#
# Copyright (c) 2024 Fraser Heavy Software
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

.SUFFIXES:
CFLAGS := -g
CPPFLAGS := -Wall -Wextra -Wpedantic

-include ../../local.mk

ROOT=../../..
BUILD=$(ROOT)/build/test

BUILD1=$(BUILD)/libc-1-omc
BUILD2=$(BUILD)/libc-2-opc
BUILD3=$(BUILD)/libc-3-full

SRC1=$(ROOT)/core/libc/1-omc/src
SRC2=$(ROOT)/core/libc/2-opc/src
SRC3=$(ROOT)/core/libc/3-full/src

LIBC1=$(BUILD)/libc-1-omc/libc.oa
LIBC2=$(BUILD)/libc-2-opc/libc.oa

TOOL_VM=$(BUILD)/vm-c-debugger/vm
TOOL_AR=$(BUILD)/ar-0-cat/ar.oe    # TODO ar/1
TOOL_CC=$(BUILD)/cc/cc
TOOL_CPP=$(BUILD)/cpp-1-omc/cpp    # TODO cpp/2
TOOL_CCI=$(BUILD)/cci-2-full/cci
TOOL_AS=$(BUILD)/as-2-full/as
TOOL_LD=$(BUILD)/ld-2-full/ld
TOOLS=$(TOOL_VM) $(TOOL_AR) $(TOOL_CC) $(TOOL_CCI) $(TOOL_CPP) $(TOOL_AS) $(TOOL_LD)

OBJS3=\
		$(BUILD3)/atexit.oo \
		$(BUILD3)/bsearch.oo \
		$(BUILD3)/malloc.oo \
		$(BUILD3)/qsort.oo \
		$(BUILD3)/rand.oo \
		$(BUILD3)/stdbit_llong.oo \
		$(BUILD3)/stdlib_3.oo \
		$(BUILD3)/time.oo \

# TODO remove libc/1 malloc.oo when libc/3 builds
OBJS_ALL=\
		$(BUILD2)/start.oo \
		\
		$(ROOT)/core/libc/0-oo/src/errno.oo \
		$(ROOT)/core/libc/0-oo/src/malloc_util.oo \
		$(ROOT)/core/libc/0-oo/src/spawn.oo \
		\
		$(BUILD1)/malloc.oo \
		$(BUILD1)/strtol.oo \
		\
		$(BUILD2)/assert.oo \
		$(BUILD2)/ctype.oo \
		$(BUILD2)/environ.oo \
		$(BUILD2)/file.oo \
		$(BUILD2)/format.oo \
		$(BUILD2)/io.oo \
		$(BUILD2)/llong.oo \
		$(BUILD2)/multibyte.oo \
		$(BUILD2)/setjmp.oo \
		$(BUILD2)/stdbit.oo \
		$(BUILD2)/stdlib.oo \
		$(BUILD2)/string.oo \
		$(BUILD2)/strings.oo \
		$(BUILD2)/syscalls.oo \
		$(BUILD2)/system.oo \
		\
		$(OBJS3)

# Arguments for Onramp compiler
CCARGS=\
	    -with-cpp=$(TOOL_CPP) \
	    -with-cci=$(TOOL_CCI) \
	    -with-as=$(TOOL_AS) \
	    -with-ld=$(TOOL_LD) \
		-nostdinc \
		-nostdlib \
		-I$(ROOT)/core/libc/common/include \
		-I$(ROOT)/core/libc/common/src \
		-include __onramp/__predef.h

# Arguments for external compiler
CPPFLAGS += \
		-nostdinc -nostdlib \
		-D__onramp_libc__=1 \
		-D__onramp_libc_opc__=1 \
		-I$(ROOT)/core/libc/common/include \
		-I$(ROOT)/core/libc/common/src \
		-include $(ROOT)/test/portability.h \
		-include __onramp/__predef.h

all: build test FORCE
FORCE:
build: $(BUILD3)/libc.oa FORCE

clean: FORCE
	rm -rf $(BUILD3)

# dependencies
$(TOOL_VM): FORCE
	@(make -C $(ROOT)/platform/vm/c-debugger build)
$(TOOL_AR): FORCE
	@(make -C ../../ar/0-cat build)    # TODO ar/1
$(TOOL_CC): FORCE
	@(make -C ../../cc build)
$(TOOL_CCI): FORCE
	@(make -C ../../cci/2-full build)
$(TOOL_CPP): FORCE
	@(make -C ../../cpp/1-omc build)   # TODO cpp/2 build
$(TOOL_AS): FORCE
	@(make -C ../../as/2-full build)
$(TOOL_LD): FORCE
	@(make -C ../../ld/2-full build)
$(LIBC1): FORCE
	@(make -C ../../libc/1-omc build)
$(LIBC2): FORCE
	@(make -C ../../libc/2-opc build)


# Each C file is compiled with the external C compiler first because it has
# much better error checking than our Onramp compiler. We can't actually link
# the resulting objects (at least, not unless we want to implement all of our
# syscalls for Linux.)


$(BUILD3)/atexit.oo: $(SRC3)/atexit.c Makefile
	@rm -f $@
	@mkdir -p $(BUILD3)
	$(CC) $(CPPFLAGS) -c $(SRC3)/atexit.c -o $(BUILD3)/atexit.o
	$(TOOL_CC) $(CCARGS) -c $(SRC3)/atexit.c -o $@

$(BUILD3)/bsearch.oo: $(SRC3)/bsearch.c Makefile
	@rm -f $@
	@mkdir -p $(BUILD3)
	$(CC) $(CPPFLAGS) -c $(SRC3)/bsearch.c -o $(BUILD3)/bsearch.o
	$(TOOL_CC) $(CCARGS) -c $(SRC3)/bsearch.c -o $@

$(BUILD3)/malloc.oo: $(SRC3)/malloc.c Makefile
	@rm -f $@
	@mkdir -p $(BUILD3)
	$(CC) $(CPPFLAGS) -c $(SRC3)/malloc.c -o $(BUILD3)/malloc.o
	$(TOOL_CC) $(CCARGS) -c $(SRC3)/malloc.c -o $@

$(BUILD3)/qsort.oo: $(SRC3)/qsort.c Makefile
	@rm -f $@
	@mkdir -p $(BUILD3)
	$(CC) $(CPPFLAGS) -c $(SRC3)/qsort.c -o $(BUILD3)/qsort.o
	$(TOOL_CC) $(CCARGS) -c $(SRC3)/qsort.c -o $@

$(BUILD3)/rand.oo: $(SRC3)/rand.c Makefile
	@rm -f $@
	@mkdir -p $(BUILD3)
	$(CC) $(CPPFLAGS) -c $(SRC3)/rand.c -o $(BUILD3)/rand.o
	$(TOOL_CC) $(CCARGS) -c $(SRC3)/rand.c -o $@

$(BUILD3)/stdbit_llong.oo: $(SRC3)/stdbit_llong.c Makefile
	@rm -f $@
	@mkdir -p $(BUILD3)
	$(CC) $(CPPFLAGS) -c $(SRC3)/stdbit_llong.c -o $(BUILD3)/stdbit_llong.o
	$(TOOL_CC) $(CCARGS) -c $(SRC3)/stdbit_llong.c -o $@

$(BUILD3)/stdlib_3.oo: $(SRC3)/stdlib_3.c Makefile
	@rm -f $@
	@mkdir -p $(BUILD3)
	$(CC) $(CPPFLAGS) -c $(SRC3)/stdlib_3.c -o $(BUILD3)/stdlib_3.o
	$(TOOL_CC) $(CCARGS) -c $(SRC3)/stdlib_3.c -o $@

$(BUILD3)/time.oo: $(SRC3)/time.c Makefile
	@rm -f $@
	@mkdir -p $(BUILD3)
	$(CC) $(CPPFLAGS) -c $(SRC3)/time.c -o $(BUILD3)/time.o
	$(TOOL_CC) $(CCARGS) -c $(SRC3)/time.c -o $@


$(BUILD3)/libc.oa: Makefile $(TOOLS) $(LIBC1) $(LIBC2) $(OBJS3)
	@rm -f $@
	@mkdir -p $(BUILD3)
	$(TOOL_VM) $(TOOL_AR) rc $@ $(OBJS_ALL)

test: build FORCE $(VM)
	../run.sh ../0-oo  full $(BUILD3)/libc.oa
	../run.sh ../1-omc full $(BUILD3)/libc.oa
	../run.sh ../2-opc full $(BUILD3)/libc.oa
	../run.sh .        full $(BUILD3)/libc.oa
