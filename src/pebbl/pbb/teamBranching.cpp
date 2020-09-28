// teamBranching.cpp
//
// Teams-only parallel computation class code for PEBBL.
// Used when the only parallelism is within subproblems, and also as a base
// class for two-way parallelism
//
// Will Loughlin and Rohith Rokkam; later modifications by Jonathan Eckstein

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


// Minions run this to receive commands from the heads and then act on them

void teamBranching::awaitWork() {
  parallelOp opCode;
  bool done = false;
  while(!done){
    teamComm.broadcast(&opCode, 1, MPI_INT, getHeadRank());
    DEBUGPR(1,ucout << "Received team opCode " << opCode << std::endl);
    done = minionDispatch(opCode);
  }
}


// Figures out what to do with the minion commands
// Returns true when it's time to exit
bool teamBranching::minionDispatch(teamBranching::parallelOp opCode)
{
  switch(opCode) 
  {
    case boundOp: 
      minionBound();
      break;
    case separateOp:
      minionSplit();
      break; 
    case makeChildOp:
      minionMakeChild();
      break;
    case exitOp:
      return true;
    default:
      EXCEPTION_MNGR(std::runtime_error, "awaitWork recieved bad opCode: " << opCode);
  }
  return false;
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


void teamBranching::broadcastProbNameToTeam()
{
  if (iAmHead())
  {
    int nameLen = problemName.length();
    teamComm.broadcast(&nameLen,1,MPI_INT,0);
    teamComm.broadcast((void *) problemName.c_str(),nameLen,MPI_CHAR,0);
  }
  else if (iAmMinion())
  {
    int nameLen = -1;
    teamComm.broadcast(&nameLen,1,MPI_INT,0);
    char* nameBuf = new char[nameLen + 1];
    nameBuf[nameLen] = 0;
    teamComm.broadcast(nameBuf,nameLen,MPI_CHAR,0);
    problemName.assign(nameBuf);
  }
}

}
#endif 


