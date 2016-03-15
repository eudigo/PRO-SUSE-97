# Entering rdbms.mk

# Entering /olympia/src805/buildtools/mkfile/prefix.mk

# Entering /olympia/src805/buildtools/mkfile/defs.mk

SHELL=/bin/sh

AS_EXT=s
LIB_EXT=a
OBJ_EXT=o
PLB_EXT=plb
SO_EXT=so
LOCK_EXT=lk
SQL_EXT=sql


AR=ar
AS=as
AWK=awk
CAT=cat
CC=cc
CD=cd
CHMOD=chmod
CP=cp
CPP=cpp
DATE=date
ECHO=echo
ECHON=echo -n
EXEC=exec
FIND=find
FOLLOW=-follow
NOLEAF=-noleaf
GREP=grep
KILL=kill
SLEEP=sleep
LD=ld
LMAKE=make
LN=ln
LNS=ln -s
MKDIR=mkdir
MV=mv
NM=nm
PERL=perl
RM=rm
RMF=rm -f
RMRF=rm -rf
SED=sed
SORT=sort
TOUCH=touch
XARGS=xargs
LS=ls

BINHOME=$(ORACLE_HOME)/bin/
LIBHOME=$(ORACLE_HOME)/lib/

PRODLIBHOME=$(LIBHOME)

ECHODO=$(BINHOME)echodo
GENSYSLIB=$(BINHOME)gensyslib
GENCLNTSH=$(BINHOME)genclntsh
GENNAFLG=$(BINHOME)gennaflg
GENAUTAB=$(BINHOME)genautab

ARCHIVE_OBJ=$(AR) r $@ $*.$(OBJ_EXT)
ARCREATE=$(AR) cr$(ARLOCAL)
ARDELETE=$(AR) d$(ARLOCAL)
AREXTRACT=$(AR) x 
ARPRINT=$(AR) t
ARREPLACE=$(AR) r 
DOAR=$(ARCREATE) $@ $? ; $(RANLIB) $@
MAKE=$(LMAKE)
QUIET=>/dev/null 2>&1
QUIETE=2>/dev/null
SILENT=@
CTSMAKEFLAGS=$(SILENT:@=--no-print-directory -s)

CFLAGS=$(GFLAG) $(OPTIMIZE) $(CCFLAGS) $(QACCFLAGS) $(PFLAGS)\
	$(SHARED_CFLAG) $(USRFLAGS)
PFLAGS=$(INCLUDE) $(SPFLAGS) $(LPFLAGS)

LDCCOM=$(PURECMDS) $(CC) $(GFLAG) $(CCFLAGS) $(EXOSFLAGS) $(LDFLAGS)
LDFLAGS=-o $@ -L$(PRODLIBHOME) -L$(LIBHOME) -L/usr/lib
LDLIBS=$(EXPDLIBS) $(EXOSLIBS) $(SYSLIBS) $(EXSYSLIBS) $(MATHLIB) $(USRLIBS)
MATHLIB=-lm
COMPATLIB=-lcompat
PURECMDS=$(PURELINK) $(PURIFY) $(PURECOV) $(QUANTIFY)

RANLIB=/usr/bin/ranlib
RANLIBCMD=/usr/bin/ranlib

SYSLIBLIST=$(LIBHOME)sysliblist
SYSLIBS=`$(CAT) $(SYSLIBLIST)`

NACCFLAGS=$(LIBHOME)naccflgs
NALDFLAGS=$(LIBHOME)naldflgs
NACCFLAGSLIST=`$(CAT) $(NACCFLAGS)`
NALDFLAGSLIST=`$(CAT) $(NALDFLAGS)`

TTLIBD=$(NETLIBD) $(LIBRDBMS_CLT) $(LIBMM) $(CORELIBD) $(LIBEPC) $(LIBCLNTSH)
STATICTTLIBS=$(NAUTAB) $(NAETAB) $(NAEDHS) $(NALDFLAGSLIST) \
       $(NETLIBS) $(LLIBRDBMS_CLT) $(LLIBMM) $(CORELIBS)\
       $(NETLIBS) $(LLIBRDBMS_CLT) $(LIBPLS_CLT)\
       $(LLIBEPC) $(CORELIBS) $(LLIBRDBMS_CLT) $(CORELIBS) $(LDLIBS)
TTLIBS=$(LIBCLNTSH) $(STATICTTLIBS)
DEVTTLIBS=$(TTLIBS)

