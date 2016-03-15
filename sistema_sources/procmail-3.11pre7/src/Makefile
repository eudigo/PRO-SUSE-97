
LOCKINGTEST=__defaults__

#LOCKINGTEST=/tmp .	# Uncomment and add any directories you see fit.
#			If LOCKINGTEST is defined, autoconf will NOT
#			prompt you to enter additional directories.
#			See INSTALL for more information about the
#			significance of the locking tests.

########################################################################
# Only edit below this line if you *think* you know what you are doing #
########################################################################

#LOCKINGTEST=100	# Uncomment (and change) if you think you know
#			it better than the autoconf lockingtests.
#			This will cause the lockingtests to be hotwired.
#			100	to enable fcntl()
#			010	to enable lockf()
#			001	to enable flock()
#			Or them together to get the desired combination.

# Optional system libraries we search for
SEARCHLIBS = -lm -ldir -lx -lsocket -lnet -linet -lnsl_s -lnsl_i -lnsl -lsun \
 -lgen -lsockdns -ldl
#			-lresolv	# not really needed, is it?

# Informal list of directories where we look for the libraries in SEARCHLIBS
LIBPATHS=/lib /usr/lib /usr/local/lib

GCC_WARNINGS = -O2 -pedantic -Wreturn-type -Wunused -Wformat -Wtraditional \
 -Wpointer-arith -Wconversion -Waggregate-return \
 #-Wimplicit -Wshadow -Wid-clash-6 #-Wuninitialized

# The place to put your favourite extra cc flag
CFLAGS0 = -O #$(GCC_WARNINGS)
LDFLAGS0= -s
# Read my libs :-)
LIBS=

CFLAGS1 = $(CFLAGS0) #-posix -Xp
LDFLAGS1= $(LDFLAGS0) $(LIBS) #-lcposix

####CC	= cc # gcc
# object file extension
O	= o
RM	= /bin/rm -f
MV	= mv -f
LN	= ln
BSHELL	= /bin/sh
INSTALL = cp
DEVNULL = /dev/null

SUBDIRS = src man
BINSS	= procmail lockfile formail mailstat
MANS1S	= procmail formail lockfile
MANS5S	= procmailrc procmailsc procmailex

# Makefile.1 - mark, don't (re)move this, a sed script needs it

FGREP	= grep -F
CC	= gcc
CFLAGS	= $(CFLAGS1)
LDFLAGS	= $(LDFLAGS1) -lm -lnsl -ldl -lc

BINS= new/procmail new/lockfile new/formail new/mailstat
MANS= new/procmail.1 new/formail.1 new/lockfile.1 new/procmailrc.5 new/procmailsc.5 new/procmailex.5
MANS1= procmail.$(MAN1SUFFIX) formail.$(MAN1SUFFIX) lockfile.$(MAN1SUFFIX)
MANS5= procmailrc.$(MAN5SUFFIX) procmailsc.$(MAN5SUFFIX) procmailex.$(MAN5SUFFIX)
MANSS= procmail.1 formail.1 lockfile.1 procmailrc.5 procmailsc.5 procmailex.5
NBINS= ../new/procmail ../new/lockfile ../new/formail ../new/mailstat
NMANS= ../new/procmail.1 ../new/formail.1 ../new/lockfile.1 ../new/procmailrc.5 ../new/procmailsc.5 ../new/procmailex.5

#$Id: Makefile.0,v 1.49 1997/04/28 00:27:44 srb Exp $

PM_OBJ=cstdio.$(O) common.$(O) exopen.$(O) goodies.$(O) locking.$(O) \
 mailfold.$(O) misc.$(O) pipes.$(O) regexp.$(O) robust.$(O) sublib.$(O) \
 acommon.$(O) mcommon.$(O) lastdirsep.$(O) authenticate.$(O)
LF_OBJ=exopen.$(O) sublib.$(O) acommon.$(O) mcommon.$(O) authenticate.$(O)
FM_OBJ=common.$(O) fields.$(O) formisc.$(O) sublib.$(O) ecommon.$(O) \
 acommon.$(O)
MG_OBJ=sublib.$(O) ecommon.$(O) mcommon.$(O) hsort.$(O) lastdirsep.$(O)

all:	$(BINSS)

make:							# fake target
	@$(SHELL) -c "exit 0"

