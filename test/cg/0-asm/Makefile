# The MIT License (MIT)
#
# Copyright (c) 2025 Fraser Heavy Software
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

CFLAGS := -g
CPPFLAGS := -Wall -Wextra -Wpedantic

-include ../../local.mk

ROOT=../../..
BUILD=$(ROOT)/build/test

OUT=$(BUILD)/cg-0-asm
SRC=$(ROOT)/core/cg/0-asm
LIBO=$(ROOT)/core/libo/1-opc
CCI0=$(ROOT)/build/test/cci-0-omc/cci.oe
CCI1=$(ROOT)/build/test/cci-1-opc/cci

CPPFLAGS += \
	-Wno-unused-function \
	-include ../../portability.h \
	-I$(LIBO)/include \
	-D__onramp_omc__=1

SRCS=\
	$(LIBO)/src/libo-error.c \
	$(LIBO)/src/libo-string.c \
	$(LIBO)/src/libo-table.c \
	$(LIBO)/src/libo-util.c \
	\
	$(SRC)/src/main.c \

HEADERS=\
	$(SRC)/src/common.h \
	$(SRC)/src/parse.h \
	$(SRC)/src/instruction.h \
	$(SRC)/src/opcode.h \
	$(SRC)/src/optimize.h \
	$(SRC)/src/register.h \

all: build FORCE test-cci1-0 test-cci1-1 # TODO test-cci0-0 not passing yet
FORCE:
build: $(OUT)/cg FORCE

clean: FORCE
	rm -rf $(OUT)

$(OUT)/cg: $(SRCS) $(HEADERS) Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRCS) -o $@

$(CCI0): FORCE
	@(make -C ../../cci/0-omc build)

$(CCI1): FORCE
	@(make -C ../../cci/1-opc build)

test-cci0-0: build $(CCI0) FORCE
	( cd ../../.. && test/cci/run.sh \
			--tests test/cci/0-omc \
			--nonstd \
			--output build/test/cg-0-asm/cci0 \
			--cg build/test/cg-0-asm/cg \
			--cci build/test/cci-2-full/cci --cci-id full )

test-cci1-0: build $(CCI1) FORCE
	( cd ../../.. && test/cci/run.sh \
			--tests test/cci/0-omc \
			--output build/test/cg-0-asm/cci1 \
			--cci build/test/cci-1-opc/cci --cci-id opc )

test-cci1-1: build $(CCI1) FORCE
	( cd ../../.. && test/cci/run.sh \
			--tests test/cci/1-opc \
			--nonstd \
			--output build/test/cg-0-asm/cci1 \
			--cci build/test/cci-1-opc/cci --cci-id opc )

test: test-cci0-0 test-cci1-0 test-cci1-1