STLIBS=$(SGL_ON) $(CONFIG) $(LLIBSERVER) $(LLIBPLSF) $(LLIBSLAX) \
       $(LIBGENERIC) $(LLIBCLIENT) $(LLIBMM) $(LLIBKNLOPT) $(LLIBPLSB)\
       $(LLIBSERVER) $(LLIBEXTP) $(NAUTAB) $(NAETAB) $(NAEDHS) \
       $(NALDFLAGSLIST) $(NETLIBS) $(LLIBCLIENT) \
       $(LLIBPLSF) $(LLIBICX) $(LLIBOWSUTL) \
       $(NETLIBS) $(LLIBEPC) $(SDOLIBS) $(LLIBCORE) $(LLIBVSN) $(LLIBCOMMON)\
       $(LIBGENERIC) $(LLIBKNLOPT) $(NMLIBS) $(CARTLIBS) $(CORELIBS) $(LDLIBS)

# Exiting /olympia/src805/buildtools/mkfile/defs.mk
# Entering /olympia/src805/buildtools/mkfile/platform.mk

PLATFORM=unified_intel

MOTIFHOME=/usr/X11R6/
GUILIBHOME=$(OPENWINHOME)lib/



GMAKE=/usr/bin/gmake
LMAKE=/usr/bin/gmake
FIND=/usr/bin/find # gnu find!
PS=ps -aux

ARCHIVE=@`if echo ${LIB} | grep lib > /dev/null 2>&1;then $(ARCREATE) $(LIB) $?; $(RANLIB) $(LIB); else echo ""; fi`

GROUP=/etc/group

LINK=gcc

LLIBTHREAD=
THREADLIBS=
MOTIFLIBS=-L$(MOTIFHOME)lib -lXm -lgen -L$(GUILIBHOME) \
	-lXt -lX11 -lw -lm
XLIBS=-L$(GUILIBHOME) $(GUILIBHHOME)libXaw.so.4 -lXt -lXmu\
	-lXext -lX11 -lm


CCVER=GCC2.7.2.3
COMPOBJ=$(ORACLE_HOME)/lib/$(CCVER)
COMPOBJS=$(COMPOBJ)/crti.o $(COMPOBJ)/crt1.o $(COMPOBJ)/crtn.o

EXSYSLIBS= -ldl 

DEVTTLIBS=$(NAUTAB) $(NAETAB) $(NAEDHS) $(NALDFLAGSLIST) \
	$(NETLIBS) $(LLIBRDBMS_CLT) $(LLIBMM) $(CORELIBS)\
	$(NETLIBS) $(LLIBRDBMS_CLT) $(LIBPLS_CLT)\
	$(LLIBEPC) $(CORELIBS) $(LLIBRDBMS_CLT) $(CORELIBS)

OPTIMIZE2=-xO2
OPTIMIZE4=-xO4
OPTIMIZE=$(OPTIMIZE2)

ROFLAGS=-c -xMerge
SPFLAGS=-D_SVID_GETTOD

AR=/usr/ccs/bin/ar
AS=/usr/ccs/bin/as
CPP=$(CC) -E $(ANSI)
CHMOD=/bin/chmod
CHGRP=/bin/chgrp
CHOWN=/bin/chown

ASFLAGS=
ASPFLAGS=-P $(PFLAGS)
ASRO=$(AS) $(ASROFLAGS) $(MAKERO) $<
ASROFLAGS=$(ASFLAGS)
CXC=-Xc
CXA=-Xa
ANSI=$(CXC)



NOKPIC_CCFLAGS= -Xa $(PROFILE) -xstrconst -xF -I$(ORACLE_HOME)/shdrs $(XS) -mr \
        -xarch=v8 -xcache=16/32/1:1024/64/1 -xchip=ultra -D_REENTRANT
CCFLAGS= -DLINUX -DUNIFIED_INTEL  $(NOKPIC_CCFLAGS) -K PIC
FASTCCFLAGS= $(NOKPIC_CCFLAGS)

SHARED_LDFLAG=-shared -L$(ORACLE_HOME)/lib -R$(ORACLE_HOME)/lib -o
SHARED_CFLAG=

# Entering /olympia/src805/buildtools/mkfile/s_platform.mk

PLATFORM=

CUS_PLFRM=$(ORACLE_HOME)/buildtools/mkfile/cus_plfrm.mk

MOTIFHOME=
OPENWINHOME=
GUILIBHOME=


GMAKE=gmake
MAKE=gmake
LMAKE=make
FIND=/bin/find # gnu find!
PS=ps -feda

ARCHIVE=@`if echo ${LIB} | grep lib > /dev/null 2>&1;then $(ARCREATE) $(LIB) $? $(RANLIB); else echo ""; fi`

GROUP=/etc/group

LINK=gcc $(COMPOBJS)

LLIBTHREAD=

MOTIFLIBS= -lXm -lXt -lX11 -lgen -lm
XLIBS=

EXOSFLAGS=

CCVER=SC4.2
COMPOBJ=$(ORACLE_HOME)/lib/$(CCVER)
COMPOBJS=

