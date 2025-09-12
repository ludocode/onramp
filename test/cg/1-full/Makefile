# The MIT License (MIT)
#
# Copyright (c) 2025-2026 Fraser Heavy Software
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
BUILD=$(ROOT)/output/test

OUT=$(BUILD)/cg-1-full
SRC=$(ROOT)/core/cg/1-full
LIBO=$(ROOT)/core/libo/1-opc
CCI2=$(ROOT)/output/test/cci-2-full/cci

CPPFLAGS += \
	-Wno-unused-function \
	-include ../../portability.h \
	-I$(LIBO)/include

SRCS=\
	$(LIBO)/src/libo-error.c \
	$(LIBO)/src/libo-reader.c \
	$(LIBO)/src/libo-string.c \
	$(LIBO)/src/libo-table.c \
	$(LIBO)/src/libo-unicode.c \
	$(LIBO)/src/libo-util.c \
	$(LIBO)/src/libo-vector.c \
	\
	$(SRC)/src/argument.c \
	$(SRC)/src/block.c \
	$(SRC)/src/common.c \
	$(SRC)/src/emit.c \
	$(SRC)/src/instruction.c \
	$(SRC)/src/location.c \
	$(SRC)/src/main.c \
	$(SRC)/src/opcode.c \
	$(SRC)/src/optimize.c \
	$(SRC)/src/parse.c \
	$(SRC)/src/symbol.c \

HEADERS=\
	$(SRC)/src/argument.h \
	$(SRC)/src/block.h \
	$(SRC)/src/common.h \
	$(SRC)/src/emit.h \
	$(SRC)/src/instruction.h \
	$(SRC)/src/location.h \
	$(SRC)/src/opcode.h \
	$(SRC)/src/optimize.h \
	$(SRC)/src/parse.h \
	$(SRC)/src/symbol.h \

all: build test FORCE
FORCE:
build: $(OUT)/cg FORCE

clean: FORCE
	rm -rf $(OUT)

$(OUT)/cg: $(SRCS) $(HEADERS) Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRCS) -o $@

$(CCI2): FORCE
	@(make -C ../../cci/2-full build-ir)

test-cg1: build FORCE
	( cd ../../.. && test/cg/run.sh \
			--tests test/cg/1-full \
			--output output/test/cg-1-full \
			--cg output/test/cg-1-full/cg )

test-cci0: build $(CCI2) FORCE
	( cd ../../.. && test/cci/run.sh \
			--tests test/cci/0-omc \
			--output output/test/cg-1-full \
			--cg output/test/cg-1-full/cg \
			--cci output/test/cci-2-full/cci-ir --cci-id full )

test-cci1: build $(CCI2) FORCE
	( cd ../../.. && test/cci/run.sh \
			--tests test/cci/1-opc \
			--output output/test/cg-1-full \
			--cg output/test/cg-1-full/cg \
			--cci output/test/cci-2-full/cci-ir --cci-id full )

test-cci2: build $(CCI2) FORCE
	( cd ../../.. && test/cci/run.sh \
			--tests test/cci/2-full \
			--nonstd \
			--output output/test/cg-1-full \
			--cg output/test/cg-1-full/cg \
			--cci output/test/cci-2-full/cci-ir --cci-id full )

test: test-cg1 test-cci0 test-cci1 test-cci2
