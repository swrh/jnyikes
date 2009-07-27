# Makefile
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

include ../config.mk

SRCDIR=		org/jnyikes

TMPMK=		.tmp.mk

.PHONY: all clean
all clean:
	@[ -f $(TMPMK) ] || $(MAKE) $(TMPMK)
	@$(MAKE) -f $(TMPMK) $@

# This target will create a temporary Makefile named as $(TMPMK) which will be
# updated with all ".java" and ".class" files before each "all" and "clean"
# targets. The $(TMPMK) is used as list of files to be compiled and/or cleaned.
#
# FIXME If for any reason the file creation fails, an error will not be noticed
# by make(1) unless find(1) fails too.
.PHONY: $(TMPMK)
$(TMPMK):
	@find $(SRCDIR) -name '*.java' | ( \
			echo "# $@"; \
			echo ".DEFAULT_GOAL=all"; \
			echo "%.class: %.java"; \
			echo $$'\tjavac $$<'; \
			all=""; \
			while read j; do \
				c="$${j%.java}.class"; \
				all="$${all} $${c}"; \
				echo "$${c}: $${j}"; \
			done; \
			echo "all:$${all}"; \
			echo "clean:"; \
			echo $$'\t$$(RM)'"$${all}"; \
			echo $$'\t$$(RM) $@'; \
		) > "$@"