EXSYSLIBS=-ldl
DEVTTLIBS=$(NAUTAB) $(NAETAB) $(NAEDHS) $(NALDFLAGSLIST) \
       $(NETLIBS) $(LLIBRDBMS_CLT) $(LLIBMM) $(CORELIBS)\
       $(NETLIBS) $(LLIBRDBMS_CLT) $(LIBPLS_CLT)\
       $(LLIBEPC) $(CORELIBS) $(LLIBRDBMS_CLT) $(CORELIBS) \
       $(EXPDLIBS) $(EXOSLIBS) $(SYSLIBS) -lc -laio $(MATHLIB) $(USRLIBS)

OPTIMIZE3=-O3
OPTIMIZE6=-O6
OPTIMIZE=$(OPTIMIZE3)
NOOPTIMIZE=-O0

ROFLAGS=-c -xMerge
SPFLAGS=`$(ORACLE_HOME)/unified_bin/spflags`

AR=ar
AS=as
CPP=/lib/cpp
CHMOD=/bin/chmod
CHGRP=/bin/chgrp
CHOWN=/bin/chown

ASFLAGS=
ASPFLAGS=-P $(PFLAGS)
ASRO=$(AS) $(ASROFLAGS) $(MAKERO) $<
ASROFLAGS=$(ASFLAGS)
CXC=-Xc
CXA=-Xa
ANSI=$(CXC)



NOKPIC_CCFLAGS=`$(ORACLE_HOME)/unified_bin/ccflags`

CCFLAGS= $(NOKPIC_CCFLAGS)
FASTCCFLAGS= $(NOKPIC_CCFLAGS)

SHARED_LDFLAG=-shared -L$(ORACLE_HOME)/lib -o
SHARED_CFLAG=

# Exiting /olympia/src805/buildtools/mkfile/s_platform.mk
# Exiting /olympia/src805/buildtools/mkfile/platform.mk
# Entering /olympia/src805/buildtools/mkfile/rules.mk
 
# Exiting /olympia/src805/buildtools/mkfile/rules.mk
# Exiting /olympia/src805/buildtools/mkfile/prefix.mk
# Entering /olympia/src805/oracore/exports.mk

COREHOME= $(ORACLE_HOME)/oracore/
ORACOREHOME= $(COREHOME)

LIBCORE= $(LIBHOME)libcore4.$(LIB_EXT)
LLIBCORE= -lcore4

S0MAIN= $(LIBHOME)s0main.o
SCOREPT= $(LIBHOME)scorept.o
SSCOREED= $(LIBHOME)sscoreed.o

COREPUBLIC=$(COREHOME)include/ $(COREHOME)public/
ORACOREPUBH=$(I_SYM)$(COREHOME)include $(I_SYM)$(COREHOME)public

CORELIBD=$(LIBNLSRTL) $(LIBCV6) $(LIBCORE) $(LIBNLSRTL) $(LIBCORE) $(LIBNLSRTL)
CORELIBS=$(LLIBNLSRTL) $(LLIBCV6) $(LLIBCORE) $(LLIBNLSRTL) $(LLIBCORE) $(LLIBNLSRTL)

# Entering /olympia/src805/oracore/s_exports.mk
 
# Exiting /olympia/src805/oracore/s_exports.mk

# Exiting /olympia/src805/oracore/exports.mk
# Entering /olympia/src805/nlsrtl/exports.mk

NLSRTLHOME= $(ORACLE_HOME)/nlsrtl/

ORA_NLS = $(ORACLE_HOME)/ocommon/nls/admin/data/
ORA_NLS33 = $(ORACLE_HOME)/ocommon/nls/admin/data/

LIBNLSRTL= $(LIBHOME)libnlsrtl3.$(LIB_EXT)
LLIBNLSRTL= -lnlsrtl3

NLSRTLPUBH = $(I_SYM)$(NLSRTLHOME)include
# Exiting /olympia/src805/nlsrtl/exports.mk
# Entering /olympia/src805/network/exports.mk

LIBNETV2     = $(LIBHOME)libnetv2.$(LIB_EXT)
LLIBNETV2    = -lnetv2

LIBNTTCP     = $(LIBHOME)libnttcp.$(LIB_EXT)
LLIBNTTCP    = -lnttcp

LIBNTOD      = $(LIBHOME)libntod.$(LIB_EXT)
LLIBNTOD     = -lntod

LIBNTLU62    = $(LIBHOME)libntlu62.$(LIB_EXT)
LLIBNTLU62   = -lntlu62

LIBNTS       = $(LIBHOME)libnts.$(LIB_EXT)
LLIBNTS      = -lnts

LIBNETWORK   = $(LIBHOME)libnetwork.$(LIB_EXT)  	
LLIBNETWORK  = -lnetwork

