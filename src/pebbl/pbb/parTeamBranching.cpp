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

  void parallelTeamBranching::splitCommunicator()
  {
    int worldRank, worldSize;
    worldRank = passedComm.myRank();
    worldSize = passedComm.mySize();

    // Terminology: a "full cluster" is a cluster including all minions
    //              a "cluster" is just the head nodes in a a full cluster

	  // calculate the desired size of a cluster including bounding processors
    bool hubsWork = hubsDontWorkSize > clusterSize;

    // Size of a cluster including minion processors
    int fullClusterSize = !hubsWork + (clusterSize - !hubsWork) * teamSize;

    int clustersWanted = worldSize / fullClusterSize;
    int forceSeparateSize = 1 + (hubsDontWorkSize - 1) * teamSize;

    clusterObj fc;                                    // "fc" stands for "full cluster"
    fc.reset(worldRank, worldSize, fullClusterSize, 
             clustersWanted, forceSeparateSize);

    // Figure out if there are any processors that cannot be used

    int throwAway 
       = (fc.lastClusterSize - (fc.lastClusterSize >= forceSeparateSize)) % teamSize;
    DEBUGPR(10, ucout << throwAway << " processors are idled\n");
    int usedSize = worldSize - throwAway;
    DEBUGPR(10, ucout << usedSize << " processors in use\n");

    // Thrown-away processors cannot be used -- sad!

    idleFlag = worldRank >= (worldSize - throwAway);
    DEBUGPR(10, ucout << "idleFlag = " << idleFlag << endl);

    // Recompute clustering with idled processors removed
    if (!idleFlag)
    {
      clustersWanted = usedSize / fullClusterSize;
      fc.reset(worldRank, usedSize, fullClusterSize, 
               clustersWanted, forceSeparateSize);
    }

    // Make a group out of all the ranks we're dealing with, including any idled processors
    MPI_Group worldGroup;
    MPI_Comm_group(passedComm.myComm(), &worldGroup);

    // We'll use this repeatedly below
    int range[1][3];

    // usedGroup is all the ranks we're dealing with, except those that are idled
    // in the Group_range_excl call, throwAway > 0 is 0 if we don't have to throw 
    // anything away, in which case we just copy the group
    MPI_Group usedGroup;
    mpiComm::setRange(range,worldSize - throwAway, worldSize - 1, 1); 
    MPI_Group_range_excl(worldGroup,throwAway > 0,range,&usedGroup);

    // create group for the team communicator
    MPI_Group teamGroup = MPI_GROUP_NULL;

    // Only followers need a team Communicator
    if(!idleFlag && fc.iAmFollower) 
    {
      DEBUGPR(10, ucout << "I am in a team within cluster " << fc.clusterNumber << endl);
      // separateFunctions indicates whether our cluster has a pure hub or not. 
      // In the case of a pure hub, we must exclude the head of the cluster from the group
      int firstIncluded = fc.leader + fc.separateFunctions;
      int teamNumber    = (fc.positionInCluster - fc.separateFunctions)/teamSize;
      int teamLeader    = firstIncluded + teamNumber*teamSize;
      // range = [[teamLeader, teamLeader + teamSize - 1, 1]]
      mpiComm::setRange(range,teamLeader, teamLeader + teamSize - 1, 1);
      MPI_Group_range_incl(usedGroup, 1, range, &teamGroup);
    }

    // Determine which processesors are "heads" and thus part of the search communicator
    int teamGroupRank = -1;
    if(teamGroup != MPI_GROUP_NULL)
      MPI_Group_rank(teamGroup, &teamGroupRank);

    // create group for the search communicator
    MPI_Group searchGroup = MPI_GROUP_NULL;
    if (!idleFlag && (teamGroup == MPI_GROUP_NULL || teamGroupRank == 0))
    {
      DEBUGPR(10,ucout << "I am in the search group\n");
      if (fc.typicallySeparated)
      {
        DEBUGPR(10,"Setup with pure hubs\n");
        // This group will contain all processors with a team, that is,
        // everything except pure hubs
        MPI_Group workerGroup = MPI_GROUP_NULL; 
        // This group will contain all workers minions 
        MPI_Group minionGroup = MPI_GROUP_NULL; 
        // Beginning of second to last cluster if last cluster does not have a pure hub
        int lastPureHub = (numClusters - 1)*fc.typicalSize 
                              - (fc.typicalSize * !fc.lastSeparated); 
        DEBUGPR(10, ucout << "lastPureHub = " << lastPureHub << std::endl);
        // range = [[0, lastPureHub, typicalSize]]
        mpiComm::setRange(range,0,lastPureHub,fc.typicalSize);
        // Pull all the pure hubs out of the used group to make the group of all workers
        MPI_Group_range_excl(usedGroup, 1, range, &workerGroup);
        // range = [[0, worldSize - numPureLeaders - 1, teamSize]]
        mpiComm::setRange(range,0,worldSize - fc.numPureLeaders - 1,teamSize);
        // Yank out the first of each span of "teamSize" of those to get the minions
        MPI_Group_range_excl(workerGroup, 1, range, &minionGroup);
        // All the non-minons are part of the search
        MPI_Group_difference(worldGroup, minionGroup, &searchGroup);
        MPI_Group_free(&workerGroup);
        MPI_Group_free(&minionGroup);
      }
      else 
      {
        DEBUGPR(10,ucout << "Setup without pure hubs\n");
        // Every processor is in a team so we just group by team size
        // range = [[0, worldSize - 1, teamSize]]
        mpiComm::setRange(range,0,usedSize - 1,teamSize);
        MPI_Group_range_incl(worldGroup, 1, range, &searchGroup);
      }
    }

    MPI_Group_free(&usedGroup);

  // MPI_Comm teamComm = MPI_COMM_NULL;
  // if(teamGroup != MPI_GROUP_NULL){
  //   MPI_Comm_create_group(passedComm.myComm(), teamGroup, 0, &teamComm);
  //   *team = mpiComm(teamComm);
  // }
  // else {
  //   *team = mpiComm();
  // }

    int searchGroupSize = 0;  //DBG
    if (searchGroup != MPI_GROUP_NULL)
      MPI_Group_size(searchGroup,&searchGroupSize);  //DBG

    DEBUGPR(10, ucout << "Search group size is " << searchGroupSize << std::endl);  //DBG

    searchComm.setup(passedComm,searchGroup);
    teamComm.setup(passedComm,teamGroup);

    DEBUGPR(10,ucout << "splitCommunicator complete\n"
                     << "searchComm rank " 
                     << searchComm.myRank() << " out of " 
                     << searchComm.mySize() << std::endl);
    DEBUGPR(10,ucout << "teamComm rank " 
                     << teamComm.myRank() << " out of " 
                     << teamComm.mySize() << std::endl);
  }


  double parallelTeamBranching::search()
  {
    // Set up the team if we're part of one
    if (!teamComm.isNull())
      setTeam(teamComm);
    // Participate in the search if this is a PEBBL search processor
    if(iAmHead())
    {
      double objVal = parallelSearchFramework(NULL);
      alertTeam(exitOp);
      return objVal;
    }
    // If a minion, just await commands
    // It is now possible to be an unused processor here, and bail right away
    if (iAmMinion())
      awaitWork();
    return nan("");
  }


  bool parallelTeamBranching::minionDispatch(parallelOp opCode)
  {
    switch(opCode)
    {
    // Handle the two commands that should only occur in the parallel teams settings
      case rampUpStartOp:
      rampUpFlag = true;
      rampUpSetup();
      return false;             // Not done with minion loop
      case rampUpEndOp:
      rampUpCleanUp();
      rampUpFlag = false;
      return false;             // Not done with minion loop
    }
    // If it's one of the ones that occurs in the just-teams setting too, use 
    // the just-teams dispatch code
    return teamBranching::minionDispatch(opCode);
  }

}


#endif
