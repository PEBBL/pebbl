// parTeamBranching.cpp
//
// Parallel search and computation class code for PEBBL.
//
// Will Loughlin and Rohith Rokkam



 
#ifndef pebbl_parTeamBranching_h
#define pebbl_parTeamBranching_h


#ifdef ACRO_HAVE_MPI

namespace pebbl {

// Class that will implement methods used by the application for
// parallel search with computation
  class parallelTeamBranching : public virtual teamBranching,
                                public virtual parallelBranching
  {
    protected:

      // Communicator to hold the teamComm during operations like ramp up
      mpiComm backupComm; 

    public:
      
      // Splits a world comm into team comms and a search comm according to the parameters passed to pebbl
      void splitCommunicator(mpiComm worldComm, int teamSize, int clusterSize, int hubsDontWorkSize, mpiComm *search, mpiComm *team);

      // Overrides the search function of parBranching
      virtual double search(); 

      // Override ramp up search to use the global communicator during ramp up
      void rampUpSearch();

      // Override the setupSeachComm method called in parallelBranching::setup()
      // to now split the communicators and initialize the searchComm and the boundComm
      void setupSearchComm();

      parallelTeamBranching(){
        // TODO
      }

      ~parallelTeamBranching(){
        // TODO
      }
  };
}

#endif
#endif