LIBSQLNET    = $(LIBNETV2)   $(LIBNTTCP)  $(LIBNETWORK)
LLIBSQLNET   = $(LLIBNETV2) $(LLIBNTTCP) $(LLIBNETWORK)

LIBRPC       = $(LIBHOME)libncr.$(LIB_EXT)
LLIBRPC      = -lncr 

LIBTNSAPI    = $(NETWORKLIB)libtnsapi.$(LIB_EXT)
LLIBTNSAPI   = -ltnsapi

TNSLSNR	     = $(BINHOME)tnslsnr
LSNRCTL	     = $(BINHOME)lsnrctl
NAMES	     = $(BINHOME)names
NAMESCTL     = $(BINHOME)namesctl

OKINIT       = $(BINHOME)okinit
OKLIST       = $(BINHOME)oklist
OKDSTRY      = $(BINHOME)okdstry

NIGTAB       = $(LIBHOME)nigtab.$(OBJ_EXT)
NIGCON       = $(LIBHOME)nigcon.$(OBJ_EXT) 
NTCONTAB     = $(LIBHOME)ntcontab.$(OBJ_EXT)
MTS_NCR	     = $(LIBHOME)ncrstab.$(OBJ_EXT)

OSNTABST     = $(NETWORKLIB)osntabst.$(OBJ_EXT)
NNFGT        = $(NETWORKLIB)nnfgt.$(OBJ_EXT)

NAETAB       = $(LIBHOME)naeet.$(OBJ_EXT) $(LIBHOME)naect.$(OBJ_EXT)
NAEDHS	     = $(LIBHOME)naedhs.$(OBJ_EXT)
NAUTAB	     = $(LIBHOME)nautab.$(OBJ_EXT)

NETLIBS      = $(LLIBSQLNET) $(LLIBRPC) $(LLIBSQLNET)
NETLIBD      = $(LIBSQLNET) $(LIBRPC)

# Entering /olympia/src805/network/s_exports.mk

NATIVE =


# Exiting /olympia/src805/network/s_exports.mk
# Exiting /olympia/src805/network/exports.mk
# Entering /olympia/src805/plsql/exports.mk

LIBEXTP=$(LIBHOME)libextp.$(LIB_EXT)
LLIBEXTP=-lextp

WRAP=$(BINHOME)wrap

LIBPLSB=$(LIBHOME)libplsb.$(LIB_EXT)
LLIBPLSB=-lplsb
LIBPLSF=$(LIBHOME)libplsf.$(LIB_EXT)
LLIBPLSF=-lplsf
PLSQLLIBS=$(LIBPLSF) $(LIBPLSB)
LPLSQLLIBS=$(LLIBPLSF) $(LLIBPLSB)

LLIBPLSQL= $(LPLSQLLIBS) $(LLIBEXTP)

LIBICD=$(LIBHOME)libicd.$(LIB_EXT)
LLIBICD=-licd
LIBPSD=$(LIBHOME)libpsd.$(LIB_EXT)
LLIBPSD=-lpsd
LIBPSA=$(LIBHOME)libpsa.$(LIB_EXT)
LLIBPSA=-lpsa

# Entering /olympia/src805/plsql/s_exports.mk
 
# Exiting /olympia/src805/plsql/s_exports.mk
# Exiting /olympia/src805/plsql/exports.mk
# Entering /olympia/src805/otrace/exports.mk

EPCLIBS=$(LLIBEPC)

LIBEPC=$(LIBHOME)libepc.$(LIB_EXT)
LLIBEPC=-lepc

LIBEPCPT=$(LIBHOME)libepcpt.$(LIB_EXT)
LLIBEPCPT=-lepcpt

LIBEPCFE=$(LIBHOME)libepcfe.$(LIB_EXT)
LLIBEPCFE=-lepcfe

# Entering /olympia/src805/otrace/s_exports.mk

# Exiting /olympia/src805/otrace/s_exports.mk
# Exiting /olympia/src805/otrace/exports.mk
# Entering /olympia/src805/icx/exports.mk

LIBICX= $(LIBHOME)/libdbicx.$(LIB_EXT)
LLIBICX= -ldbicx

ICXPUBH= $(I_SYM). $(I_SYM)$(ICXHOME)/public 

# Exiting /olympia/src805/icx/exports.mk
# Entering /olympia/src805/owsutl/exports.mk
 
LIBOWSUTL=$(LIBHOME)libndw1.$(LIB_EXT)
LLIBOWSUTL=-lndw1

# Entering /olympia/src805/owsutl/s_exports.mk
 
# Exiting /olympia/src805/owsutl/s_exports.mk
# Exiting /olympia/src805/owsutl/exports.mk
# Entering /olympia/src805/ordts/exports.mk

LLIBKIDT=-lkidt

# Entering /olympia/src805/ordts/s_exports.mk


