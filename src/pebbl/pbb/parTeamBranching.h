// parTeamBranching.cpp
//
// Parallel search and computation class code for PEBBL.
//
// Will Loughlin and Rohith Rokkam

#include <cstdlib>

 
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
}


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
      return PEBBL_mode.parallelTeamMode;
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
    return PEBBL_mode.parallelTeamMode;
  } else if (hadForceParallel) {
    return PEBBL_mode.parallelMode;
  } else if (hadForceTeam) {
    return PEBBL_mode.teamMode;
  }

  // If we reach this point, no force options have been given.
  if(nproc == 1)
    return PEBBL_mode.serialMode;
  if(teamSize == -1)
    return PEBBL_mode.parallelMode;
  if(teamSize >= nproc)
    return PEBBL_mode.teamMode;
  else
    return PEBBL_mode.parallelTeamMode;
}


template<class B, class PB, class TB, class PTB>
int driver(int argc, char **argv, MPI_Comm comm)
{
  int nproc;
  MPI_Comm_size(comm, &nproc); // swap this out for uMPI::init.
  PEBBL_mode mode = parallel_exec_test(argc, argv, nproc);
  std::cout << mode; // debugging
//  switch(mode)
//  {
//    case serialMode:
//      break;
//    case parallelMode:
//      break;
//    case teamMode:
//      break;
//    case parallelTeamMode:
//      break;
//  }
  //
}


#endif
#endif

