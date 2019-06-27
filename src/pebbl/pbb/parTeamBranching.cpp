// parTeamBranching.cpp
//
// Parallel search and computation class code for PEBBL.
//
// Will Loughlin and Rohith Rokkam

#include <pebbl_config.h>

#include <pebbl/bb/branching.h>
#include <pebbl/pbb/parBranching.h>
#include <pebbl/pbb/teamBranching.h>
#include <pebbl/pbb/parTeamBranching.h>
#include <pebbl/utilib/exception_mngr.h>



#include <sys/types.h>
#include <unistd.h>


#ifdef ACRO_HAVE_MPI

#include <pebbl/comm/mpiComm.h>


using namespace std;

namespace pebbl {

// Method used to properly allocate processors into teams, taking into account
// differences between working and non working hubs
void parallelTeamBranching::splitCommunicator(mpiComm argComm, 
                                              int teamSize,
                                              int clusterSize,
                                              int hubsDontWorkSize,
                                              mpiComm *search,
                                              mpiComm *bound){
  // TODO
  return;
  
}

double parallelTeamBranching::search(){
  setTeam(teamComm);
  if(iAmHead()){
    double objVal = parallelSearchFramework(NULL);
    alertTeam(exitOp);
    return objVal;
  }
  else {
    awaitWork();
    return nan("");
  }
}

void parallelTeamBranching::rampUpSearch(){
  setTeam(passedComm);
  parallelBranching::rampUpSearch();
}

void parallelTeamBranching::setupSearchComm(){
  // Free communicators we make in case setup is called more than once
  if(searchComm.myComm() != MPI_COMM_NULL){
    searchComm.free();
  }
  // freeing backupComm here makes sure that we don't free passedComm, since
  // teamComm = passedComm when we are in rampUp
  if(backupComm.myComm() != MPI_COMM_NULL){
    backupComm.free();
  }
  splitCommunicator(passedComm, teamSize, clusterSize, hubsDontWorkSize, &searchComm, &teamComm);
  backupComm = teamComm;
}
  
}
  

#endif
