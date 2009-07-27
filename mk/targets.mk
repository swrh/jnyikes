# targets.mk
#
# Copyright 2005, 2009 Fernando Silveira <fsilveira@gmail.com>
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

# C compiler.
CFLAGS+=		-std=gnu99 -Wall -Werror -Wunused
#CFLAGS+=		-Wshadow
ifeq ($(DEBUG),y)
CFLAGS+=		-O0 -ggdb3 -DDEBUG
else
CFLAGS+=		-O3
endif

# C++ compiler.
CXXFLAGS=		$(CFLAGS)

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

# Directories.
PREFIX=			/usr/local

SH_PREFIX=		PS4=""; set -ex
#SH_PREFIX=		:

# Avoid unwanted default goals. The `all' goal must be redefined at the end of
# this file.
.PHONY: all
.DEFAULT_GOAL:=		all
all:

# Empty goals.
.PHONY: no not empty null
no not empty null:

# override some global definitions
%.c: %.y
	yacc -d -o $@ $(firstword $^)


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

VPATH+=			$(LIBDIRS)


# binaries

define BIN_template
define BIN_SRC_template
ifndef $$(1)_CFLAGS
$$(1)_CFLAGS=		$$(CFLAGS)
endif
$$(patsubst %.c,%.o,$$(1)):: $$(1) $$$$($$(1)_DEPS)
	$(CC) $$$$($$(1)_CFLAGS) $$(patsubst %,-I%,$$($(1)_INCDIRS)) $$(patsubst %,-I%,$(INCDIRS)) -c -o $$$$@ $$$$<

$(1)_OBJS+=		$$(patsubst %.c,%.o,$$(1))
endef

$$(foreach src,$$($(1)_SRCS),$$(eval $$(call BIN_SRC_template,$$(src))))

define BIN_INCDIR_template
MKDEPARGS+=		-I$$(1) # mkdep
CTAGSARGS+=		$$(1) # ctags
endef

$$(foreach incdir,$$($(1)_INCDIRS),$$(eval $$(call BIN_INCDIR_template,$$(incdir))))

VPATH+=			$$($(1)_LIBDIRS)

MKDEPARGS+=		$$($(1)_SRCS) # mkdep
CTAGSARGS+=		$$($(1)_SRCS) # ctags
endef

$(foreach bin,$(LIBS) $(PROGS),$(eval $(call BIN_template,$(bin))))


# libraries

define LIB_template
DEFAULT_TARGETS+=	lib$(1).a
STLIBS+=		lib$(1).a
lib$(1).a: $$($(1)_OBJS)

DEFAULT_TARGETS+=	lib$(1).so
SHLIBS+=		lib$(1).so
lib$(1).so: $$($(1)_OBJS) $$($(1)_DEPLIBS:%=-l%)

$(1): lib$(1).a lib$(1).so

CLEAN_TARGETS+=		$(1)_clean
DISTCLEAN_TARGETS+=	$(1)_clean
$(1)_clean:
	$(RM) lib$(1).a lib$(1).so $$($(1)_OBJS)
endef

$(foreach lib,$(LIBS),$(eval $(call LIB_template,$(lib))))


$(STLIBS):
	$(AR) $(ARFLAGS) $@ $^

$(SHLIBS):
	$(CC) $^ $(LDFLAGS) -o $@ -shared


# programs

define PROG_template
DEFAULT_TARGETS+=	$(1)
$(1): $$($(1)_OBJS) $$($(1)_DEPLIBS:%=-l%)

CLEAN_TARGETS+=	$(1)_clean
DISTCLEAN_TARGETS+=	$(1)_clean
$(1)_clean:
	$(RM) $(1) $$($(1)_OBJS)
endef

$(foreach prog,$(PROGS),$(eval $(call PROG_template,$(prog))))

$(PROGS):
	$(LINK.o) $^ $(LDLIBS) -o $@


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
distclean: $(DISTCLEAN_TARGETS)

.PHONY: dep
dep:
	$(MKDEP) $(CFLAGS) $(MKDEPARGS)

.PHONY: tags
tags:
	$(CTAGS) -R $(CTAGSARGS)

-include .depend

endif # ndef targets.mk
