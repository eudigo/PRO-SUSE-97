#
# Example for building demo OCI programs:
#
# 1. All OCI demos:
#
#    make -f demo_rdbms.mk demos
#
# 2. A single OCI demo:
#
#    make -f demo_rdbms.mk build EXE=demo OBJS="demo.o ..."
#    e.g. make -f demo_rdbms.mk build EXE=oci02 OBJS=oci02.o
#
# 3. A single OCI demo with static libraries:
#
#    make -f demo_rdbms.mk build_static EXE=demo OBJS="demo.o ..."
#    e.g. make -f demo_rdbms.mk build_static EXE=oci02 OBJS=oci02.o
#
# 4. To re-generate shared library:
#
#    make -f demo_rdbms.mk generate_sharedlib
#
# Example for building external procedures demo programs:
#
# 1. All external procedure demos:
#
# 2. A single external procedure demo without database callbacks:
#
#    make -f demo_rdbms.mk extproc_nocallback SHARED_LIBNAME=libname 
#                                             OBJS="demo.o ..."
#    e.g. make -f demo_rdbms.mk extproc_nocallback SHARED_LIBNAME=epdemo.so
#                                             OBJS="epdemo1.o epdemo2.o"
#
# 3. A single external procedure demo with database callbacks:
#
#    make -f demo_rdbms.mk extproc_callback SHARED_LIBNAME=libname 
#                                             OBJS="demo.o ..."
#    e.g. make -f demo_rdbms.mk extproc_callback SHARED_LIBNAME=epdemo.so
#                                             OBJS="epdemo1.o epdemo2.o"
#
# 4. To link C++ demos:
#
#    make -f demo_rdbms.mk c++demos
#
#
# NOTE: 1. ORACLE_HOME must be either:
#		   . set in the user's environment
#		   . passed in on the command line
#		   . defined in a modified version of this makefile
#
#       2. If the target platform support shared libraries (e.g. Solaris)
#          look in the platform specific documentation for information
#          about environment variables that need to be properly
#          defined (e.g. LD_LIBRARY_PATH in Solaris).
#


# include $(ORACLE_HOME)/rdbms/lib/env_rdbms.mk
include env_rdbms.mk


RDBMSLIB=$(ORACLE_HOME)/rdbms/lib/

LDFLAGS=-L$(LIBHOME) -L$(ORACLE_HOME)/rdbms/lib
LLIBPSO=`cat $(ORACLE_HOME)/rdbms/lib/psoliblist`

FC=`if $(ORACLE_HOME)/bin/get_platform  | $(GREP) "DG" > /dev/null ; then \
     echo 'epcf77' ; \
    else \
     echo 'lpifortran' ; \
   fi`

COB=cob
COBFLAGS=-C IBMCOMP -x
COBGNTFLAGS=-C IBMCOMP -u
CPLPL=`if $(ORACLE_HOME)/bin/get_platform  | $(GREP) "DG" > /dev/null ; then \
       echo 'ec++' ; \
       else \
       if $(ORACLE_HOME)/bin/get_platform  | $(GREP)  "NCR" > /dev/null; then \
       echo 'cc' ; \
       else \
       echo 'CC' ; \
       fi; \
       fi`

# directory that contain oratypes.h and other oci demo program header files
INCLUDE= -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/rdbms/public -I$(ORACLE_HOME)/plsql/public -I$(ORACLE_HOME)/network/public

#
CONFIG = $(ORACLE_HOME)/rdbms/lib/config.o

# module to be used for linking with non-deferred option

# flag for linking with non-deferred option (default is deferred mode)
NONDEFER=false

# libraries for linking oci programs
OCISHAREDLIBS=$(TTLIBS) $(LLIBTHREAD)
OCISTATICLIBS=$(STATICTTLIBS) $(LLIBTHREAD)

PSOLIBLIST=$(ORACLE_HOME)/rdbms/lib/psoliblist
CLEANPSO=rm -f $(PSOLIBLIST); $(GENPSOLIB)

DOLIB=$(ORACLE_HOME)/lib/liborcaccel.a
DUMSDOLIB=$(ORACLE_HOME)/lib/liborcaccel_stub.a
REALSDOLIB=/usr/lpp/orcaccel/liborcaccel.a


PROC=$(ORACLE_HOME)/bin/proc
PCCINCLUDE= include=$(ORACLE_HOME)/precomp/public
PCCI=-I$(ORACLE_HOME)/precomp/public
USERID=scott/tiger
PCCPLSFLAGS= $(PCCINCLUDE) ireclen=132 oreclen=132 sqlcheck=full \
ltype=none user=$(USERID)
LLIBSQL= -lsql
PROLDLIBS= $(LLIBSQL) $(TTLIBS)

DEMO_MAKEFILE = demo_rdbms.mk
 
DEMOS = cdemo1 cdemo2 cdemo3 cdemo4 cdemo5 cdemo81 cdemo82 \
        cdemobj cdemolb cdemodsc cdemocor cdemolb2 \
        cdemodr1 cdemodr2 cdemodr3 cdemodsa obndra \
        oci02 oci03 oci04 oci05 oci06 oci07 oci08 oci09 oci10 \
        oci11 oci12 oci13 oci14 oci15 oci16 oci17 oci18 oci19 oci20 \
        oci21 oci22 oci23 oci24 oci25 readpipe 
C++DEMOS = cdemo6


.SUFFIXES: .o .cob .for .cc


demos: $(DEMOS)

generate_sharedlib:
	$(SILENT)$(ECHO) "Building client shared library ..."
	$(SILENT)$(ECHO) "Calling script $$ORACLE_HOME/bin/genclntsh ..."
	$(GENCLNTSH)
	$(SILENT)$(ECHO) "The library is $$ORACLE_HOME/lib/libclntsh.so ... DONE"

MAKECPLPLDEMO= \
	@if [ "$(NONDEFER)" = "true" -o "$(NONDEFER)" = "TRUE" ] ; then \
	$(ECHODO) $(CPLPL) $(LDFLAGS) -o $(EXE) $? $(NDFOPT) $(OCISHAREDLIBS); \
	else \
	$(ECHODO) $(CPLPL) $(LDFLAGS) -o $(EXE) $? $(OCISHAREDLIBS); \
	fi

$(DEMOS):
	$(MAKE) -f $(DEMO_MAKEFILE) build EXE=$@ OBJS=$@.o

c++demos: $(C++DEMOS)

$(C++DEMOS):
	$(MAKE) -f $(DEMO_MAKEFILE) buildc++ EXE=$@ OBJS=$@.o

buildc++: $(LIBCLNTSH) $(OBJS)
	$(MAKECPLPLDEMO)

.cc.o:
	$(CPLPL) -c $(INCLUDE) $<

build: $(LIBCLNTSH) $(OBJS)
	$(ECHODO) $(CC) $(LDFLAGS) -o $(EXE) $(OBJS) $(DEF_OPT) $(SSCOREED) $(OCISHAREDLIBS)

build_static: $(OBJS)
	$(ECHODO) $(CC) $(LDFLAGS) -o $(EXE) $(OBJS) $(SSDBED) $(DEF_ON) $(OCISTATICLIBS)

extproc_nocallback: $(OBJS)
	$(LD) $(SHARED_LDFLAG) $(SHARED_LIBNAME) $(OBJS)

extproc_callback: $(OBJS) $(LIBCLNTSH)
	$(LD) $(SHARED_LDFLAG) $(SHARED_LIBNAME) $(OBJS) $(DEF_OPT) $(OCISHAREDLIBS) $(LIBEXTP)

clean:
	$(RM) -f $(DEMOS) *.o *.so