# Exiting /olympia/src805/ordts/s_exports.mk
# Exiting /olympia/src805/ordts/exports.mk

# Entering /olympia/src805/rdbms/exports.mk

LIBSERVER=$(LIBHOME)libserver.$(LIB_EXT)
LLIBSERVER=-lserver

LIBCLIENT=$(LIBHOME)libclient.$(LIB_EXT)
LLIBCLIENT=-lclient

LIBGENERIC=$(LIBHOME)libgeneric.$(LIB_EXT)
LLIBGENERIC=-lgeneric

LIBCOMMON=$(LIBHOME)libcommon.$(LIB_EXT)
LLIBCOMMON=-lcommon

LIBVSN=$(LIBHOME)libvsn.$(LIB_EXT)
LLIBVSN=-lvsn

LIBAGENT=$(LIBHOME)libagent.$(LIB_EXT)
LLIBAGENT=-lagent

DLMHOME=$(ORACLE_HOME)/odlm/

SKGXNS=$(RDBMSLIB)skgxns.$(OBJ_EXT)
SKGXND=$(RDBMSLIB)skgxnd.$(OBJ_EXT)
SKGXN=$(RDBMSLIB)skgxn.$(OBJ_EXT)

LIBDLM=$(SKGXN) $(LIBHOME)libudlm.$(LIB_EXT)
LLIBDLM=$(SKGXN) -L /var/opt/ncrdlm/lib -ludlm

LIBNM=$(LIBDLM)
LLIBNM=$(LLIBDLM)
NMLIBLIST=$(RDBMSLIB)nmliblist
NMLIBS=`$(CAT) $(NMLIBLIST)`

LIBMM=$(LIBHOME)libmm.$(LIB_EXT)
LLIBMM=-lmm


LIBRDBMS_CLT=$(LIBCLIENT) $(LIBVSN) $(LIBCOMMON) $(LIBGENERIC)
LLIBRDBMS_CLT=$(LLIBCLIENT) $(LLIBVSN) $(LLIBCOMMON) $(LLIBGENERIC)

LIBCLNTSH=$(LIBHOME)libclntsh.$(SO_EXT)
LLIBCLNTSH=-lclntsh

CONFIG = $(RDBMSLIB)config.$(OBJ_EXT)

SDOLIBS = `if ${AR} tv ${ORACLE_HOME}/rdbms/lib/libknlopt.a | grep "kxmnsd.o" > /dev/null 2>&1 ; then echo " " ; else echo "-lmdknl -lmdhh"; fi`

DEF_ON= $(RDBMSLIB)kpudfo.$(OBJ_EXT)
DEF_OFF= $(RDBMSLIB)kpundf.$(OBJ_EXT)
DEF_OPT= $(RDBMSLIB)defopt.$(OBJ_EXT)

LIBSLAX=$(LIBHOME)libslax.a
LLIBSLAX=-lslax

LIBSQL=$(LIBHOME)libsql.$(LIB_EXT)
LLIBSQL=-lsql

LIBSVRM= $(LIBHOME)smalmain.$(OBJ_EXT) $(LIBHOME)libsvrmgrl.$(LIB_EXT) $(LIBHOME)libslpm.$(LIB_EXT)
LLIBSVRM= $(LIBHOME)smalmain.$(OBJ_EXT) -lsvrmgrl -lslpm

RDBMSHOME=$(ORACLE_HOME)/rdbms/
RDBMSLIB=$(RDBMSHOME)lib/
RDBMSADMIN=$(RDBMSHOME)admin/

# Entering /olympia/src805/rdbms/s_exports.mk

# Exiting /olympia/src805/rdbms/s_exports.mk
# Exiting /olympia/src805/rdbms/exports.mk
AR=ar

LIBKNLOPT=$(RDBMSLIB)libknlopt.$(LIB_EXT)
LLIBKNLOPT=-lknlopt

CARTLIBS= $(LLIBKIDT)
CARTLIBD= $(LLIBKIDT)
HS_OCI_DRIVER=$(RDBMSLIB)hoaoci.$(OBJ_EXT)		# ORACLE/OCI driver
HS_PCC_DRIVER=$(RDBMSLIB)hoapcc.$(OBJ_EXT)		# ORACLE/PCC driver
HS_DEP_DRIVER=$(RDBMSLIB)hoax.$(OBJ_EXT)                # DEP/XA driver
HS_OTS_DRIVER=$(RDBMSLIB)hoat.$(OBJ_EXT)                # HS OTS driver

HS_XASWITCH_ORA=$(RDBMSLIB)hsxaora.$(OBJ_EXT)           # XA switch for Oracle
 
HS_OCI_DRIVER_ON=$(RDBMSLIB)hokoci.$(OBJ_EXT)	# ORACLE/OCI driver for kernel
HS_OCI_DRIVER_OFF=$(RDBMSLIB)hoksoci.$(OBJ_EXT)	# ORACLE/OCI driver

