SRC_BINDIR=@top_srcdir@/bin
BLD_BINDIR=@top_builddir@/bin
CP_U=$(SRC_BINDIR)/cp_u

#
# These application directories are not part of the autoconf'd
# directory tree.  The user will work in these directories after
# acro has been built.  Top level "make", etc. does not descend
# into these application directories.
#

PMEDIAN=pmedian.dat pmedian_extras.h README ufl.h VolVolume.cpp \
        Makefile pmedian_extras.cpp pmedian.mod ufl.par VolVolume.h

PMEDIAN_SRC=$(PMEDIAN:%=@srcdir@/pmedian/%)

SIMPLE=README simple_sched_extras.cpp simple-sched.mod \
       Makefile simple-sched.dat simple_sched_extras.h

SIMPLE_SRC=$(SIMPLE:%=@srcdir@/simple_sched/%)

COLIN_HJ=README hooke.c Makefile main.cpp Hooke.h Hooke.cpp

COLIN_HJ_SRC=$(SIMPLE:%=@srcdir@/colin_hj/%)

## if test -f Makefile.am
##   then this is not an out-of-source build
## If it is an out-of-source build, copy over the application directories

all-local:
	@if test -d simple_sched ; then : ; else mkdir simple_sched ; fi
	@if test -d pmedian ; then : ; else mkdir pmedian ; fi
	@if test -d bin ; then : ; else mkdir bin ; fi
	@if test -f Makefile.am ; then : ; else \
            $(CP_U) $(SIMPLE_SRC) simple_sched ; \
            $(CP_U) $(PMEDIAN_SRC) pmedian ; \
            $(CP_U) $(COLIN_SRC) colin_hj ; \
            $(CP_U) @srcdir@/bin/gen_*_app bin ; \
         fi

clean-local:
	@cd simple_sched && make clean 
	@cd pmedian && make clean 
	@cd colin_hj && make clean 

EXTRA_DIST=pmedian simple_sched doc bin colin_hj
