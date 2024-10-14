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

CFLAGS := -g
CPPFLAGS := -Wall -Wextra -Wpedantic

-include ../../local.mk

ROOT=../../..
BUILD=$(ROOT)/build/test

OUT=$(BUILD)/cci-1-opc
SRC=$(ROOT)/core/cci/1-opc/src
LIBO=$(ROOT)/core/libo/1-opc
CPPFLAGS += \
	-include $(ROOT)/test/portability.h \
	-I$(LIBO)/include \
	-D__onramp_omc__=1

SRCS=\
	$(LIBO)/src/libo-error.c \
	$(LIBO)/src/libo-string.c \
	$(LIBO)/src/libo-table.c \
	$(LIBO)/src/libo-util.c \
	$(LIBO)/src/libo-vector.c \
	\
	$(SRC)/common.c \
	$(SRC)/compile.c \
	$(SRC)/emit.c \
	$(SRC)/member.c \
	$(SRC)/global.c \
	$(SRC)/lexer.c \
	$(SRC)/locals.c \
	$(SRC)/main.c \
	$(SRC)/parse-decl.c \
	$(SRC)/parse-expr.c \
	$(SRC)/parse-stmt.c \
	$(SRC)/record.c \
	$(SRC)/type.c \
	$(SRC)/types.c \

all: build test FORCE
FORCE:
build: $(OUT)/cci FORCE

clean: FORCE
	rm -rf $(OUT)

$(OUT)/cci: $(SRCS) Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRCS) -o $@

test-0: build FORCE
	../run.sh ../0-omc opc $(OUT)/cci

test-1: build FORCE
	../run.sh --nonstd . opc $(OUT)/cci

test: test-0 test-1 FORCE

generate: build FORCE $(VM)
	../generate.sh . opc $(VM) $(OUT)/cci
