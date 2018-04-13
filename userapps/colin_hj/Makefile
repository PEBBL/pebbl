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

LIBFLAGS=-L$(TOPLIB) \
        $(TOPBUILD)/packages/colin/src/libcolin.a\
        $(TOPBUILD)/packages/utilib/src/.libs/libutilib.a -ltinyxml -lamplsolver -l3po -lm

#
# Paths to:
#  all pico headers, to config.h, and to other configuration headers.
#
INCLUDE_FLAGS=-I. -I$(TOPINC) -I$(TOPINC)/tinyxml
OTHER_FLAGS=-DHAVE_CONFIG_H
FLAGS=$(INCLUDE_FLAGS) $(OTHER_FLAGS)

all: colintest

colintest: colintest.o Example.o
	$(CXX) $(FLAGS) -o colintest \
           colintest.o Example.o $(LIBFLAGS)

colintest.o: colintest.cpp
	$(CXX) $(FLAGS) -c colintest.cpp

Example.o: Example.cpp Example.h
	$(CXX) $(FLAGS) -c Example.cpp

clean:
	@rm -f *.o colintest
