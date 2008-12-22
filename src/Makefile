# Makefile

LIB	= libjnyikes.so
OBJS	= llist.o pobject.o interface.o

JAVADIR	= /usr/lib/jvm/java-1.5.0-sun-1.5.0.14

CFLAGS	= -O2 -pipe -Wall -Werror -Wshadow \
	  -I$(JAVADIR)/include \
	  -I$(JAVADIR)/include/linux

LDFLAGS	= -shared -lpthread

CC	= gcc

.PHONY: all
all: $(LIB)

.PHONY: clean
clean:
	$(RM) $(LIB)
	$(RM) $(OBJS)

$(LIB): $(OBJS)
	$(CC) -o $(LIB) $(LDFLAGS) $(OBJS)