HO_DRV_ON=$(HS_OCI_DRIVER_OFF)
HO_DRV_OFF=$(HS_OCI_DRIVER_OFF)
 

VER_ENT= vsnfprd.$(OBJ_EXT)
VER_STD= vsnfstd.$(OBJ_EXT)
VER_PO= vsnfpo.$(OBJ_EXT)
VSNNTP= vsnntp.$(OBJ_EXT)
VSNSTD= vsnstd.$(OBJ_EXT)
VSNPO= vsnpo.$(OBJ_EXT)

PLS_ON= kkxwtp.$(OBJ_EXT)
PLS_OFF= kkxntp.$(OBJ_EXT)

OPS_ON= kcsm.$(OBJ_EXT)
OPS_OFF= ksnkcs.$(OBJ_EXT)

DDB_ON= ktd.$(OBJ_EXT)
DDB_OFF= ksnktd.$(OBJ_EXT)

REP_ON= kkzo.$(OBJ_EXT)
REP_OFF= ksnkkz.$(OBJ_EXT)

SCF_ON = kksdopt.$(OBJ_EXT)
SCF_OFF= kksdnop.$(OBJ_EXT)

SDO_ON= kxmwsd.$(OBJ_EXT)
SDO_OFF= kxmnsd.$(OBJ_EXT)

HO_ON= $(RDBMSLIB)npiapho.$(OBJ_EXT) $(RDBMSLIB)homs.$(OBJ_EXT) $(HO_DRV_OFF)	
HO_ON_NPIAPH=npiapho.$(OBJ_EXT)
HO_ON_HOMS=homs.$(OBJ_EXT)
HO_OFF=$(RDBMSLIB)npiap.$(OBJ_EXT) $(RDBMSLIB)homsstub.$(OBJ_EXT) $(HO_DRV_OFF)
HO_OFF_NPIAP=npiap.$(OBJ_EXT)
HO_OFF_HOMSSTUB=homsstub.$(OBJ_EXT)

OBJ_ON= kokiobj.$(OBJ_EXT)
OBJ_OFF= koknobj.$(OBJ_EXT)

PART_ON= kkpoban.$(OBJ_EXT)
PART_OFF= ksnkkpo.$(OBJ_EXT)

LIL_ON= liblil.$(LIB_EXT)
LIL_OFF= sllfls.$(OBJ_EXT)

XAS_ON=$(LIBSQL)
XAS_OFF=xaonsl.$(OBJ_EXT)
 
XAD_OFF=xaondy.$(OBJ_EXT)

SQLLIBS=              			 # By default no SQLLIB, no dummy stubs

KPT_ON=kprwts.$(OBJ_EXT)
KPT_OFF=kprnts.$(OBJ_EXT)

SGL_ON=$(OSNTABST) $(RDBMSLIB)ttcsoi.$(OBJ_EXT) $(RDBMSLIB)ttcoerr.$(OBJ_EXT)
SGL_OFF=$(NIGTAB)

