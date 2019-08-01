#include <math.h>
#include <stdlib.h>
#include <mpi.h>
#include <sstream>
#include <iostream>

using namespace std;

int setupCommunicator(MPI_Comm argComm, 
                      int teamSize,
                      int clusterSize,
                      int hubsDontWorkSize){
  int boundRank = -1;
  int headRank = -1;
  MPI_Comm searchComm = MPI_COMM_NULL;
  MPI_Comm teamComm = MPI_COMM_NULL;
	int worldRank, worldSize, errorCode;
  MPI_Comm_rank(argComm, &worldRank);
  MPI_Comm_size(argComm, &worldSize);
	
	// calculate the desired size of a cluster including bounding processors
	bool hubsWork = hubsDontWorkSize > clusterSize;

  // Size of a cluster including minion processors
	int fullClusterSize = !hubsWork +
		(clusterSize - !hubsWork) * teamSize;

	int clustersWanted = worldSize / fullClusterSize;
	int forceSeparateSize = 1 + (hubsDontWorkSize - 1) * teamSize;

//	worldCluster.reset(worldRank, worldSize, fullClusterSize, clustersWanted, forceSeparateSize);
// Copied math from the cluster object
  int typicalSize = (int) std::ceil(((double) worldSize)/std::max(clustersWanted,1));
  if (typicalSize > fullClusterSize)
    typicalSize = fullClusterSize;
  if (typicalSize < 1)
    typicalSize = 1;

  int numClusters = (int) std::ceil(((double) worldSize)/typicalSize);

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
  MPI_Comm_group(argComm, &worldGroup);
// end copied math from the cluster object

  std::stringstream teststream;
  teststream << "World Rank: " << worldRank <<std::endl << std::endl;

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

      teststream << "Team Range: " << range[0][0] << ", " << range[0][1] << ", " << range[0][2]<<std::endl;

    MPI_Group_range_incl(worldGroup, 1, range, &teamGroup);
    teststream << "team ID: " << teamGroup << std::endl;
      int teamSize, teamRank;
      MPI_Group_size(teamGroup, &teamSize);
      MPI_Group_rank(teamGroup, &teamRank);
      teststream << "teamSize: " << teamSize << std::endl;
      teststream << "teamRank: " << teamRank << std::endl;
      teststream << std::endl;

  }

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
      // range = [[0, lastPureHub, myClusterSize]]
      range[0][0] = 0;
      range[0][1] = lastPureHub;
      range[0][2] = typicalSize;

      teststream << "PureHubRange: " << range[0][0] << ", " << range[0][1] << ", " << range[0][2]<<std::endl;

      MPI_Group_range_excl(worldGroup, 1, range, &workerGroup);
      // range = [[0, worldSize - numPureLeaders - 1, teamSize]]
      range[0][0] = 0;
      range[0][1] = worldSize - numPureLeaders - 1;
      range[0][2] = teamSize;

      teststream << "Non minion Range: " << range[0][0] << ", " << range[0][1] << ", " << range[0][2]<<std::endl;

      MPI_Group_range_excl(workerGroup, 1, range, &minionGroup);
      MPI_Group_difference(worldGroup, minionGroup, &searchGroup);

      teststream << "searchGroup id: " << searchGroup << std::endl;

      MPI_Group_free(&workerGroup);
      MPI_Group_free(&minionGroup);

      int searchSize, searchRank;
      MPI_Group_size(searchGroup, &searchSize);
      MPI_Group_rank(searchGroup, &searchRank);
      teststream << "SearchSize: " << searchSize << std::endl;
      teststream << "SearchRank: " << searchRank << std::endl;
      teststream << std::endl;


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

  
//      std::cout << teststream.str();

  if(teamGroup != MPI_GROUP_NULL){
    MPI_Comm_create_group(argComm, teamGroup, 0, &teamComm);
  }

  if(searchGroup != MPI_GROUP_NULL){
    MPI_Comm_create_group(argComm, searchGroup, 0, &searchComm);
  }
  if (searchComm != MPI_COMM_NULL) {
    MPI_Comm_rank(searchComm, &headRank);
  }
  if (teamComm != MPI_COMM_NULL) {
    MPI_Comm_rank(teamComm, &boundRank);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  // printouts for debugging
  std::stringstream stream;
  stream << worldRank << " " << headRank << " " << boundRank << 
	  " " << iAmFollower << " " << iAmLeader << std::endl;
  std::cout << stream.str();
  for (int i = 0; i < worldSize; i++) {
    if (i == worldRank)
      std::cout << stream.str();
    MPI_Barrier(MPI_COMM_WORLD);
  }
}



int main (int argc, char **argv)
{
  MPI_Init(&argc, &argv);

  if (argc < 4)
  {
    std::cout << "USAGE: hubsDontWorkSize clusterSize boundingGroupSize numClusters" << std::endl;
    return 1;
  }
  
  int hubsDontWorkSize = strtol(argv[1], NULL, 10);
  int clusterSize = strtol(argv[2], NULL, 10);
  int boundingGroupSize = strtol(argv[3], NULL, 10);
  
  setupCommunicator(MPI_COMM_WORLD, hubsDontWorkSize, clusterSize, boundingGroupSize);
  
  MPI_Finalize();
  return 0;
}
