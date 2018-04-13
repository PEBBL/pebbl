/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

/**
 * \file workerThread.h
 * \author Jonathan Eckstein
 *
 * Thread object used by the worker in PEBBL
 */

#ifndef pebbl_workerThread_h
#define pebbl_workerThread_h

#include <pebbl_config.h>
#include <pebbl/pbb/parBranching.h>
#include <pebbl/pbb/parBranchThreads.h>

#ifdef ACRO_HAVE_MPI


namespace pebbl {


class workerThreadObj : public parBranchSelfAdjThread
{
public:

  workerThreadObj(parallelBranching* global_);

  RunStatus run(double* controlParam);

  ThreadState state()
    {
      if (ready())
	return ThreadReady;
      else
	return ThreadBlocked;
    };

  bool emptyPool() 
    { 
      return (workerPool == 0) || (workerPool->size() == 0); 
    };

  bool ready();
  bool blocked() { return !ready(); };

private:  

  branchPool<parallelBranchSub,parLoadObject> *workerPool;

};

} // namespace pebbl

#endif

#endif
