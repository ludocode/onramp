# The MIT License (MIT)
#
# Copyright (c) 2023-2026 Fraser Heavy Software
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
CPPFLAGS := -Wall -Wextra -Wpedantic -DDEBUG

-include ../../local.mk

ROOT=../../..
BUILD=$(ROOT)/output/test

OUT=$(BUILD)/cci-2-full
SRC=$(ROOT)/core/cci/2-full/src
LIBO=$(ROOT)/core/libo/1-opc
CPPFLAGS += \
	-include $(ROOT)/test/portability.h \
	-I$(LIBO)/include

SRCS=\
	$(LIBO)/src/libo-error.c \
	$(LIBO)/src/libo-otable.c \
	$(LIBO)/src/libo-string.c \
	$(LIBO)/src/libo-table.c \
	$(LIBO)/src/libo-util.c \
	$(LIBO)/src/libo-vector.c \
	\
	$(SRC)/arithmetic.c \
	$(SRC)/block.c \
	$(SRC)/common.c \
	$(SRC)/emit.c \
	$(SRC)/enum.c \
	$(SRC)/function.c \
	$(SRC)/generate.c \
	$(SRC)/generate_ops.c \
	$(SRC)/generate_stmt.c \
	$(SRC)/instruction.c \
	$(SRC)/lexer.c \
	$(SRC)/main.c \
	$(SRC)/node.c \
	$(SRC)/optimize_asm.c \
	$(SRC)/optimize_tree.c \
	$(SRC)/options.c \
	$(SRC)/parse_decl.c \
	$(SRC)/parse_expr.c \
	$(SRC)/parse_init.c \
	$(SRC)/parse_stmt.c \
	$(SRC)/record.c \
	$(SRC)/scope.c \
	$(SRC)/strings.c \
	$(SRC)/symbol.c \
	$(SRC)/token.c \
	$(SRC)/type.c \

all: build test FORCE
FORCE:
build: $(OUT)/cci FORCE

CG: FORCE
	make -C ../../cg/1-full build

clean: FORCE
	rm -rf $(OUT)

$(OUT)/cci: $(SRCS) Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRCS) -o $@

test-0: build CG FORCE
	( cd ../../.. && test/cci/run.sh \
			--tests test/cci/0-omc \
			--output output/test/cci-2-full \
			--cg output/test/cg-1-full/cg \
			--cci output/test/cci-2-full/cci --cci-id full )

test-1: build CG FORCE
	( cd ../../.. && test/cci/run.sh \
			--tests test/cci/1-opc \
			--output output/test/cci-2-full \
			--cg output/test/cg-1-full/cg \
			--cci output/test/cci-2-full/cci --cci-id full )

test-2: build CG FORCE
	( cd ../../.. && test/cci/run.sh \
			--tests test/cci/2-full \
			--nonstd \
			--output output/test/cci-2-full \
			--cg output/test/cg-1-full/cg \
			--cci output/test/cci-2-full/cci --cci-id full )

test: test-0 test-1 test-2 FORCE
