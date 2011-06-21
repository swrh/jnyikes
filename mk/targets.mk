# targets.mk
#
# $Id$
#
# Copyright (C) 2005-2011 Fernando Silveira <fsilveira@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#      This product includes software developed by Fernando Silveira.
# 4. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

ifndef targets.mk
targets.mk=		y

ifdef DEBUG
	ifeq ($(DEBUG),)
		mk_DEBUG=n
	else ifeq ($(DEBUG),n)
		mk_DEBUG=n
	else ifeq ($(DEBUG),no)
		mk_DEBUG=n
	else ifeq ($(DEBUG),N)
		mk_DEBUG=n
	else ifeq ($(DEBUG),NO)
		mk_DEBUG=n
	else
		mk_DEBUG=y
	endif
else
	mk_DEBUG=n
endif

# C compiler.
CFLAGS+=		-std=gnu99 -Wall -Werror -Wunused -fPIC -DPIC
CFLAGS+=		-Wshadow
ifeq ($(mk_DEBUG),y)
CFLAGS+=		-O0 -ggdb3 -DDEBUG
else
CFLAGS+=		-O2 -DNDEBUG
endif

# C++ compiler.
CXXFLAGS+=		-Wall -Werror -Wunused -fPIC -DPIC
CXXFLAGS+=		-Wshadow
ifeq ($(mk_DEBUG),y)
CXXFLAGS+=		-O0 -ggdb3 -DDEBUG
else
CXXFLAGS+=		-O2 -DNDEBUG
endif

# Linker.
ifeq ($(mk_DEBUG),y)
LDFLAGS+=		
else
LDFLAGS+=		
endif

# Archiver.
ARFLAGS=		rcs

# Commands.
AR=			ar
AWK=			awk
CC=			gcc
CP=			cp
CTAGS=			ctags
CXX=			g++
INSTALL=		install
MKDEP=			mkdep
MKDIR=			mkdir
RM=			rm -f
TEST=			test
TOUCH=			touch
WGET=			wget

LINK.o=			$(CC)

# Avoid unwanted default goals. The `all' goal must be redefined at the end of
# this file.
.PHONY: all
.DEFAULT_GOAL:=		all
all:

# Empty goals.
.PHONY: no not empty null
no not empty null:

# override some global rules definitions
%.c: %.y
	yacc -d -o $@ $^
%.c: %.l
	lex -o $@ $^


# global variables

ifdef LIB
LIBS+=			$(LIB)
$(LIB)_SRCS+=		$(SRCS)
$(LIB)_DEPLIBS+=	$(DEPLIBS)
endif

ifdef PROG
PROGS+=			$(PROG)
$(PROG)_SRCS+=		$(SRCS)
$(PROG)_INCDIRS+=	$(INCDIRS)
$(PROG)_LIBDIRS+=	$(LIBDIRS)
$(PROG)_DEPLIBS+=	$(DEPLIBS)
endif

ifdef SUBDIR
SUBDIRS+=		$(SUBDIR)
endif


# binaries

define BIN_template
$(1)_LINK=		$(CC)

define BIN_SRC_template

# Read ".depend" file to append dependencies to each object target.
ifneq ($(wildcard .depend),)
	# Using $(MAKE) to read file dependencies is TOO MUCH slow. Please use $(AWK).
	#$$(1)_depend=	$$$$(shell OBJ="$$(notdir $$$$(patsubst %.cpp,%.o,$$$$(1)))"; echo -e ".PHONY: $$$$$$$${OBJ}\\n$$$$$$$${OBJ}:\\n\\t@echo $$$$$$$$^\\n" | make -f - -f .depend)
	# Faster, but less compatible.
	$$(1)_depend=	$$$$(shell exec $(AWK) -v OBJ=$$(notdir $$(patsubst %.cpp,%.o,$$(1))) '{ if (/^[^ \t]/) obj = 0; if ($$$$$$$$1 == OBJ":") { obj = 1; $$$$$$$$1 = ""; } else if (!obj) next; if (/\\$$$$$$$$/) sub(/\\$$$$$$$$/, " "); else sub(/$$$$$$$$/, "\n"); printf("%s", $$$$$$$$0); }' .depend)
endif

ifneq ($$(1),$$(patsubst %.cpp,%.o,$$(1)))

$(1)_LINK=		$(CXX)

ifndef $$(1)_CXXFLAGS
$$(1)_CXXFLAGS+=	$$($(1)_CXXFLAGS) $$(CXXFLAGS)
endif

# avoid defining a target more than one time
ifneq ($$$$(_$$(1)),x)
$$(patsubst %.cpp,%.o,$$(1)): $$(1) $$$$($$(1)_DEPS) $$$$($$(1)_depend)
	$(CXX) $$$$($$(1)_CXXFLAGS) $$($(1)_INCDIRS:%=-I%) $$(INCDIRS:%=-I%) -c -o $$$$@ $$$$<
