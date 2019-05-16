// teamBranching.cpp
//
// Parallel computation class code for PEBBL.
//
// Will Loughlin and Rohith Rokkam



 
#ifndef pebbl_parBranching_h
#define pebbl_parBranching_h


#ifdef ACRO_HAVE_MPI

namespace pebbl {

// Class that will implement methods used by the application for
// parallel computation
  class teamBranching : public branching 
  {
    protected: 

      // Communicator used by a bounding group for computation
      mpiComm teamComm; 

      enum parallelOp = { boundOp, separateOp, makeChildOp, exitOp }

      // Sets teamComm and calls teamOrganize
      void setTeam(mpiComm comm);

      bool alertTeam(parallelOp op);

      void awaitWork();

      virtual void teamOrganize() = 0;

    public:

      bool iAmHead();

      bool iamMinion();

      int getHeadRank();

      bool setup(int& argc, char**& argv, mpiComm teamComm = MPI_COMM_WORLD);

      bool alertBound();

      bool alertSeparate();

      bool alertMakeChild();

      virtual double search();
  }
}

#endif ACRO_HAVE_MPI
      
    