ORAMAI = $(RDBMSLIB)opimai.$(OBJ_EXT)			# oracle server
SSORED = $(RDBMSLIB)ssoraed.$(OBJ_EXT)			# oracle prog. mgr. data
IMPMAI = $(RDBMSLIB)impdrv.$(OBJ_EXT)			# import
EXPMAI = $(RDBMSLIB)exudrv.$(OBJ_EXT)			# export
LDRMAI = $(RDBMSLIB)ulmai.$(OBJ_EXT)			# loader
MIGMAI = $(RDBMSLIB)kumain.$(OBJ_EXT)			# migrate
MXMMAI = $(RDBMSLIB)maxmem.$(OBJ_EXT)			# maxmem
DBGMAI = $(RDBMSLIB)dbfmig.$(OBJ_EXT)           # dbfmig
TSMMAI = $(RDBMSLIB)tstshm.$(OBJ_EXT)			# tstshm
TKPMAI = $(RDBMSLIB)s0kvpf.$(OBJ_EXT)			# tkprof
GOCMAI = $(RDBMSLIB)genoci.$(OBJ_EXT)			# genoci
ADLMAI = $(RDBMSLIB)addlcmp.$(OBJ_EXT)			# addlcmp
PWDMAI = $(RDBMSLIB)s0kuzr.$(OBJ_EXT)			# orapwd
DBFMAI = $(RDBMSLIB)dbfsize.$(OBJ_EXT)			# dbfsize
CURMAI = $(RDBMSLIB)cursize.$(OBJ_EXT)			# cursize
DBVMAI = $(RDBMSLIB)kudbv.$(OBJ_EXT)			# dbverify
SSDBED=$(RDBMSLIB)ssdbaed.$(OBJ_EXT)			# sqldba prog. mgr. data
TDSMAI = $(RDBMSLIB)kopc.$(OBJ_EXT)			# tdscomp
EXTPMAI = $(RDBMSLIB)hormc.$(OBJ_EXT)			# extproc main
HSALLMAI = $(RDBMSLIB)horm.$(OBJ_EXT)			# hs all modules main
HSSQLMAI = $(RDBMSLIB)horms.$(OBJ_EXT)			# hs SQL main
HSDEPMAI = $(RDBMSLIB)hormd.$(OBJ_EXT)			# hs DEP main
HSOTSMAI = $(RDBMSLIB)hormt.$(OBJ_EXT)			# hs OTS main
GMSMAI = $(RDBMSLIB)/kgxgd.$(OBJ_EXT)	$(RDBMSLIB)/kgxgm.$(OBJ_EXT)							# OGMS main
GMCTLM = $(RDBMSLIB)kgxgk.$(OBJ_EXT)			# ogmsctl main
LKDMAI = $(RDBMSLIB)kjdx.$(OBJ_EXT)                             # lkdbx main
LKDFMT = $(RDBMSLIB)kjdxf.$(OBJ_EXT)                    # lkdbx format
SSORED = $(RDBMSLIB)ssoraed.$(OBJ_EXT)			# oracle prog. mgr. data
TTCSOI = $(RDBMSLIB)ttcsoi.$(OBJ_EXT)			# for osncoi, osnsoi.
ARCMAI = $(RDBMSLIB)archmon.$(OBJ_EXT)			# archmon
OSHMAI = $(RDBMSLIB)osh.$(OBJ_EXT)				# osh
GKSMAI = $(RDBMSLIB)genksms.$(OBJ_EXT)			# genksms
GKFMAI = $(RDBMSLIB)genkflat.$(OBJ_EXT)			# genkflat
SSKRMED = $(RDBMSLIB)sskrmed.$(OBJ_EXT)			# rman executable descriptor
SKRMPT = $(RDBMSLIB)skrmpt.$(OBJ_EXT)			# rman products table
SBBDPT = $(RDBMSLIB)sbbdpt.$(OBJ_EXT)           # bbed products table
SSBBDED = $(RDBMSLIB)ssbbded.$(OBJ_EXT)         # bbed executable descriptor
SKRSPT = $(RDBMSLIB)skrspt.$(OBJ_EXT)           # sbttest products table
SSKRSED = $(RDBMSLIB)sskrsed.$(OBJ_EXT)         # sbttest executable descriptor

ORACLE=$(RDBMSBIN)oracle
IMP=$(RDBMSBIN)imp
EXP=$(RDBMSBIN)exp
SQLLDR=$(RDBMSBIN)sqlldr
DBFMIG=$(RDBMSBIN)dbfmig
SQLLDR_LIL=$(RDBMSBIN)sqlldr_lil
MIG=$(RDBMSBIN)mig
DBVERIFY=$(RDBMSBIN)dbv
TSTSHM=$(RDBMSBIN)tstshm
MAXMEM=$(RDBMSBIN)maxmem
ORAPWD=$(RDBMSBIN)orapwd
TKPROF=$(RDBMSBIN)tkprof
DBFSIZE=$(RDBMSBIN)dbfsize
CURSIZE=$(RDBMSBIN)cursize
RMAN=$(RDBMSBIN)rman
GENOCI=$(RDBMSBIN)genoci
ADDLCMP=$(RDBMSBIN)addlcmp
GENKFLAT=$(RDBMSBIN)genkflat
SVRMGRL=$(RDBMSBIN)svrmgrl
TDSCOMP=$(RDBMSBIN)tdscomp
PLSFF=$(RDBMSBIN)plsff
EXTPROC=$(RDBMSBIN)extproc
HSALLOCI=$(RDBMSBIN)hsalloci
HSSQLPCC=$(RDBMSBIN)hssqlpcc
HSDEPXA=$(RDBMSBIN)hsdepxa
HSOTS=$(RDBMSBIN)hsots
OGMS=$(RDBMSBIN)ogms
OGMSCTL=$(RDBMSBIN)ogmsctl
LKDBX=$(RDBMSBIN)lkdbx
ARCHMON=$(RDBMSBIN)archmon
OSH=$(RDBMSBIN)osh
RMAN=$(RDBMSBIN)rman
BBED=$(RDBMSBIN)bbed
SBTTEST=$(RDBMSBIN)sbttest

