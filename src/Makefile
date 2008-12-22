# Makefile

LIB=		jnyikes
SRCS=		llist.c pobject.c interface.c

JAVADIR=	/usr/lib/jvm/java-1.5.0-sun-1.5.0.14

INCDIRS=	$(JAVADIR)/include $(JAVADIR)/include/linux
DEPLIBS=	pthread

CFLAGS+=	-D_REENTRANT -Wmissing-prototypes \
		-I$(JAVADIR)/include \
		-I$(JAVADIR)/include/linux

include ../mk/targets.mk
