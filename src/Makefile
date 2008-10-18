# Makefile

LIB	= libjnyikes.so
OBJS	= llist.o pobject.o interface.o app.o

CFLAGS	= -O2 -pipe -Wall -Werror -Wshadow

RM	= rm -f

all: ${LIB}

clean:
	${RM} ${LIB}
	${RM} ${OBJS}

${LIB}: ${OBJS}
	${CC} -o ${LIB} ${LDFLAGS} ${OBJS}
