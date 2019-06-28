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
#include <pebbl/misc/clustering.h>



#include <sys/types.h>
#include <unistd.h>


#ifdef ACRO_HAVE_MPI

#include <pebbl/comm/mpiComm.h>


using namespace std;

namespace pebbl {

// Method used to properly allocate processors into teams, taking into account
// differences between working and non working hubs
// returns 0 on success, or an errorcode if an mpi call fails
int parallelTeamBranching::splitCommunicator(mpiComm argComm, 
                                              int teamSize,
                                              int clusterSize,
                                              int hubsDontWorkSize,
                                              mpiComm *search,
                                              mpiComm *team){
	clusterObj worldCluster; // use to find pure hubs, then throw away
	int worldRank, worldSize, errorCode;
  worldRank = argComm.myRank();
  worldSize = argComm.mySize();
	
	// calculate the desired size of a cluster including bounding processors
	bool hubsWork = hubsDontWorkSize > clusterSize;

  // Size of a cluster including minion processors
	int fullClusterSize = !hubsWork +
		(clusterSize - !hubsWork) * teamSize;

	int clustersWanted = worldSize / fullClusterSize;
	int forceSeparateSize = 1 + (hubsDontWorkSize - 1) * teamSize;

  // Initialize the cluster
	worldCluster.reset(worldRank, worldSize, clustersWanted, fullClusterSize, forceSeparateSize);

	// create an intermediate communicator that excludes pure hubs
	int inBoundingGroup = worldCluster.isFollower(worldRank);
	MPI_Comm rawIntComm;
	if (errorCode = MPI_Comm_split(argComm.myComm(), inBoundingGroup, worldRank, &rawIntComm)) {
    ucerr << "MPI_Comm_split failed, code " << errorCode << endl;
		return errorCode;
	}

  mpiComm intComm(rawIntComm);


	// form bounding groups out of intermediate processor
  MPI_Comm rawTeamComm;
	if (inBoundingGroup)
	{
		int groupNum = intComm.myRank() / teamSize;
		if (errorCode = MPI_Comm_split(intComm.myComm(), groupNum, worldRank, &rawTeamComm)) {
      ucerr << "MPI_Comm_split failed, code " << errorCode << endl;
      intComm.free();
		  return errorCode;
		}
    *team = mpiComm(rawTeamComm);
	}
  // For pure hubs not in a team, the teamComm is set to null
  else {
    *team = mpiComm();
  }


	// Determine pebbl head processors and put them in the search communicator
  MPI_Comm rawSearchComm;
	int isWorker = (intComm.myRank() % teamSize == 0);
	int isMinion = !worldCluster.isLeader(worldRank) && !isWorker;
	if (errorCode = MPI_Comm_split(argComm.myComm(), !isMinion, worldRank, &rawSearchComm)) {
      ucerr << "MPI_Comm_split failed, code " << errorCode << endl;
      intComm.free();
      return errorCode;
  }
  *search = mpiComm(rawSearchComm);
  // Free the search comm and set it to null for minions
	if (isMinion)
	{
    search->free();
	}

  // Free the intermediate communicator
  intComm.free();

  return 0;
}

double parallelTeamBranching::search(){
  setTeam(backupComm);
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
  searchComm.free();
  // freeing backupComm here makes sure that we don't free passedComm, since
  // teamComm = passedComm when we are in rampUp
  backupComm.free();

  splitCommunicator(passedComm, teamSize, clusterSize, hubsDontWorkSize, &searchComm, &backupComm);
}
  
}
  

#endif
