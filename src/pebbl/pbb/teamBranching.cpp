// teamBranching.cpp
//
// Parallel computation class code for PEBBL.
//
// Will Loughlin and Rohith Rokkam

#include <pebbl_config.h>

#include <pebbl/bb/branching.h>
#include <pebbl/pbb/teamBranching.h>
#include <pebbl/utilib/exception_mngr.h>



#include <sys/types.h>
#include <unistd.h>


#ifdef ACRO_HAVE_MPI

#include <pebbl/comm/mpiComm.h>


using namespace std;

namespace pebbl {

void teamBranching::setTeam(mpiComm comm) {
  teamComm = comm;
  teamOrganize();
}

bool teamBranching::alertTeam(parallelOp op) {
  if(!iAmHead()){
    return false;
  }
  teamComm.broadcast(&op, 1, MPI_INT, teamComm.myRank());
  return true;
}

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

bool teamBranching::iAmHead() {
  return teamComm.myRank() == getHeadRank();
}

bool teamBranching::iAmMinion() {
  return !iAmHead();
}

int teamBranching::getHeadRank() {
  return 0;
}

bool teamBranching::setup(int& argc, char**& argv, mpiComm teamComm) {
  this->teamComm = teamComm;
  branching::setup(argc, argv);
}


bool teamBranching::alertBound() {
  return alertTeam(boundOp);
}

bool teamBranching::alertSeparate() {
  return alertTeam(separateOp);
}

bool teamBranching::alertMakeChild() {
  return alertTeam(makeChildOp);
}

bool teamBranching::alertExit() {
  return alertTeam(exitOp);
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


