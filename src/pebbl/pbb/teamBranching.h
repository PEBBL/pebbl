// teamBranching.h
//
// Parallel computation class code for PEBBL.
//
// Will Loughlin and Rohith Rokkam


#include <pebbl/comm/mpiComm.h>

 
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

      enum parallelOp { boundOp, separateOp, makeChildOp, exitOp };

      // Sets teamComm and calls teamOrganize
      void setTeam(mpiComm comm) {
        teamComm = comm;
        teamOrganize();
      }

      // If this processor is the head of a team, alertTeam will send an MPI broadcast
      // to all processors in the team containing the op that needs to be performed.
      // Does nothing if the processor is not a head.
      bool alertTeam(parallelOp op) {
        if(!iAmHead()){
          return false;
        }
        teamComm.broadcast(&op, 1, MPI_INT, teamComm.myRank());
        return true;
      }

      // Method called by minion processors in the search method. Processors in await work
      // will wait to be notified by the head before entering the correct parallel method.
      void awaitWork();

      // Pure virtual method used to by the application to initialize a team of processors.
      // Called from setTeam().
      virtual void teamOrganize() = 0;

      // Pure virtual method. Called from awaitWork() when a minion needs to work on bounding
      virtual void minionBound() = 0;

      // Pure virtual method. Called from awaitWork() when a minion needs to work on separating
      virtual void minionSplit() = 0;

      // Pure virtual method. Called from awaitWork() when a minion needs to work on making children
      virtual void minionMakeChild() = 0;

    public:

      // Constructors for teamBranching
      teamBranching() :
        teamComm()
      {
        teamMessageCount = 0;
      }
      
      teamBranching(MPI_Comm _comm) :
        teamComm(_comm) 
      {
        teamMessageCount = 0;
      }

      // True if this processor is the head of a team
      bool iAmHead() {
        return teamComm.myRank() == getHeadRank();
      }

      // True if this processor is a minion in a team
      bool iAmMinion() {
        return !iAmHead();
      }

      // Returns the rank of this team's head
      // Currently this is always 0
      int getHeadRank() {
        return 0;
      }

      // Sets teamComm to the passed in commuicator before calling the setup() for branching
      bool setup(int& argc, char**& argv, mpiComm teamComm = MPI_COMM_WORLD);

      // Wrapper for alertTeam(boundOp)
      bool alertBound() {
        return alertTeam(boundOp);
      }

      // Wrapper for alertTeam(separateOp)
      bool alertSeparate() {
        return alertTeam(separateOp);
      }

      // Wrapper for alertTeam(makeChildOp)
      bool alertMakeChild() {
        return alertTeam(makeChildOp);
      }

      // Wrapper for alertTeam(exitOp)
      bool alertExit() {
        return alertTeam(exitOp);
      }

      // Overrides the search function of branching in order to send the minion processors
      // into the waiting for work loop.
      virtual double search();
  };
}

#endif
#endif
    
