/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file mpiUtil.cpp
 *
 * Utility class to make using MPI a little easier.
 *
 * \author Jonathan Eckstein
 */


#include <pebbl/comm/mpiComm.h>

#ifdef ACRO_HAVE_MPI


#include <unistd.h>
#include <pebbl/utilib/std_headers.h>
#include <pebbl/utilib/CommonIO.h>


#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif

namespace pebbl {


mpiComm::mpiComm(MPI_Comm comm_) :
comm(comm_)
{
  errorCode = MPI_Comm_rank(comm,&rank);
  if (errorCode) 
     ucerr << "MPI_Comm_rank failed, code " << errorCode << endl;
  errorCode = MPI_Comm_size(comm,&size);
  if (errorCode) 
     ucerr << "MPI_Comm_size failed, code " << errorCode << endl;

// JE -- this stuff (I think done by Mike Eldred) is probably archaic.  
// Just assume that every MPI process can write to stdout and
// #ifdef out all the manipulations of who can the IO processor

#ifdef MPI_STDOUT_RESTRICTED

  // MSE: modifications required for the case where the incoming comm is not
  // MPI_COMM_WORLD since MPI_IO is not guaranteed to be an attribute of comm.
  // If comm==MPI_COMM_WORLD or if MPI_IO==rank of the calling process, then
  // MPI_IO rank in MPI_COMM_WORLD can be mapped to rank in comm.  Otherwise, 
  // fall back on ioProc=0 default.
  int flag, result;
  int* mpiIOP;
  errorCode = MPI_Comm_get_attr(MPI_COMM_WORLD,MPI_IO,&mpiIOP,&flag);
  if (errorCode || !flag)
     ucerr << "MPI_Comm_get_attr(MPI_IO) failed, code " << errorCode << endl;
  MPI_Comm_compare(comm, MPI_COMM_WORLD, &result);
  if (result==MPI_IDENT || result==MPI_CONGRUENT) // no mapping of MPI_IO reqd.
    ioProc = *mpiIOP;
  else { // MPI_IO can only be mapped to comm in special cases
    int world_rank;
    errorCode = MPI_Comm_rank(MPI_COMM_WORLD,&world_rank);
    if (errorCode) 
       ucerr << "MPI_Comm_rank failed, code " << errorCode << endl;
    if (*mpiIOP == world_rank) // MPI_IO processor is this processor
      ioProc = rank; // MPI_IO in MPI_COMM_WORLD maps to rank in comm
    else
      ioProc = size; // no mapping of MPI_IO to comm is possible.  Assign size
                     // so that reduce works properly.
  }
  int elected;
  reduce(&ioProc,&elected,1,MPI_INT,MPI_MIN,0);
  ioProc = elected;
  broadcast(&ioProc,1,MPI_INT,0);
  if ((ioProc < 0) || (ioProc >= size))
    ioProc = 0;

#else

// Default is that the first MPI process in a communicator is its IO process
ioProc = 0;

#endif

  ioFlag = (rank == ioProc);

};


} // namespace pebbl 


#endif
