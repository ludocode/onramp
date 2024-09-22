# The MIT License (MIT)
#
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

.SUFFIXES:
CFLAGS := -g
CPPFLAGS := -Wall -Wextra -Wpedantic

-include ../../local.mk

ROOT=../../..
BUILD=$(ROOT)/build/test

OUT=$(BUILD)/libo-1-opc
SRC=$(ROOT)/core/libo/1-opc/src

TOOL_VM=$(BUILD)/vm-c-debugger/vm
TOOL_AR=$(BUILD)/ar-0-cat/ar.oe    # TODO ar/1
TOOL_CC=$(BUILD)/cc/cc
TOOL_CPP=$(BUILD)/cpp-1-omc/cpp    # TODO cpp/2
TOOL_CCI=$(BUILD)/cci-2-full/cci
TOOL_AS=$(BUILD)/as-2-full/as
TOOL_LD=$(BUILD)/ld-2-full/ld
TOOLS=$(TOOL_VM) $(TOOL_AR) $(TOOL_CC) $(TOOL_CCI) $(TOOL_CPP) $(TOOL_AS) $(TOOL_LD)

OBJS=\
		$(OUT)/libo-data.oo \
		$(OUT)/libo-error.oo \
		$(OUT)/libo-string.oo \
		$(OUT)/libo-table.oo \
		$(OUT)/libo-util.oo \
		$(OUT)/libo-vector.oo \

SRCS=\
		$(SRC)/libo-error.c \
		$(SRC)/libo-string.c \
		$(SRC)/libo-table.c \
		$(SRC)/libo-util.c \
		$(SRC)/libo-vector.c \

# Arguments for Onramp compiler
CCARGS=\
	    -with-cpp=$(TOOL_CPP) \
	    -with-cci=$(TOOL_CCI) \
	    -with-as=$(TOOL_AS) \
	    -with-ld=$(TOOL_LD) \
		-nostdinc \
		-nostdlib \
		-I$(ROOT)/core/libc/common/include \
		-I$(ROOT)/core/libo/1-opc/include \
		-include __onramp/__predef.h

# Arguments for external compiler
CPPFLAGS += \
		-nostdinc -nostdlib \
		-I$(ROOT)/core/libc/common/include \
		-I$(ROOT)/core/libo/1-opc/include \
		-include __onramp/__predef.h \
		-include $(ROOT)/test/portability.h

all: build test FORCE
FORCE:
build: $(OUT)/libo.oa FORCE

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
	@(make -C ../../cci/2-full build)
$(TOOL_CPP): FORCE
	@(make -C ../../cpp/1-omc build)   # TODO cpp/2 build
$(TOOL_AS): FORCE
	@(make -C ../../as/2-full build)
$(TOOL_LD): FORCE
	@(make -C ../../ld/2-full build)


$(OUT)/libo-data.oo: $(SRC)/libo-data.os Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC)/libo-data.os -o $@


# Each C file is compiled with the external C compiler first because it has
# much better error checking than our Onramp compiler. We can't actually link
# the resulting objects (at least, not unless we want to implement all of our
# syscalls for Linux.)

$(OUT)/libo-data.oo: $(SRC)/libo-data.os Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	# no C file to compile
	$(TOOL_CC) $(CCARGS) -c $(SRC)/libo-data.os -o $@

$(OUT)/libo-error.oo: $(SRC)/libo-error.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) -c $(SRC)/libo-error.c -o $(OUT)/libo-error.o
	$(TOOL_CC) $(CCARGS) -c $(SRC)/libo-error.c -o $@

$(OUT)/libo-string.oo: $(SRC)/libo-string.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) -c $(SRC)/libo-string.c -o $(OUT)/libo-string.o
	$(TOOL_CC) $(CCARGS) -c $(SRC)/libo-string.c -o $@

$(OUT)/libo-table.oo: $(SRC)/libo-table.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) -c $(SRC)/libo-table.c -o $(OUT)/libo-table.o
	$(TOOL_CC) $(CCARGS) -c $(SRC)/libo-table.c -o $@

$(OUT)/libo-util.oo: $(SRC)/libo-util.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) -c $(SRC)/libo-util.c -o $(OUT)/libo-util.o
	$(TOOL_CC) $(CCARGS) -c $(SRC)/libo-util.c -o $@

$(OUT)/libo-vector.oo: $(SRC)/libo-vector.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) -c $(SRC)/libo-vector.c -o $(OUT)/libo-vector.o
	$(TOOL_CC) $(CCARGS) -c $(SRC)/libo-vector.c -o $@


$(OUT)/libo.oa: Makefile $(TOOLS) $(OBJS)
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_VM) $(TOOL_AR) rc $@ $(OBJS)

test: build FORCE
	../run.sh . $(OUT)/libo.oa
	../run.sh ../0-oo $(OUT)/libo.oa
