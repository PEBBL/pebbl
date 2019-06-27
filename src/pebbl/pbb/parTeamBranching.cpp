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

  // not sure how to deal with this. Might have to make it static to deal with the need 
  // to set the seach communicator on instantiation
void parallelTeamBranching::splitCommunicator(mpiComm worldComm, int teamSize, int clusterSize, int hubsDontWorkSize){
  return;
}

double parallelTeamBranching::search(){
  setTeam(teamComm);
  if(iAmHead()){
    return parallelSearchFramework(NULL);
  }
  else {
    awaitWork();
  }
  return 0;
}

void parallelTeamBranching::rampUpSearch(){
  setTeam(worldComm);
  parallelBranching::rampUpSearch();
}

bool parallelTeamBranching::setup(int& argc, char**& argv, mpiComm worldComm = MPI_WORLDCOMM){
  return true;
}

#endif