# Generated automatically from Makefile.in by configure.
CSRCS		=	flock.c pop_dele.c pop_dropcopy.c \
			pop_get_command.c pop_get_subcommand.c pop_init.c \
			pop_last.c pop_list.c pop_log.c pop_lower.c \
			pop_msg.c pop_parse.c pop_pass.c pop_quit.c \
			pop_rset.c pop_send.c pop_stat.c pop_updt.c \
			pop_user.c pop_xtnd.c pop_xmit.c popper.c \
			pop_bull.c xtnd_xlst.c pop_uidl.c mktemp.c \
			pop_rpop.c pop_apop.c md5.c pop_auth.c

OBJS		=	flock.o pop_dele.o pop_dropcopy.o \
			pop_get_command.o pop_get_subcommand.o pop_init.o \
			pop_last.o pop_list.o pop_log.o pop_lower.o \
			pop_msg.o pop_parse.o pop_pass.o pop_quit.o \
			pop_rset.o pop_send.o pop_stat.o pop_updt.o \
			pop_user.o pop_xtnd.o pop_xmit.o popper.o \
			pop_bull.o xtnd_xlst.o pop_uidl.o mktemp.o \
			pop_rpop.o pop_apop.o md5.o pop_auth.o

DOCS		=	README pop3.rfc1081 pop3e.rfc1082 popper.8

INCLUDES	=	popper.h version.h

SRCS		=	${CSRCS} ${INCLUDES}

CC = gcc

MAKEFILE	=	Makefile

#               Defines are described in the INSTALL document.

srcdir          =       .
CFLAGS		=	-g -O2 -fstrength-reduce -fpcc-struct-return 
DEFS		=	-DHAVE_CONFIG_H
O_DEFS		= 	 -DLINUX
prefix          =       /usr/local
exec_prefix     =       ${prefix}
TARGET		=	popper 
INSTALLDIR	=	${exec_prefix}/bin
MANPAGE		=	popper.8
CATPAGE		=	popper.0
MANDIR		=	${prefix}/man/cat8
NETWORK_LIBS	=	 -lresolv
KERBEROS_LIBS	=	
DBM_LIBS	=	-lgdbm
LIBS		= 	 -lresolv  -lgdbm  -lcrypt



all: ${TARGET}

popper: ${OBJS}
	${CC}  ${OBJS} -o popper ${LIBS}

popauth: popauth.o flock.o
	${CC}  -o popauth popauth.o  flock.o ${DBM_LIBS}

.c.o: 
	${CC} -c -I. -I${srcdir} ${CFLAGS} ${DEFS} ${O_DEFS} $< -o $@

tar: ${SRCS} ${DOCS} ${MAKEFILE}
	rm -f ${TAR} *.Z*
	tar -cvf ${TAR} ${SRCS} ${DOCS} ${MAKEFILE}
	compress ${TAR}
	uuencode ${TAR}.Z ${TAR}.Z > ${TAR}.Z.uuencoded
	split -300 ${TAR}.Z.uuencoded
	mv xaa ${TAR}.Z.uuencoded.xaa
	mv xab ${TAR}.Z.uuencoded.xab
	mv xac ${TAR}.Z.uuencoded.xac
	mv xad ${TAR}.Z.uuencoded.xad
	mv xae ${TAR}.Z.uuencoded.xae

clean:
	rm -f core *.o *.Z* *~

realclean: clean
	rm -f config.h config.status config.cache config.log ${TARGET} Makefile

${OBJS}:    ${INCLUDES}
pop_dropcopy.o: flock.h
pop_updt.o: flock.h
flock.o: flock.h










