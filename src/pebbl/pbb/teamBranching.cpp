// teamBranching.cpp
//
// Parallel computation class code for PEBBL.
//
// Will Loughlin and Rohith Rokkam

#include <pebbl_config.h>

#include <pebbl/bb/branching.h>
#include <pebbl/pbb/parPebblParams.h>
#include <pebbl/pbb/teamBranching.h>
#include <pebbl/utilib/exception_mngr.h>



#include <sys/types.h>
#include <unistd.h>


#ifdef ACRO_HAVE_MPI

#include <pebbl/comm/mpiComm.h>


using namespace std;

namespace pebbl {


void teamBranching::awaitWork() {
  parallelOp opCode;
  bool done = false;
  while(!done){
    teamComm.broadcast(&opCode, 1, MPI_INT, getHeadRank());
    switch(opCode) {
      case boundOp: 
        double controlParam;
        minionBound();
        break;
      case separateOp:
        minionSplit();
        break;
      case makeChildOp:
        minionMakeChild();
        break;
      case exitOp:
        done = true;
        break;
      default:
        EXCEPTION_MNGR(std::runtime_error, "Await work recieved bad opCode: " << opCode);
    }
  }
}

bool teamBranching::setup(int& argc, char**& argv) {
  // This is the branching::setup code with printout disabled for minion processors
  resetTimers();

  if (!processParameters(argc,argv,min_num_required_args))
    return false;
  if (iAmHead()) {
    if (plist.size() == 0) {
      ucout << "Using default values for all solver options" << std::endl;
    }
    else {
      ucout << "User-specified solver options: " << std::endl;
      plist.write_parameters(ucout);
      ucout << std::endl;
    }
  }
  set_parameters(plist,false);
  if ((argc > 0) && !checkParameters(argv[0]))
    return false;
  if (!setupProblem(argc,argv))
    return false;
  if (plist.unused() > 0) {
    if (iAmHead()) {
      ucout << "\nERROR: unused parameters: " << std::endl;
      plist.write_unused_parameters(ucout);
      ucout << utilib::Flush;
    }
    return false;
  }
  return true;
}

double teamBranching::search() {
  setTeam(teamComm);
  if(iAmHead()){
    double objVal = searchFramework(NULL);
    alertTeam(exitOp);
    return objVal;
  }
  else {
    awaitWork();
    return nan("");
  }
}

}
#endif 


