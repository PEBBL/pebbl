##
## Set TOPBUILD to the top of your acro build directory.
##
## See the README file for further information.
##

include ../buildFlags.txt

TOPBUILD=../..
TOPBIN=$(TOPBUILD)/bin
TOPINC=$(TOPBUILD)/include
TOPLIB=$(TOPBUILD)/lib

## Uncomment to use GLPK
GLPK_LIB= -lOsiGlpk -lglpk

## Uncomment to use SOPLEX
SOPLEX_LIB= -lOsiSpx -lsoplex

## Uncomment to use APPSPACK
## LAPACK_LIBS=/usr/local/lib...
## BLAS_LIBS=/usr/local/lib...
## FLIBS =/usr/local/lib...
## APPSPACKLIB= $(TOPBUILD)/packages/appspack/src/libappspack.a \
##        $(LAPACK_LIBS) $(BLAS_LIBS) $(FLIBS)

## Uncomment to use 3PO
##THREEPOLIB=-l3po

## Uncomment if you need libdl
DLOPEN_LIBS=-ldl

## Uncomment if you need CPLEX
##CPLEX_LIB=/usr/local/lib...
##CPLEX_INCLUDES=-I/

LIBFLAGS=-L$(TOPLIB) \
        $(TOPBUILD)/packages/pico/src/libpico.a\
        $(TOPBUILD)/packages/pebbl/src/libpebbl.a\
        $(TOPBUILD)/packages/utilib/src/.libs/libutilib.a\
        -lCgl -lOsiClp -lClp $(SOPLEX_LIB) $(GLPK_LIB) \
        -lOsi -lCoinUtils $(CPLEX_LIB) \
         -lamplsolver $(DLOPEN_LIBS)

#
# Paths to:
#  all pico headers, to config.h, and to other configuration headers.
#
INCLUDE_FLAGS=-I. -I$(TOPINC)/pico -I$(TOPINC) \
        -I$(TOPINC)/coin -I$(TOPINC)/soplex \
        -I$(TOPINC)/glpk -I$(TOPINC)/ampl $(CPLEX_INCLUDES)

OTHER_FLAGS=-DHAVE_CONFIG_H

FLAGS=$(INCLUDE_FLAGS) $(OTHER_FLAGS)

all: pmedian

pmedian: VolVolume.o pmedian_info.o pmedian_milp.o pmedian_extras.o
	$(CXX) $(FLAGS) -o pmedian \
           VolVolume.o pmedian_info.o pmedian_milp.o pmedian_extras.o \
           $(LIBFLAGS)

VolVolume.o: VolVolume.cpp VolVolume.h
	$(CXX) -I. -c VolVolume.cpp

pmedian_info.o: pmedian_info.cpp pmedian_info.h
	$(CXX) $(FLAGS) -c pmedian_info.cpp

pmedian_milp.o: pmedian_milp.cpp pmedian_milp.h pmedian_extras.h pmedian_info.h
	$(CXX) $(FLAGS) -c pmedian_milp.cpp

pmedian_extras.o: pmedian_extras.cpp pmedian_extras.h pmedian_milp.h pmedian_info.h ufl.h
	$(CXX) $(FLAGS) -c pmedian_extras.cpp

clean:
	@rm -f *.o pmedian

GEN_MILP_APP_OUTPUT= \
	ampl_script.in   pmedian.solution.dat \
        pmedian_milp.h pmedian_milp.cpp \
        pmedian_info.h       pmedian_info.cpp      \
        Makefile-pmedian   \
        pmedian.mps pmedian.map \
        pmedian.row pmedian.val pmedian.col

really-clean:
	@rm -f *.o pmedian $(GEN_MILP_APP_OUTPUT) *.txt
