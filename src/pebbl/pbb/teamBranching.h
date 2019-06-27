// teamBranching.h
//
// Parallel computation class code for PEBBL.
//
// Will Loughlin and Rohith Rokkam



 
#ifndef pebbl_teamBranching_h
#define pebbl_teamBranching_h


#ifdef ACRO_HAVE_MPI

namespace pebbl {

// Class that will implement methods used by the application for
// parallel computation
  class teamBranching : public virtual branching 
  {
    protected: 

      // Communicator used by a bounding group for computation
      mpiComm teamComm; 

      // Counter for mpi messages sent in team Communication
      int teamMessageCount;

      enum parallelOp = { boundOp, separateOp, makeChildOp, exitOp }

      // Sets teamComm and calls teamOrganize
      void setTeam(mpiComm comm);

      // If this processor is the head of a team, alertTeam will send an MPI broadcast
      // to all processors in the team containing the op that needs to be performed.
      // Does nothing if the processor is not a head.
      bool alertTeam(parallelOp op);

      // Method called by minion processors in the search method. Processors in await work
      // will wait to be notified by the head before entering the correct parallel method.
      void awaitWork();

      // Pure virtual method used to by the application to initialize a team of processors.
      // Called from setTeam().
      virtual void teamOrganize() = 0;

    public:

      // Constructor for teamBranching
      branching() {
        teamComm = mpiComm();
        teamMessageCount = 0;
      }

      // True if this processor is the head of a team
      bool iAmHead();

      // True if this processor is a minion in a team
      bool iamMinion();

      // Returns the rank of this team's head
      // Currently this is always 0
      int getHeadRank();

      // Sets teamComm to the passed in commuicator before calling the setup() for branching
      bool setup(int& argc, char**& argv, mpiComm teamComm = MPI_COMM_WORLD);

      // Wrapper for alertTeam(boundOp)
      bool alertBound();

      // Wrapper for alertTeam(separateOp)
      bool alertSeparate();

      // Wrapper for alertTeam(makeChildOp)
      bool alertMakeChild();

      // Overrides the search function of branching in order to send the minion processors
      // into the waiting for work loop.
      virtual double search();
  }
}

#endif
#endif
    
