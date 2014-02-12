# Makefile
#
# Copyright 2005-2014 Fernando Silveira <fsilveira@gmail.com>
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

SRCDIR=		src
BINDIR=		bin
CLASSPATH=	../java/bin
BINMK=		$(BINDIR)/.mk

.PHONY: all
all: $(BINMK)
	@$(MAKE) -f $(BINMK) $@

.PHONY: clean
clean:
	$(RM) -r $(BINDIR)

# This target will create a temporary Makefile which will include all ".java"
# files found under $(SRCDIR) directory.
$(BINMK): $(shell find $(SRCDIR) ! -path '*/.svn/*' -and -name '*.java')
	$(MKDIR) -p $(BINDIR)
	@echo "find $(SRCDIR) > $(BINMK)"; \
		( \
		set -e; \
		src="$^"; \
		echo "# $(BINMK)"; \
		echo ".DEFAULT_GOAL=all"; \
		echo ".PHONY: all"; \
		echo "all: $(BINMK).all"; \
		echo "$(BINMK).all: $${src}"; \
		echo "	javac -classpath $(CLASSPATH) -d $(BINDIR) $${src}"; \
		echo "	touch \$$@"; \
		echo ".PHONY: clean"; \
		echo "clean:"; \
		echo "	find $(BINDIR) ! -path '*/.svn/*' -and -name '*.class' -delete"; \
	) > $(BINMK)

include ../mk/targets.mk
