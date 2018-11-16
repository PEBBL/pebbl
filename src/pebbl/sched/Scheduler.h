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
 * \file Scheduler.h
 * \author William Hart
 *
 * This code currently makes the following assumptions:
 *   a) the message tags are numbered consecutively beginning with one
 *   b) the priorities from from 0 to NumQueueLevels-1
 */

#ifndef pebbl_Scheduler_h
#define pebbl_Scheduler_h

#include <pebbl_config.h>

//  JE allows MPI compiles with partially developed CMake
#if     defined(HAVE_MPI) && !defined(ACRO_HAVE_MPI)
#define ACRO_HAVE_MPI
#endif


#ifdef ACRO_HAVE_MPI
#include <mpi.h>
#endif
#include <pebbl/utilib/LinkedList.h>
#include <pebbl/utilib/QueueList.h>
#include <pebbl/utilib/BasicArray.h>
#include <pebbl/sched/ThreadObj.h>
#include <pebbl/sched/ThreadQueue.h>

namespace pebbl {

using utilib::LinkedList;
using utilib::QueueList;
using utilib::BasicArray;

class Scheduler : public CommonIO
{
public:

  Scheduler(const int numTags=0);
  virtual ~Scheduler();

  void reset();

  int add(ThreadQueue* queue);
  int execute();
  void dump();

  void insert(ThreadObj* thread, int group=-1);

  void insert(ThreadObj* thread, int group, int debug_, const char* name=NULL);

  void check_waiting_threads();

  static int termination_flag;

  int wtime_flag;		// If true, then compute (and schedule with)
				// wall-clock time.  Otherwise, use CPU time

  //
  // Timing information
  //	total		Total time spent running the scheduler
  //	partial		Time spent idling in the scheduler
  //	runtime		Time spent running the threads
  //
  void timing(double& total, double& partial, double& runtime)
	{total=total_time; partial=partial_time; runtime=run_time;}


protected:

	///
  BasicArray<LinkedList<ThreadObj*>*> interrupts;

	/// The list of dead/finished threads
  LinkedList<ThreadObj*> deadList;

	/// A queue containing the blocked ThreadObj's
  QueueList<ThreadObj*> algBlockedList;

	/// The array of thread groups
  BasicArray<ThreadQueue*> threadGroup;

#ifdef ACRO_HAVE_MPI
	///
  int num_requests;

	///
  BasicArray<MPI_Request> requests;

	///
  BasicArray<int> requests_ndx;

	///
  BasicArray<int> testsome_ndx;

	///
  BasicArray<MPI_Status> testsome_status;
#endif

  void insert(ThreadObj::RunStatus, ThreadObj*);

  void clearQueues();

  double total_time, partial_time, run_time;

  int state_changed;
  
  double getTime();

};

} // namespace pebbl

#endif