.PRECIOUS: Makefile

procmail: procmail.$(O) $(PM_OBJ) setid
	$(CC) $(CFLAGS) $@.$(O) $(PM_OBJ) -o $@ $(LDFLAGS)

lockfile: lockfile.$(O) $(LF_OBJ)
	$(CC) $(CFLAGS) $@.$(O) $(LF_OBJ) -o $@ $(LDFLAGS)

formail: formail.$(O) $(FM_OBJ)
	$(CC) $(CFLAGS) $@.$(O) $(FM_OBJ) -o $@ $(LDFLAGS)

mailstat: ../examples/mailstat
	cp ../examples/$@ $@
	@chmod 0755 $@

multigram: multigram.$(O) $(MG_OBJ) setid
	$(CC) $(CFLAGS) $@.$(O) $(MG_OBJ) -o $@ $(LDFLAGS)

../config.check:
	@cd ..; $(MAKE) config.check

_autotst: _autotst.$(O) sublib.c sublib.h
	$(CC) $(CFLAGS) $@.$(O) -o $@ $(LDFLAGS)

../autoconf.h: autoconf Makefile
	@echo No this was not make -n >make_n
	$(SHELL) ./autoconf $(SHELL) "$(RM)" "$(MV)" $(DEVNULL) \
 "$(FGREP)" "$(MAKE)" $(O) "$(LOCKINGTEST)" \
 "$(VISIBLE_BINDIR)" $@

autoconf.h: ../autoconf.h

targetdir.h:
	echo "You only can/need to make multigram if you are installing"
	echo "the mailinglist scripts.	Read ../SmartList/INSTALL for"
	echo "more directions."
	exit 64

acommon.$(O): ../autoconf.h ../config.h includes.h acommon.h robust.h shell.h

authenticate.$(O): ../autoconf.h ../config.h includes.h robust.h shell.h
authenticate.$(O): misc.h authenticate.h authenticate.c
	$(CC) -c -DPROCMAIL $(CFLAGS) $*.c

common.$(O): ../autoconf.h ../config.h includes.h procmail.h sublib.h robust.h
common.$(O): shell.h misc.h common.h

cstdio.$(O): ../autoconf.h ../config.h includes.h procmail.h robust.h cstdio.h
cstdio.$(O):  misc.h

ecommon.$(O): ../autoconf.h ../config.h includes.h ecommon.h common.h shell.h

exopen.$(O): ../autoconf.h ../config.h includes.h procmail.h acommon.h robust.h
exopen.$(O): misc.h exopen.h

fields.$(O): ../autoconf.h ../config.h includes.h formail.h sublib.h shell.h
fields.$(O): common.h fields.h ecommon.h formisc.h

formail.$(O): ../autoconf.h ../config.h includes.h formail.h acommon.h sublib.h
formail.$(O): shell.h common.h fields.h ecommon.h formisc.h header.h

formisc.$(O): ../autoconf.h ../config.h includes.h formail.h sublib.h shell.h
formisc.$(O): common.h ecommon.h formisc.h

goodies.$(O): ../autoconf.h ../config.h includes.h procmail.h sublib.h robust.h
goodies.$(O): shell.h misc.h pipes.h common.h cstdio.h goodies.h

hsort.$(O): ../autoconf.h ../config.h includes.h hsort.h

lastdirsep.$(O): ../autoconf.h ../config.h includes.h lastdirsep.h

lockfile.$(O): ../autoconf.h ../config.h includes.h sublib.h exopen.h mcommon.h
lockfile.$(O): authenticate.h

locking.$(O): ../autoconf.h ../config.h includes.h procmail.h robust.h shell.h
locking.$(O): misc.h pipes.h exopen.h locking.h lastdirsep.h

mailfold.$(O): ../autoconf.h ../config.h includes.h procmail.h acommon.h
mailfold.$(O): sublib.h robust.h shell.h misc.h pipes.h common.h exopen.h
mailfold.$(O): goodies.h locking.h mailfold.h

mcommon.$(O): ../autoconf.h ../config.h includes.h mcommon.h

misc.$(O): ../autoconf.h ../config.h includes.h procmail.h acommon.h sublib.h
misc.$(O): robust.h shell.h misc.h pipes.h common.h cstdio.h exopen.h regexp.h
misc.$(O): mcommon.h goodies.h locking.h network.h mailfold.h lastdirsep.h
misc.$(O): authenticate.h

