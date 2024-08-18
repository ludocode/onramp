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

SRC1=$(ROOT)/core/libc/1-omc/src
SRC2=$(ROOT)/core/libc/2-opc/src
OUT=$(BUILD)/libc-2-opc

TOOL_VM=$(BUILD)/vm-c-debugger/vm
TOOL_AR=$(BUILD)/ar-0-cat/ar.oe    # TODO ar/1
TOOL_CC=$(BUILD)/cc/cc
TOOL_CPP=$(BUILD)/cpp-1-omc/cpp    # TODO cpp/2
TOOL_CCI=$(BUILD)/cci-1-opc/cci    # TODO cci/2
TOOL_AS=$(BUILD)/as-2-full/as
TOOL_LD=$(BUILD)/ld-2-full/ld
TOOLS=$(TOOL_VM) $(TOOL_AR) $(TOOL_CC) $(TOOL_CCI) $(TOOL_CPP) $(TOOL_AS) $(TOOL_LD)

OBJS=\
		$(OUT)/start.oo \
		\
		$(ROOT)/core/libc/0-oo/src/errno.oo \
		$(ROOT)/core/libc/0-oo/src/malloc_util.oo \
		$(ROOT)/core/libc/0-oo/src/spawn.oo \
		\
		$(OUT)/malloc.oo \
		$(OUT)/strtol.oo \
		\
		$(OUT)/setjmp.oo \
		$(OUT)/syscalls.oo \
		\
		$(OUT)/assert.oo \
		$(OUT)/ctype.oo \
		$(OUT)/environ.oo \
		$(OUT)/file.oo \
		$(OUT)/format.oo \
		$(OUT)/io.oo \
		$(OUT)/llong.oo \
		$(OUT)/string.oo \
		$(OUT)/strings.oo \
		$(OUT)/system.oo \

# Arguments for Onramp compiler
# TODO remove -D__onramp_cci_opc__ etc. once we have full cci, full cpp
CCARGS=\
	    -with-cpp=$(TOOL_CPP) \
	    -with-cci=$(TOOL_CCI) \
	    -with-as=$(TOOL_AS) \
	    -with-ld=$(TOOL_LD) \
		-nostdinc \
		-nostdlib \
		-D__onramp_cci_opc__=1 \
		-D__onramp_libc_opc__=1 \
		-I$(ROOT)/core/libc/common/include \
		-include __onramp/__predef.h

# Arguments for external compiler
CPPFLAGS += \
		-nostdinc -nostdlib \
		-I$(ROOT)/core/libc/common/include \
		-include $(ROOT)/test/portability.h \
		-include __onramp/__predef.h

all: build test FORCE
FORCE:
build: $(OUT)/libc.oa FORCE

clean: FORCE
	rm -rf $(OUT)

# tools
$(TOOL_VM): FORCE
	@(make -C $(ROOT)/platform/vm/c-debugger build)
$(TOOL_AR): FORCE
	@(make -C ../../ar/0-cat build)    # TODO ar/1
$(TOOL_CC): FORCE
	@(make -C ../../cc build)
$(TOOL_CCI): FORCE
	@(make -C ../../cci/1-opc build)   # TODO cci/2 build
$(TOOL_CPP): FORCE
	@(make -C ../../cpp/1-omc build)   # TODO cpp/2 build
$(TOOL_AS): FORCE
	@(make -C ../../as/2-full build)
$(TOOL_LD): FORCE
	@(make -C ../../ld/2-full build)


$(OUT)/start.oo: $(SRC2)/start.os Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/start.os -o $@

$(OUT)/setjmp.oo: $(SRC2)/setjmp.os Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/setjmp.os -o $@

$(OUT)/syscalls.oo: $(SRC2)/syscalls.os Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/syscalls.os -o $@


# Each C file is compiled with the external C compiler first because it has
# much better error checking than our Onramp compiler. We can't actually link
# the resulting objects (at least, not unless we want to implement all of our
# syscalls for Linux.)


$(OUT)/malloc.oo: $(SRC1)/malloc.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) -c $(SRC1)/malloc.c -o $(OUT)/malloc.o
	$(TOOL_CC) $(CCARGS) -c $(SRC1)/malloc.c -o $@

$(OUT)/strtol.oo: $(SRC1)/strtol.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) -c $(SRC1)/strtol.c -o $(OUT)/strtol.o
	$(TOOL_CC) $(CCARGS) -c $(SRC1)/strtol.c -o $@


$(OUT)/assert.oo: $(SRC2)/assert.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) -c $(SRC2)/assert.c -o $(OUT)/assert.o
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/assert.c -o $@

$(OUT)/ctype.oo: $(SRC2)/ctype.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) -c $(SRC2)/ctype.c -o $(OUT)/ctype.o
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/ctype.c -o $@

$(OUT)/environ.oo: $(SRC2)/environ.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) -c $(SRC2)/environ.c -o $(OUT)/environ.o
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/environ.c -o $@

$(OUT)/file.oo: $(SRC2)/file.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) -c $(SRC2)/file.c -o $(OUT)/file.o
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/file.c -o $@

$(OUT)/format.oo: $(SRC2)/format.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) -c $(SRC2)/format.c -o $(OUT)/format.o
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/format.c -o $@

$(OUT)/io.oo: $(SRC2)/io.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) -c $(SRC2)/io.c -o $(OUT)/io.o
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/io.c -o $@

$(OUT)/llong.oo: $(SRC2)/llong.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) -c $(SRC2)/llong.c -o $(OUT)/llong.o
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/llong.c -o $@

$(OUT)/string.oo: $(SRC2)/string.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) -c $(SRC2)/string.c -o $(OUT)/string.o
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/string.c -o $@

$(OUT)/strings.oo: $(SRC2)/strings.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) -c $(SRC2)/strings.c -o $(OUT)/strings.o
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/strings.c -o $@

$(OUT)/system.oo: $(SRC2)/system.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) -c $(SRC2)/system.c -o $(OUT)/system.o
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/system.c -o $@


$(OUT)/libc.oa: Makefile $(TOOLS) $(OBJS)
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_VM) $(TOOL_AR) rc $@ $(OBJS)

test: build FORCE $(VM)
	../run.sh ../0-oo  opc $(OUT)/libc.oa
	../run.sh ../1-omc opc $(OUT)/libc.oa
	../run.sh .        opc $(OUT)/libc.oa
