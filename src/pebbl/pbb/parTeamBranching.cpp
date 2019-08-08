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
// returns 0 on success, or an errorcode if an mpi call fails
void parallelTeamBranching::splitCommunicator(mpiComm argComm,
                                              int teamSize,
                                              int clusterSize,
                                              int hubsDontWorkSize,
                                              mpiComm *search,
                                              mpiComm *team){
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

//	worldCluster.reset(worldRank, worldSize, fullClusterSize, clustersWanted, forceSeparateSize);
// Copied math from the cluster object
  int typicalSize = (int) ceil(((double) worldSize)/std::max(clustersWanted,1));
  if (typicalSize > fullClusterSize)
    typicalSize = fullClusterSize;
  if (typicalSize < 1)
    typicalSize = 1;

  int numClusters = (int) ceil(((double) worldSize)/typicalSize);

  int clusterNumber   = worldRank/typicalSize;
  int leader          = clusterNumber*typicalSize;
  int myClusterSize     = std::min(typicalSize,worldSize - leader);
  int lastClusterSize = worldSize - (numClusters - 1)*typicalSize;

  int separateFunctions  = (myClusterSize     >= forceSeparateSize);
  int typicallySeparated = (typicalSize     >= forceSeparateSize);
  int lastSeparated      = (lastClusterSize >= forceSeparateSize);

  int positionInCluster    = worldRank - leader;
  int iAmLeader            = (positionInCluster == 0);
  int iAmFollower          = !iAmLeader || !separateFunctions;

  int numPureLeaders       = (numClusters - 1)*typicallySeparated + lastSeparated;

  MPI_Group worldGroup;
  MPI_Comm_group(argComm.myComm(), &worldGroup);
// end copied math from the cluster object


  // create group for the team communicator
  int range[1][3];
  MPI_Group teamGroup = MPI_GROUP_NULL;
  // Only followers need a team Comm
  if(iAmFollower) {
    // separateFunctions indicates whether our cluster has a pure hub or not. In the case of a pure hub
    // we must exclude the head of the cluster from the group
    int firstIncluded = leader + separateFunctions;
    int teamNumber = (positionInCluster - separateFunctions)/teamSize;
    int teamLeader = firstIncluded + teamNumber*teamSize;
    // range = [[teamLeader, teamLeader + teamSize - 1, 1]]
    range[0][0] = teamLeader;
    range[0][1] = teamLeader + teamSize - 1;
    range[0][2] = 1;
    MPI_Group_range_incl(worldGroup, 1, range, &teamGroup);
  }

  // Determine which processesors are part of the search
  int teamGroupRank = -1;
  if(teamGroup != MPI_GROUP_NULL){
    teamGroupRank = MPI_Group_rank(teamGroup, &teamGroupRank);
  }

  // create group for the search communicator
  MPI_Group searchGroup = MPI_GROUP_NULL;
  if(teamGroup == MPI_GROUP_NULL || teamGroupRank == 0){
    if(typicallySeparated){
      MPI_Group workerGroup = MPI_GROUP_NULL; // This group will contain all processors with a team i.e. non-pure hubs
      MPI_Group minionGroup = MPI_GROUP_NULL; // Group with all non-head workers
      int lastPureHub = (numClusters - 1)*typicalSize - (typicalSize * !lastSeparated); // beginning of second to last cluster if last cluster does not have a pure hub
      // range = [[0, lastPureHub, typicalSize]]
      range[0][0] = 0;
      range[0][1] = lastPureHub;
      range[0][2] = typicalSize;
      MPI_Group_range_excl(worldGroup, 1, range, &workerGroup);
      // range = [[0, worldSize - numPureLeaders - 1, teamSize]]
      range[0][0] = 0;
      range[0][1] = worldSize - numPureLeaders - 1;
      range[0][2] = teamSize;
      MPI_Group_range_excl(workerGroup, 1, range, &minionGroup);
      MPI_Group_difference(worldGroup, minionGroup, &searchGroup);
      MPI_Group_free(&workerGroup);
      MPI_Group_free(&minionGroup);
    }
    else {
      // Every processor is in a team so we just group by team size
      //range = [[0, worldSize - 1, teamSize]]
      range[0][0] = 0;
      range[0][1] = worldSize - 1;
      range[0][2] = teamSize;
      MPI_Group_range_incl(worldGroup, 1, range, &searchGroup);
    }
  }


  MPI_Comm teamComm = MPI_COMM_NULL;
  if(teamGroup != MPI_GROUP_NULL){
    MPI_Comm_create_group(argComm.myComm(), teamGroup, 0, &teamComm);
  }
  *team = mpiComm(teamComm);

  MPI_Comm searchComm = MPI_COMM_NULL;
  if(teamGroup != MPI_GROUP_NULL){
    MPI_Comm_create_group(argComm.myComm(), searchGroup, 0, &searchComm);
  }
  *search = mpiComm(searchComm);
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
  mpiComm savedComm = teamComm;
  setTeam(passedComm);
  parallelBranching::rampUpSearch();
  setTeam(savedComm);
}

  
}
  

#endif