multigram.$(O): ../autoconf.h ../config.h includes.h sublib.h hsort.h shell.h
multigram.$(O): ecommon.h mcommon.h lastdirsep.h targetdir.h

pipes.$(O): ../autoconf.h ../config.h includes.h procmail.h robust.h shell.h
pipes.$(O): misc.h pipes.h common.h cstdio.h exopen.h mcommon.h goodies.h
pipes.$(O): mailfold.h

procmail.$(O): ../patchlevel.h ../autoconf.h ../config.h includes.h procmail.h
procmail.$(O): acommon.h sublib.h robust.h shell.h misc.h pipes.h common.h
procmail.$(O): cstdio.h exopen.h regexp.h mcommon.h goodies.h locking.h
procmail.$(O): mailfold.h lastdirsep.h authenticate.h

regexp.$(O): ../autoconf.h ../config.h includes.h procmail.h sublib.h robust.h
regexp.$(O): shell.h misc.h regexp.h goodies.h

robust.$(O): ../autoconf.h ../config.h includes.h procmail.h robust.h shell.h
robust.$(O): misc.h pipes.h common.h mailfold.h shell.h

sublib.$(O): ../autoconf.h ../config.h includes.h sublib.h shell.h

gethome.$(O) setid.$(O) recommend.$(O): ../autoconf.h ../config.h includes.h

gethome.$(O): gethome.c
	@$(CC) -c $(CFLAGS) $*.c

setid.$(O): setid.c
	@$(CC) -c $(CFLAGS) $*.c

recommend.$(O): recommend.c
	@$(CC) -c $(CFLAGS) $*.c

.c.$(O):
	$(CC) -c $(CFLAGS) $<

gethome: gethome.$(O) setid
	@$(CC) $(CFLAGS) $@.$(O) -o $@ $(LDFLAGS)

getparams:
	@echo "ln=\"$(LN)\"" >../SmartList/targetdir.tmp

setid: setid.$(O)
	@$(CC) $(CFLAGS) $@.$(O) -o $@ $(LDFLAGS)

recommend: recommend.$(O) sublib.$(O)
	@$(CC) $(CFLAGS) $@.$(O) sublib.$(O) -o $@ $(LDFLAGS)

../man/man.sed: manconf.c ../autoconf.h ../config.h includes.h procmail.h
../man/man.sed: ../patchlevel.h lastdirsep.h lastdirsep.$(O)
	@$(CC) $(CFLAGS) "-DBINDIR=\"$(VISIBLE_BINDIR)\"" -o _autotst \
 manconf.c lastdirsep.$(O) $(LDFLAGS)
	@./_autotst $@
	@echo Housekeeping file >$@
	@$(RM) _autotst

clean:
	$(RM) -r _locktest
	$(RM) procmail.$(O) $(PM_OBJ) lockfile.$(O) $(LF_OBJ) formail.$O \
 $(FM_OBJ) multigram.$(O) $(MG_OBJ) $(BINSS) multigram ../autoconf.h \
 _autotst* lookfor _locktst* grepfor recommend recommend.$(O) manconf \
 _Makefile lock.log *core* targetdir.h setid setid.$(O) gethome \
 gethome.$(O) make_n

Makefile: ../Makefile Makefile.0
	@echo "You have made changes to the master Makefile, in order for"
	@echo "these changes to show through, you will first have to do:"
	@echo "$(MAKE) makefiles"

makefiles Makefiles makefile:
	cd ..; $(MAKE) makefiles

init:
	cd ..; $(MAKE) $@
VISIBLE_BINDIR=/usrp/bin

../new/procmail: procmail ../config.check
	@test -d ../new || mkdir ../new
	@$(RM) $@
	$(LN) ../src/procmail $@

../new/lockfile: lockfile ../config.check
	@test -d ../new || mkdir ../new
	@$(RM) $@
	$(LN) ../src/lockfile $@

../new/formail: formail ../config.check
	@test -d ../new || mkdir ../new
	@$(RM) $@
	$(LN) ../src/formail $@

../new/mailstat: mailstat ../config.check
	@test -d ../new || mkdir ../new
	@$(RM) $@
	$(LN) ../src/mailstat $@
#
