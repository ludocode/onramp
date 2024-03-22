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
		$(ROOT)/core/libc/0-oo/src/spawn.oo \
		\
		$(OUT)/malloc.oo \
		$(OUT)/strtol.oo \
		\
		$(OUT)/assert.oo \
		$(OUT)/constructors.oo \
		$(OUT)/ctype.oo \
		$(OUT)/environ.oo \
		$(OUT)/file.oo \
		$(OUT)/format.oo \
		$(OUT)/io.oo \
		$(OUT)/math64.oo \
		$(OUT)/string.oo \
		$(OUT)/syscalls.oo \
		$(OUT)/system.oo \

# TODO remove -nostddef and macros once we have full cci, full cpp
CCARGS=\
	    -v \
	    -with-cpp=$(TOOL_CPP) \
	    -with-cci=$(TOOL_CCI) \
	    -with-as=$(TOOL_AS) \
	    -with-ld=$(TOOL_LD) \
		-nostddef \
		-D__onramp__=1 \
		-D__onramp_cpp__=1 -D__onramp_cpp_omc__=1 \
		-D__onramp_cci__=1 -D__onramp_cci_opc__=1 \
		-I../../../core/libc/2-opc/include \
		-I../../../core/libc/1-omc/include \
		-I../../../core/libc/0-oo/include \
		-include __onramp/__predef.h

all: build test FORCE
FORCE:
build: $(OUT)/libc.oa FORCE

clean: FORCE
	rm -rf $(BUILD)/libc-0-oo

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


$(OUT)/malloc.oo: $(SRC1)/malloc.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC1)/malloc.c -o $@

$(OUT)/strtol.oo: $(SRC1)/strtol.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC1)/strtol.c -o $@


$(OUT)/assert.oo: $(SRC2)/assert.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/assert.c -o $@

$(OUT)/constructors.oo: $(SRC2)/constructors.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/constructors.c -o $@

$(OUT)/ctype.oo: $(SRC2)/ctype.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/ctype.c -o $@

$(OUT)/environ.oo: $(SRC2)/environ.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/environ.c -o $@

$(OUT)/file.oo: $(SRC2)/file.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/file.c -o $@

$(OUT)/format.oo: $(SRC2)/format.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/format.c -o $@

$(OUT)/io.oo: $(SRC2)/io.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/io.c -o $@

$(OUT)/math64.oo: $(SRC2)/math64.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/math64.c -o $@

$(OUT)/string.oo: $(SRC2)/string.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/string.c -o $@

$(OUT)/syscalls.oo: $(SRC2)/syscalls.os Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/syscalls.os -o $@

$(OUT)/system.oo: $(SRC2)/system.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC2)/system.c -o $@


$(OUT)/libc.oa: Makefile $(TOOLS) $(OBJS)
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_VM) $(TOOL_AR) rc $@ $(OBJS)

test: build FORCE $(VM)
	../run.sh . $(OUT)/libc.oa \
		-I$(ROOT)/core/libc/1-opc/include \
		-I$(ROOT)/core/libc/1-omc/include \
		-I$(ROOT)/core/libc/0-oo/include
	../run.sh ../1-omc $(OUT)/libc.oa \
		-I$(ROOT)/core/libc/1-omc/include \
		-I$(ROOT)/core/libc/0-oo/include
	../run.sh ../0-oo $(OUT)/libc.oa \
		-I$(ROOT)/core/libc/0-oo/include
