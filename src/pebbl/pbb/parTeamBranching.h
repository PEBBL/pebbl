// parTeamBranching.h
//
// Two-level parallel search and computation class code for PEBBL.
// Designed to permit parallelism within subproblems and in the search.
//
// Will Loughlin and Rohith Rokkam, later work by Jonathan Eckstein

#include <cstdlib>
#include <pebbl/pbb/teamBranching.h>
 
#ifndef pebbl_parTeamBranching_h
#define pebbl_parTeamBranching_h


#ifdef ACRO_HAVE_MPI

namespace pebbl {

// Enum for the possible parallelism modes PEBBL can run in
enum PEBBL_mode {serialMode, parallelMode, teamMode, parallelTeamMode};


// Class that will implement methods used by the application for
// parallel search with computation
  class parallelTeamBranching : virtual public teamBranching,
                                virtual public parallelBranching
  {
    public:
      
      bool setup(int& argc, char**& argv){
        return parallelBranching::setup(argc, argv);
      }

      // Splits a world comm into team comms and a search comm according to the parameters passed to pebbl
      // returns 0 on success, or an errorcode if an mpi call fails
      void splitCommunicator(mpiComm worldComm, int teamSize, 
                             int clusterSize, int hubsDontWorkSize, 
                             mpiComm *search, mpiComm *team);

      // Overrides the search function of parBranching
      virtual double search(); 

      // Override the minion dispatch function of teamBranching to recognize the
      // "start of ramp-up" and "end of ramp-up" commands

      bool minionDispatch(parallelOp opCode);

      // Tweak the ramp-up search routine to issue those commands

      void rampUpInternalSetup()   { alertTeam(rampUpStartOp); };
      void rampUpInternalCleanUp() { alertTeam(rampUpEndOp);   };

      // Override the setupSeachComm method called in parallelBranching::setup()
      // to now split the communicators and initialize the searchComm and the boundComm
      void setupSearchComm(){
        // Free communicators we make in case setup is called more than once
        searchComm.free();
        teamComm.free();
        splitCommunicator(passedComm, teamSize, clusterSize, hubsDontWorkSize, 
                          &searchComm, &teamComm);
      }

      // Minions only do a serial-style reset, while heads do a parallel reset
      // Will and Rohith set this up, hopefully it is OK
      
      void reset(bool VBFlag=true) {
        teamMessageCount = 0;
        if (iAmHead()) {
          parallelBranching::reset(VBFlag);
        }
        else {
          branching::reset(VBFlag);
        }
      }

      // Override the solutionToFile method as only search processors participate
      virtual void solutionToFile() {
        if (iAmHead()) {
          parallelBranching::solutionToFile();
        }
      }

      // Override the printSPStatistics method as only search processors participate
      virtual void printSPStatistics() {
        if (iAmHead()) {
          parallelBranching::printSPStatistics();
        }
      }

      void printConfiguration(std::ostream& stream = ucout){
        parallelBranching::printConfiguration(stream);
        if (iDoSearchIO){
          CommonIO::end_tagging();
          stream << "Searching using teams of size: " << teamSize << ".\n\n";
          CommonIO::begin_tagging();
        }
      }

      
      // Disambiguate printing methods shared by parallelBranching and teamBranching
      virtual void printSolValue(std::ostream& stream){
        parallelBranching::printSolValue(stream);
      }

      virtual void printSolution(const char* header,
				      const char* footer,
				      std::ostream& outStream){
        parallelBranching::printSolution(header, footer, outStream);
      }

      parallelTeamBranching(MPI_Comm _comm) :
        parallelBranching(_comm)
      {
      }

      ~parallelTeamBranching(){
        // Free the communicators we create in split communicator
        searchComm.free();
        teamComm.free();
      }
  };

// RR: This method is "kludgey".
// We need to determine the mode of parallelism before we process the rest of
// the parameters, since that parameter processing requires the exception
// manager in UTILIB to be set up, which happens during instance.setup().
// So we have to scan through argv and check relevant flags manually to
// determine the mode of parallelism.
inline PEBBL_mode
parallel_exec_test(int argc, char **argv, int nproc)
{
  bool hadForceParallel = false;
  bool hadForceTeam = false;
  int teamSize = -1;

  for(int i=1; i<argc; i++) {
    if(strncmp(argv[i],"--help",6) == 0)
      // This will give help info for parameters in all modes.
      return parallelTeamMode;
    if(strncmp(argv[i],"--forceParallel",15) == 0)
      hadForceParallel = true;
    if(strncmp(argv[i],"--forceTeam",11) == 0)
      hadForceTeam = true;
    if(strncmp(argv[i],"--teamSize",10) == 0)
      // nb. If the string is invalid, strtol returns 0. Then we pick
      // parallelTeamMode. (if teamSize=0 caused a choice
      // of serialMode, say, then the parameter processing might give an
      // error about the presence of some non-serial parameter before it
      // gets a chance to complain about teamSize).
      teamSize = std::strtol(argv[i]+11, (char **)NULL, 10);
  }

  if(hadForceParallel && hadForceTeam) {
    return parallelTeamMode;
  } else if (hadForceParallel) {
    return parallelMode;
  } else if (hadForceTeam) {
    return teamMode;
  }

  // If we reach this point, no force options have been given.
  if(nproc == 1)
    return serialMode;
  if(teamSize == -1)
    return parallelMode;
  if(teamSize >= nproc)
    return teamMode;
  else
    return parallelTeamMode;
}

template <class B> bool runParallel(int argc,char** argv, MPI_Comm comm=MPI_COMM_WORLD)
{
  B instance(comm);
  utilib::exception_mngr::set_stack_trace(false);
  bool flag = instance.setup(argc,argv);
  if (flag)
    {
      utilib::exception_mngr::set_stack_trace(true); 
      instance.reset();
      instance.printConfiguration();
      instance.solve();
    }
  return flag;
}

template<class B, class PB, class TB, class PTB>
int driver(int argc, char **argv)
{
  bool flag = true;

  try {
    uMPI::init(&argc, &argv, MPI_COMM_WORLD);
    int nproc = uMPI::size;

    PEBBL_mode mode = parallel_exec_test(argc, argv, nproc);

    if(mode == serialMode) {
      flag = runSerial<B>(argc, argv);
      uMPI::done();
      return !flag;
    }

    CommonIO::begin();
    CommonIO::setIOFlush(1);

    switch(mode) {
      case parallelMode:
        runParallel<PB>(argc, argv);
        break;
      case teamMode:
        runParallel<TB>(argc, argv);
        break;
      case parallelTeamMode:
        runParallel<PTB>(argc, argv);
        break;
      case serialMode:
        // Already handled above. This case will never be reached.
        break;
    }

    CommonIO::end();
    uMPI::done();
  }
  UTILIB_STD_CATCH(CommonIO::end(); uMPI::done();)
  return !flag;
}

}  // namespace pebbl

#endif
#endif

// Need to define the driver in the case ACRO_HAVE_MPI is not defined.
