#
# $Id: Makefile,v 1.6 1997/06/05 08:20:45 cdr Exp $ %Y% Copyright 1991 Livingston Enterprises Inc
#
# If your system has the /usr/include/shadow.h file and uses getspnam() for
# shadow passwords then delete the -DNOSHADOW flag from CPPFLAGS in conf/$(EXT)
#
# To compile with support for SecurID
#  1) copy the SecurID header files $(SDHEADERS) into this directory
#  2) copy the sdiclient.a library from SECURID into $(OBJDIR)
#	(and run ranlib on it if needed)
#  3) make EXT=sun4_4.1 sradiusd 
#	(RS6000_4.1, HP9000_10.01, and sun4_5.5 are also available for sradiusd)
#  4) run sradiusd exactly as you would run radiusd 
#	(you can rename it to radiusd if you wish)
#
# To build on a platform type "make EXT=platform build" where platform
# is replaced by the one values in the conf subdirectory.
# For example, to build on SunOS 4.1.4 type "make EXT=sun4_4.1 build"
# 
CC              = gcc
COPT            = -O
#CPPFLAGS        = -DNOSHADOW
LIBS            = $(RADLIBS)

SRCDIR		= .
OBJDIR		= $(SRCDIR)/$(EXT)
SLIBS		= $(OBJDIR)/sdiclient.a

include $(SRCDIR)/conf/$(EXT)

CFLAGS          = $(COPT) $(CPPFLAGS)

HEADERS		= $(SRCDIR)/conf.h $(SRCDIR)/md5.h $(SRCDIR)/radius.h \
		  $(SRCDIR)/users.h

SDHEADERS	= $(SRCDIR)/sdacmvls.h $(SRCDIR)/sdconf.h $(SRCDIR)/sdi_athd.h \
		  $(SRCDIR)/sdi_size.h $(SRCDIR)/sdi_type.h
RAD_CSRC        = $(SRCDIR)/acct.c $(SRCDIR)/attrprint.c \
		  $(SRCDIR)/dict.c $(SRCDIR)/log.c $(SRCDIR)/md5.c \
		  $(SRCDIR)/menu.c $(SRCDIR)/users.c $(SRCDIR)/util.c \
		  $(SRCDIR)/version.c $(SRCDIR)/radiusd.c
RAD_OBJS        = $(OBJDIR)/acct.o $(OBJDIR)/attrprint.o $(OBJDIR)/dict.o \
		  $(OBJDIR)/log.o $(OBJDIR)/md5.o $(OBJDIR)/menu.o \
		  $(OBJDIR)/users.o $(OBJDIR)/util.o $(OBJDIR)/version.o \
		  $(OBJDIR)/radiusd.o
SRAD_OBJS        = $(OBJDIR)/acct.o $(OBJDIR)/attrprint.o $(OBJDIR)/dict.o \
		  $(OBJDIR)/log.o $(OBJDIR)/md5.o $(OBJDIR)/menu.o \
		  $(OBJDIR)/users.o $(OBJDIR)/util.o $(OBJDIR)/sversion.o \
		  $(OBJDIR)/sradiusd.o $(OBJDIR)/securid.o

TEST_OBJS	= $(OBJDIR)/users.o $(OBJDIR)/dict.o $(OBJDIR)/attrprint.o \
		  $(OBJDIR)/util.o $(OBJDIR)/log.o $(OBJDIR)/version.o

.PRECIOUS: $(HEADERS) ($SDHEADERS) $(RAD_CSRC) builddbm.c

CMDS		= $(OBJDIR)/radiusd $(OBJDIR)/builddbm

usage:
	@ cat $(SRCDIR)/README

build:	$(OBJDIR) $(CMDS)

clean:
	-rm -f $(CMDS)
	-rm -f $(OBJDIR)/*.o

clobber: clean
	sccs clean

strip:
	strip $(CMDS)

$(OBJDIR):
	- mkdir -p $(OBJDIR)

$(OBJDIR)/radiusd: $(RAD_OBJS)
	$(CC) $(CFLAGS) -o $@ $(RAD_OBJS) $(LIBS)

$(OBJDIR)/builddbm: $(OBJDIR)/builddbm.o $(OBJDIR)/version.o $(OBJDIR)/log.o $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(OBJDIR)/builddbm.o $(OBJDIR)/version.o $(OBJDIR)/log.o $(LIBS)

$(OBJDIR)/builddbm.o:	$(SRCDIR)/builddbm.c
	$(CC) -c -o $@ $(CFLAGS) $(SRCDIR)/builddbm.c
$(OBJDIR)/radiusd.o:	$(SRCDIR)/radiusd.c
	$(CC) -c -o $@ $(CFLAGS) $(SRCDIR)/radiusd.c
$(OBJDIR)/users.o:	$(SRCDIR)/users.c
	$(CC) -c -o $@ $(CFLAGS) $(SRCDIR)/users.c
$(OBJDIR)/dict.o:	$(SRCDIR)/dict.c
	$(CC) -c -o $@ $(CFLAGS) $(SRCDIR)/dict.c
$(OBJDIR)/util.o:	$(SRCDIR)/util.c
	$(CC) -c -o $@ $(CFLAGS) $(SRCDIR)/util.c
$(OBJDIR)/md5.o:	$(SRCDIR)/md5.c
	$(CC) -c -o $@ $(CFLAGS) $(SRCDIR)/md5.c
$(OBJDIR)/attrprint.o:	$(SRCDIR)/attrprint.c
	$(CC) -c -o $@ $(CFLAGS) $(SRCDIR)/attrprint.c
$(OBJDIR)/acct.o:	$(SRCDIR)/acct.c
	$(CC) -c -o $@ $(CFLAGS) $(SRCDIR)/acct.c
$(OBJDIR)/version.o:	$(SRCDIR)/version.c
	$(CC) -c -o $@ $(CFLAGS) $(SRCDIR)/version.c
$(OBJDIR)/menu.o:	$(SRCDIR)/menu.c
	$(CC) -c -o $@ $(CFLAGS) $(SRCDIR)/menu.c
$(OBJDIR)/log.o:	$(SRCDIR)/log.c
	$(CC) -c -o $@ $(CFLAGS) $(SRCDIR)/log.c
#
# SecurID support
#
sradiusd: $(OBJDIR)/sradiusd
$(OBJDIR)/sradiusd: $(SRAD_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SRAD_OBJS) $(SLIBS) $(LIBS)
$(OBJDIR)/securid.o:	$(SRCDIR)/securid.c $(SDHEADERS)
	$(CC) -c -o $@ $(CFLAGS) -DSECURID $(SRCDIR)/securid.c
$(OBJDIR)/sversion.o:	$(SRCDIR)/version.c
	$(CC) -c -o $@ -DSECURID $(CFLAGS) $(SRCDIR)/version.c
$(OBJDIR)/sradiusd.o:	$(SRCDIR)/radiusd.c $(SDHEADERS)
	$(CC) -c -o $@ $(CFLAGS) -DSECURID $(SRCDIR)/radiusd.c
