# defs.mk

ifndef defs.mk
defs.mk=		y

#ifndef TOPDIR
#$(error You must define TOPDIR before including any .mk file)
#endif

# C compiler.
CFLAGS=			-std=gnu99 -Wall -Werror -Wunused
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
#CHECKSUM=		$(TOPDIR)/bin/checksum
CP=			cp
CTAGS=			ctags
CXX=			g++
#DIGEST=			$(TOPDIR)/bin/digest
#EXTRACT=		$(TOPDIR)/bin/extract
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

# To avoid unwanted default goals.  The `all' goal must be redefined at the end
# of the parsing at `rules.mk'.
.PHONY: all
.DEFAULT_GOAL:=		all
all:

# Empty goals.
.PHONY: no not empty null
no not empty null:

endif # ndef defs.mk
