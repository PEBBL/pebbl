// parTeamBranching.cpp
//
// Parallel search and computation class code for PEBBL.
//
// Will Loughlin and Rohith Rokkam

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

template <class B> bool runParallel(int argc,char** argv, MPI_Comm comm)
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
int driver(int argc, char **argv, MPI_Comm comm)
{
  bool flag = true;

  try {
    uMPI::init(&argc, &argv, comm);
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
        runParallel<PB>(argc, argv, comm);
        break;
      case teamMode:
        runParallel<TB>(argc, argv, comm);
        break;
      case parallelTeamMode:
        runParallel<PTB>(argc, argv, comm);
        break;
      default:
        // scream
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