endif

$(1)_OBJS+=		$$(patsubst %.cpp,%.o,$$(1))

CXX_SRCS+=		$$(1)

else

ifndef $$(1)_CFLAGS
$$(1)_CFLAGS+=		$$($(1)_CFLAGS) $$(CFLAGS)
endif

# avoid defining a target more than one time
ifneq ($$$$(_$$(1)),x)
$$(patsubst %.c,%.o,$$(1)): $$(1) $$$$($$(1)_DEPS) $$$$($$(1)_depend)
	$(CC) $$$$($$(1)_CFLAGS) $$($(1)_INCDIRS:%=-I%) $$(INCDIRS:%=-I%) -c -o $$$$@ $$$$<
endif

$(1)_OBJS+=		$$(patsubst %.c,%.o,$$(1))

C_SRCS+=		$$(1)

endif

endef

_$$(1)=			x
$$(foreach src,$$($(1)_SRCS),$$(eval $$(call BIN_SRC_template,$$(src))))

define BIN_INCDIR_template
MKDEPARGS+=		-I$$(1) # mkdep
CTAGSARGS+=		$$(1) # ctags
endef

$$(foreach incdir,$$($(1)_INCDIRS),$$(eval $$(call BIN_INCDIR_template,$$(incdir))))

endef

$(foreach bin,$(LIBS) $(PROGS),$(eval $(call BIN_template,$(bin))))


# libraries

define LIB_template
ifndef $(1)_LDFLAGS
$(1)_LDFLAGS+=		$$(LDFLAGS)
endif

DEFAULT_TARGETS+=	lib$(1).a
STLIBS+=		lib$(1).a
lib$(1).a: $$($(1)_OBJS)
	$(AR) $(ARFLAGS) $$@ $$^

DEFAULT_TARGETS+=	lib$(1).so
SHLIBS+=		lib$(1).so
lib$(1).so: $$($(1)_OBJS)
	$$($(1)_LINK) $$^ $$($(1)_LIBDIRS:%=-L%) $$($(1)_DEPLIBS:%=-l%) $$(LIBDIRS:%=-L%) $$(DEPLIBS:%=-l%) $$($(1)_LDFLAGS) $$(LDLIBS) -o $$@ -shared

$(1): lib$(1).a lib$(1).so

CLEAN_TARGETS+=		$(1)_clean
DISTCLEAN_TARGETS+=	$(1)_clean
$(1)_clean:
	$(RM) lib$(1).a lib$(1).so $$($(1)_OBJS)
endef

$(foreach lib,$(LIBS),$(eval $(call LIB_template,$(lib))))


# programs

define PROG_template
ifndef $(1)_LDFLAGS
$(1)_LDFLAGS+=		$$(LDFLAGS)
endif

DEFAULT_TARGETS+=	$(1)
$(1): $$($(1)_OBJS)
	$$($(1)_LINK) $$^ $$($(1)_LIBDIRS:%=-L%) $$($(1)_DEPLIBS:%=-l%) $$(LIBDIRS:%=-L%) $$(DEPLIBS:%=-l%) $$($(1)_LDFLAGS) $$(LDLIBS) -o $$@

CLEAN_TARGETS+=	$(1)_clean
DISTCLEAN_TARGETS+=	$(1)_clean
$(1)_clean:
	$(RM) $(1) $$($(1)_OBJS)
endef

$(foreach prog,$(PROGS),$(eval $(call PROG_template,$(prog))))


# subdirectories

define SUBDIR_template
DEFAULT_TARGETS+=	$(1)
.PHONY: $(1)
$(1):
	@$(MAKE) -C $(1) all

INSTALL_TARGETS+=	$(1)_install
$(1)_install:
	@$(MAKE) -C $(1) install

CLEAN_TARGETS+=	$(1)_clean
$(1)_clean:
	@$(MAKE) -C $(1) clean

DISTCLEAN_TARGETS+=	$(1)_distclean
$(1)_distclean:
	@$(MAKE) -C $(1) distclean
endef

$(foreach subdir,$(SUBDIRS),$(eval $(call SUBDIR_template,$(subdir))))


# rules

.PHONY: all install clean distclean
all: $(DEFAULT_TARGETS)
install: $(INSTALL_TARGETS)
clean: $(CLEAN_TARGETS)
distclean: $(CLEAN_TARGETS) $(DISTCLEAN_TARGETS)

.PHONY: dep
dep:
ifneq ($(CXX_SRCS),)
	$(MKDEP) $(CXXFLAGS) $(MKDEPARGS) $(CXX_SRCS)
endif
ifneq ($(C_SRCS),)
	$(MKDEP) $(CFLAGS) $(MKDEPARGS) $(C_SRCS)
endif

.PHONY: tags
tags:
	$(CTAGS) -R $(CTAGSARGS) $(C_SRCS) $(CXX_SRCS)

-include .depend

endif # ndef targets.mk
