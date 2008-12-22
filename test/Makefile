# Makefile

PROG=		test
SRCS=		main.c

JAVADIR=	/usr/lib/jvm/java-1.5.0-sun-1.5.0.14

INCDIRS=	../src $(JAVADIR)/include $(JAVADIR)/include/linux
LIBDIRS=	../src
DEPLIBS=	jnyikes

CFLAGS+=	-D_REENTRANT -Wmissing-prototypes

test.c_DEPS=	$(patsubst %.c,%.h,$(test_SRCS))

include ../mk/targets.mk
