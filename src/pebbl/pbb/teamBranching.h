// teamBranching.h
//
// Teams-only parallel computation class code for PEBBL.
// Used when the only parallelism is within subproblems, and also as a base
// class for two-way parallelism
//
// Will Loughlin and Rohith Rokkam; later modifications by Jonathan Eckstein


#include <pebbl/comm/mpiComm.h>
 
#ifndef pebbl_teamBranching_h
#define pebbl_teamBranching_h


#ifdef ACRO_HAVE_MPI

namespace pebbl {

// Class that will implement methods used by the application for
// parallel computation
  class teamBranching : virtual public branching,
                        virtual public parallelPebblParams
  {
    public: 

      // Communicator used by a bounding group for computation
      mpiComm teamComm; 

      // Counter for MPI messages sent in team Communication
      int teamMessageCount;

      enum parallelOp { boundOp, 
                        separateOp, 
                        makeChildOp, 
                        exitOp,
                        rampUpStartOp,    // The last two are used only in the 
                        rampUpEndOp };    // derived paralleTeamBranching class

      /// Initialize base classes and reset the state of the solver
      virtual void reset()
      {
         teamMessageCount = 0;
      };

      // Sets teamComm and calls teamOrganize
      void setTeam(mpiComm comm) {
        teamComm = comm;
        teamOrganize();
      }

      // If this processor is the head of a team, alertTeam will send an MPI broadcast
      // to all processors in the team containing the op that needs to be performed.
      // Does nothing if the processor is not a head.
      bool alertTeam(parallelOp op) {
        if(!iAmHead())
          return false;
        DEBUGPR(1,ucout << "Sending team opCode " << op << std::endl);
        teamComm.broadcast(&op, 1, MPI_INT, teamComm.myRank());
        return true;
      }

      // Method called by minion processors in the search method. Processors in await work
      // will wait to be notified by the head before entering the correct parallel method.
      void awaitWork();

      // Internal routine used by awaitWork.  Returns true when there's nothing
      // more to do
      virtual bool minionDispatch(parallelOp opCode);

      // Pure virtual method used to by the application to initialize a team of processors.
      // Called from setTeam().
      virtual void teamOrganize() = 0;

      // Pure virtual method. Called from awaitWork() when a minion needs to work on bounding
      virtual void minionBound() 
      {
        unimplementedWarning("minionBound", "alertBound"); 
      };

      // Pure virtual method. Called from awaitWork() when a minion needs to work on separating
      virtual void minionSplit()
      {
        unimplementedWarning("minionSplit", "alertSeparate"); 
      };

      // Pure virtual method. Called from awaitWork() when a minion needs to work on making children
      virtual void minionMakeChild()
      {
        unimplementedWarning("minionMakeChild", "alertMakeChild"); 
      };

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
      virtual bool iAmHead() {
        return teamComm.myRank() == getHeadRank();
      }

      // True if this processor is a minion in a team
      virtual bool iAmMinion() {
        return !iAmHead();
      }

      // Returns the rank of this team's head
      // Currently this is always 0
      int getHeadRank() { return 0; }

      // Overrides branching::setup to only print from the head processor
      bool setup(int& argc, char**& argv);

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

      void printConfiguration(std::ostream& stream = ucout){
        if(iAmHead()){
          CommonIO::end_tagging();
          stream << "Searching with a team of size " << teamComm.mySize() << "\n\n";
          CommonIO::begin_tagging();
        }
      }

      // Override the following printout functions to prevent minion processors
      // from attempting to print at the end of the search
      virtual void printSolValue(std::ostream& stream){
        if(iAmHead()){
          branching::printSolValue(stream);
        }
      }

      virtual void printSolution(const char* header,
				      const char* footer,
				      std::ostream& outStream){
        if(iAmHead()){
          branching::printSolution(header, footer, outStream);
        }
      }

      virtual void printAllStatistics(std::ostream& stream = std::cout){
        if(iAmHead()){
          branching::printAllStatistics(stream);
        }
      }

      // Overrides the search function of branching in order to send the minion processors
      // into the waiting for work loop.
      virtual double search();
  
      // Solution to file is the same as in serial, but only on the head processor
      void solutionToFile()
      {
        if (iAmHead())
          branching::solutionToFile();
      }

  protected:

      void unimplementedWarning(const string& methodName, 
                                const string& probableCaller)
      {
        string warningMessage = "WARNING:  The " + methodName + " method was called "
                                + " for an application that does not implement it.\n"
                                + "If you call " + probableCaller 
                                + ", then you should implement " + methodName + ".";
        ucout << warningMessage;
        ucerr << warningMessage;
      }
  };
}

#endif
#endif
    
