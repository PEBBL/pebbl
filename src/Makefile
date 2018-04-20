#CXX    = mpic++
#C      = mpicc
CFLAGS = -fpermissive -g

LIBSRCS = bb/branching.cpp bb/loadObject.cpp bb/pebblBase.cpp bb/pebblParams.cpp \
       comm/coTree.cpp comm/MessageID.cpp comm/outBufferQ.cpp \
       example/parKnapsack.cpp example/parMonomial.cpp \
       example/serialKnapsack.cpp example/serialMonomial.cpp \
       misc/clustering.cpp misc/clusterTree.cpp misc/followerTree.cpp \
       misc/gRandom.cpp misc/memUtil.cpp misc/scatterObj.cpp \
       misc/treeTopology.cpp pbb/earlyOutputThread.cpp \
       pbb/hubThread.cpp pbb/incumbCast.cpp pbb/incumbSearchThread.cpp \
       pbb/llChainer.cpp pbb/loadBal.cpp \
       pbb/parBranching.cpp pbb/parBranchThreads.cpp pbb/parLoadObject.cpp \
       pbb/parPebblBase.cpp pbb/parPebblParams.cpp pbb/pbCheckpoint.cpp \
       pbb/pbEnumerate.cpp pbb/pbHub.cpp pbb/pbOutput.cpp pbb/pbPack.cpp \
       pbb/pbWorker.cpp pbb/reposThreads.cpp pbb/spReceiver.cpp \
       pbb/spServer.cpp pbb/workerAuxThread.cpp pbb/workerInHeap.cpp \
       pbb/workerThread.cpp sched/Scheduler.cpp sched/SelfAdjustThd.cpp \
       sched/ThreadObj.cpp sched/ThreadQueue.cpp utilib/BitArray.cpp \
       utilib/CharString.cpp utilib/CommonIO.cpp utilib/Ereal.cpp \
       utilib/globals.cpp utilib/logEvent.cpp \
       utilib/mpiUtil.cpp utilib/PackBuf.cpp utilib/Parameter.cpp \
       utilib/ParameterSet.cpp utilib/PM_LCG.cpp utilib/seconds.cpp \
       utilib/nicePrint.cpp utilib/hash_fn.cpp utilib/memdebug.cpp \
       utilib/ParameterList.cpp utilib/exception_mngr.cpp utilib/lookupa.cpp \
       utilib/Any.cpp utilib/ParkMiller.cpp utilib/TypeManager.cpp \
       utilib/comments.cpp utilib/math_basic.cpp 

EXESRCS = example/core_test.cpp example/knapMPS.cpp example/Docking.cpp \
          example/knapsack.cpp example/lipshitzian.cpp example/logAnalyze.cpp \
          example/monomial.cpp

PLIBSRCS = $(patsubst %,pebbl/%,$(LIBSRCS))
PEXESRCS = $(patsubst %,pebbl/%,$(EXESRCS))

LIBOBJS = $(patsubst %.cpp,%.o,$(PLIBSRCS))
EXEOBJS = $(patsubst %.cpp,%.o,$(PEXESRCS))
OBJS    = $(LIBOBJS) $(EXEOBJS)

EXES = $(patsubst %.cpp,%,$(PEXESRCS))

%.o : %.cpp
	$(CXX) -c -I. $(CFLAGS) $< -o $@

% : %.o
	$(CXX) $(CFLAGS) -o $@ $< $(LIBOBJS)

all : $(OBJS) $(EXES)

clean: 
	rm -f $(EXES) $(OBJS)
