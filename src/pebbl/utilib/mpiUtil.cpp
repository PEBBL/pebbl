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

#include <pebbl_config.h>
#include <pebbl/utilib/mpiUtil.h>

#ifdef UTILIB_HAVE_MPI

#include <unistd.h>
#include <pebbl/utilib/std_headers.h>
#include <pebbl/utilib/CommonIO.h>


#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif

namespace utilib {

MPI_Comm uMPI::comm = MPI_COMM_WORLD;
int uMPI::rank   = -1;
int uMPI::size   = 1;
int uMPI::ioProc = 0;
int uMPI::iDoIO  = 1;

MPI_Comm uMPI::boundComm = MPI_COMM_NULL;
int uMPI::boundSize = 1;
int uMPI::boundRank = -1;
bool uMPI::isHead = false;

int uMPI::errorCode = 0;

// RR: This code seems more and more like a hack. Having to make assumptions about the
// clustering.cpp code is not great, and so is the need to parse argv for options
// that will tell us the information needed to figure out whether each hub will work.
// Much of this replicates work done in the clustering code.
// 
// If possible, it would be nice to merge / salvage portions of this in the clustering
// code. Maybe separate cluster objects for each bounding group, or modifying the current
// cluster object to include information about the bounding groups of each of its workers.
//
// Maybe pending the changes to mpiComm stuff.
void uMPI::splitCommunicator(MPI_Comm comm_, int boundingGroupSize, 
			     MPI_Comm *headCommunicator, 
			     MPI_Comm *boundingCommunicator
			     /*, int hubsDontWorkSize, int clusterSize */) 
{
  //duplicate(comm, headCommunicator);
  //return;

  int hubsDontWorkSize = 1000; // these two are for testing for now.
  int clusterSize = 64; // Default in parPebblParams

  int worldRank;
  int worldSize;

  MPI_Comm_rank(comm_, &worldRank);
  MPI_Comm_size(comm_, &worldSize);

  bool hubsWork = hubsDontWorkSize > clusterSize;
  int fullClusterSize = clusterSize * boundingGroupSize - (1 - hubsWork) * (boundingGroupSize - 1);

  int boundGroup;

  if (hubsWork) // would be nice to combine these two cases together.
  {
    isHead = worldRank % boundingGroupSize == 0;
    MPI_Comm_split(comm_, isHead, worldRank, headCommunicator);
    if (isHead)
    {
      MPI_Comm_rank(*headCommunicator, &rank);
      MPI_Comm_size(*headCommunicator, &size);
    }
    else
    {
      MPI_Comm_free(headCommunicator);
    }
    boundGroup = worldRank / boundingGroupSize;
    MPI_Comm_split(comm_, boundGroup, worldRank, boundingCommunicator);
    MPI_Comm_rank(*boundingCommunicator, &boundRank);
    MPI_Comm_size(*boundingCommunicator, &boundSize);
  }
  else
  {
    // this is tightly coupled with the decision making strategy
    // pebbl uses to decide how ranks are associated to workers/hubs.
    // Maybe this code shold be moved there, or scrapped to make
    // something that uses Jonathan's forthcoming MPI code
    int isHub = worldRank % fullClusterSize == 0;
    isHead = ((worldRank % fullClusterSize)) % boundingGroupSize == 1;
    int visibleToPebbl = isHub || isHead;
    MPI_Comm_split(comm_, visibleToPebbl, worldRank, headCommunicator);
    if (visibleToPebbl)
    {	
      MPI_Comm_rank(*headCommunicator, &rank);
      MPI_Comm_size(*headCommunicator, &size);
    }
    else
    {
      MPI_Comm_free(headCommunicator);	
    }		
    boundGroup = (worldRank - (worldRank / fullClusterSize + 1)) / boundingGroupSize;
    if (worldRank % fullClusterSize == 0 && (worldSize - worldRank > fullClusterSize || worldSize % fullClusterSize <= boundingGroupSize * (hubsDontWorkSize - 1)))
    { // we are a pure hub
      boundGroup = -1;
    }		
    MPI_Comm_split(comm_, boundGroup, worldRank, boundingCommunicator);
    if (boundGroup >= 0)
    {
      MPI_Comm_rank(*boundingCommunicator, &boundRank);
      MPI_Comm_size(*boundingCommunicator, &boundSize);
    }
    else
    {
      MPI_Comm_free(boundingCommunicator);
    }
  }
}


bool uMPI::init(int* argcP, char*** argvP, MPI_Comm comm_, 
		int boundingGroupSize)
{
  char *prev_dir;
  bool alreadyRunning = running();
  if (!alreadyRunning)
    {
      prev_dir = getcwd(0,256);
      errorCode = MPI_Init(argcP,argvP);
      if (errorCode)
	 ucerr << "MPI_Init failed, code " << errorCode << endl;
    }

  splitCommunicator(comm_, boundingGroupSize, &comm, &boundComm);
  
  if (comm == MPI_COMM_NULL)
  {
    return false;
  }
  
  init(comm);

  if (!alreadyRunning)
    {
      // Work around an mpich problem: force the current working directory
      // after mpi_init to be the same as before we called it.  This is only
      // applied in serial, since otherwise we assume that mpirun has
      // handled this...
      if (size == 1)
	 chdir(prev_dir);
      free(prev_dir);
      return true;
    }
}

void uMPI::init(MPI_Comm comm_)
{
  comm=comm_;
  errorCode = MPI_Comm_rank(comm,&rank);
  if (errorCode) 
     ucerr << "MPI_Comm_rank failed, code " << errorCode << endl;
  errorCode = MPI_Comm_size(comm,&size);
  if (errorCode) 
     ucerr << "MPI_Comm_size failed, code " << errorCode << endl;

  // MSE: modifications required for the case where the incoming comm is not
  // MPI_COMM_WORLD since MPI_IO is not guaranteed to be an attribute of comm.
  // If comm==MPI_COMM_WORLD or if MPI_IO==rank of the calling process, then
  // MPI_IO rank in MPI_COMM_WORLD can be mapped to rank in comm.  Otherwise, 
  // fall back on ioProc=0 default.
  int flag, result;
  int* mpiIOP;
  errorCode = MPI_Attr_get(MPI_COMM_WORLD,MPI_IO,&mpiIOP,&flag);
  if (errorCode || !flag)
     ucerr << "MPI_Attr_get(MPI_IO) failed, code " << errorCode << endl;
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
  iDoIO = (rank == ioProc);

  if (size > 1)
     CommonIO::begin_tagging();
};


void uMPI::done()
{
  if (size > 1) 
     CommonIO::end_tagging();
  MPI_Finalize();
};


int uMPI::sizeOf(MPI_Datatype t)
{
  MPI_Aint extent;
  errorCode = MPI_Type_extent(t,&extent);
  if (errorCode)
     ucerr << "MPI_Type_extent failed, code " << errorCode << endl;
  return extent;
}


void uMPI::killSendRequest(MPI_Request* request)
{
  if (*request != MPI_REQUEST_NULL)
    {
      MPI_Status trashStatus;
      if (!testSend(request,&trashStatus))
	{
          ucerr << "uMPI::killSendRequest: incomplete send." << endl;
	  cancel(request);
	  wait(request,&trashStatus,true /* suppress logging */);
	}
      *request = MPI_REQUEST_NULL;
    }
}


void uMPI::killRecvRequest(MPI_Request* request)
{
  if (*request != MPI_REQUEST_NULL)
    {
      MPI_Status trashStatus;
      // Use testSend here because it doesn't do logging; still works
      // for receives
      if (!testSend(request,&trashStatus))
	{
	  cancel(request);
	  wait(request,&trashStatus,true /* suppress logging */);
	}
      *request = MPI_REQUEST_NULL;
    }
};

#if defined(UTILIB_HAVE_MPI) && defined(UTILIB_VALIDATING) && defined(UTILIB_HAVE_MPE)


bool uMPI::messageLog = false;


void uMPI::logSend(int dest,int tag,int count,MPI_Datatype t)
{
  int typeSize;
  errorCode = MPI_Type_size(t,&typeSize);
  if (errorCode)
     ucerr << "MPI_Type-size returned error code " << errorCode << endl;
  MPE_Log_send(dest,tag,count*typeSize);
}


void uMPI::logRecv(MPI_Status* status)
{
  int src   = status->MPI_SOURCE;
  int tag   = status->MPI_TAG;
  int count = getCount(status,MPI_PACKED);
  if (src < 0)  // This usually means trying to log a send or kill
    ucerr << "WARNING: processor " << rank
	  << " trying to log bad receive src=" << src
	  << " tag=" << tag << " count=" << count << endl;
  else
    MPE_Log_receive(src,tag,count);
}


#endif


} // namespace utilib 


#else


int utilib::uMPI::rank = 0;

#endif