ALL_OPTS= $(RDBMSLIB)$(VER_ENT) $(RDBMSLIB)$(PLS_ON) $(MTS_ON) $(RDBMSLIB)$(OPS_ON) $(RDBMSLIB)$(DDB_ON) $(RDBMSLIB)$(REP_ON) $(RDBMSLIB)$(SCF_ON) $(RDBMSLIB)$(SDO_ON) $(HO_ON) $(RDBMSLIB)$(OBJ_ON) $(RDBMSLIB)$(PART_ON) $(RDBMSLIB)$(LIL_OFF) $(RDBMSLIB)$(KPT_OFF)

NO_OPTS= $(RDBMSLIB)$(VER_ENT) $(RDBMSLIB)$(PLS_ON) $(MTS_ON) $(RDBMSLIB)$(OPS_OFF) $(RDBMSLIB)$(DDB_ON) $(RDBMSLIB)$(REP_ON) $(RDBMSLIB)$(SCF_ON) $(RDBMSLIB)$(SDO_OFF) $(HO_ON) $(RDBMSLIB)$(OBJ_OFF) $(RDBMSLIB)$(PART_OFF) $(RDBMSLIB)$(LIL_OFF) $(RDBMSLIB)$(KPT_OFF)

MTS_NCR_ON= $(LIBHOME)ncrstab.o
MTS_ON= $(RDBMSLIB)kmc.$(OBJ_EXT) $(RDBMSLIB)kmd.$(OBJ_EXT) $(RDBMSLIB)kmm.$(OBJ_EXT) $(RDBMSLIB)kmn.$(OBJ_EXT) $(RDBMSLIB)kmcro.$(OBJ_EXT) $(LIBHOME)ncrstab.o
MTS_KMC= kmc.$(OBJ_EXT)
MTS_KMD= kmd.$(OBJ_EXT)
MTS_KMM= kmm.$(OBJ_EXT)
MTS_KMN= kmn.$(OBJ_EXT)
MTS_KMCRO= kmcro.$(OBJ_EXT)
MTS_NCR= ncrstab.$(OBJ_EXT)

# Entering /olympia/src805/rdbms/s_rdbms.mk
RANLIBKNLOPT=
RANLIBORA=
OTHERST=
GENARGS=-n F
LDFLAGS_ORA=
LIBLISTRDBMS=
LLAIO=
LLIBPOSIX4=

LLIBSQL=-lsql
CDEBUG=-g

 NETLIBS=-lnetv2 -lnttcp -lnetwork -lncr 
LDLIBS+=-ldl $(NETLIBS) $(NETLIBS) $(NETLIBS) $(NETLIBS) $(NETLIBS) \
        $(LLIBSQL) $(LLIBSQL)

LLIBOBK=-lobk


LIBNM=$(CLUSTERDIR)/lib/libudlm.so
LIBCLUST=$(CLUSTERDIR)/lib/libcdb.so
LLIBNM=$(SKGXN) -ludlm
LLIBCLUST=-lcdb
# Exiting /olympia/src805/rdbms/s_rdbms.mk
# Exiting rdbms.mk
# Entering /olympia/src805/buildtools/mkfile/cus_plfrm.mk

# CC=cc
CC=gcc
CCFLAGS=
SPFLAGS=
OPTIMIZE=
LINK=cc
MAKE=make
LLIBTHREAD=-lcrypt -lndbm
AR=ar
LLIBOBK=

LIBDLM=`echo "$(SKGXN) /usr/lib/libdlm.so"`

LLIBDLM=`echo "$(SKGXN) -ldlm`

LIBNM=$(LIBDLM)
LLIBNM=$(LLIBDLM)

# Exiting /olympia/src805/buildtools/mkfile/cus_plfrm.mk
# Entering /olympia/src805/buildtools/mkfile/libclntsh.mk
$(LIBCLNTSH): \
            ${ORACLE_HOME}/lib/libclient.a ${ORACLE_HOME}/lib/libsql.a \
            ${ORACLE_HOME}/lib/libnetv2.a ${ORACLE_HOME}/lib/libnttcp.a \
            ${ORACLE_HOME}/lib/libnetwork.a ${ORACLE_HOME}/lib/libncr.a \
            ${ORACLE_HOME}/lib/libcommon.a ${ORACLE_HOME}/lib/libgeneric.a \
            ${ORACLE_HOME}/lib/libmm.a ${ORACLE_HOME}/rdbms/lib/xaondy.o \
            ${ORACLE_HOME}/lib/libnlsrtl3.a ${ORACLE_HOME}/lib/libcore4.a \
            ${ORACLE_HOME}/lib/libepc.a
	$(SILENT)$(ECHO) "Building client shared library libclntsh.so ..."
	$(SILENT)$(ECHO) "Call script $$ORACLE_HOME/bin/genclntsh ..."
	$(GENCLNTSH)
	$(SILENT)$(ECHO) "Built $$ORACLE_HOME/lib/libclntsh.so ... DONE"

# Exiting /olympia/src805/buildtools/mkfile/libclntsh.mk
