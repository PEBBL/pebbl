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
 * \file logEvent.h
 *
 * Definess the utilib::logEvent class.
 *
 * \author Jonathan Eckstein
 */

#ifndef utilib_logEvent_H
#define utilib_logEvent_H

#include <pebbl_config.h>


//  JE allows MPI compiles with partially developed CMake
#if     defined(HAVE_MPI) && !defined(UTILIB_HAVE_MPI)
#define UTILIB_HAVE_MPI
#endif

#if defined(DOXYGEN) || (defined(UTILIB_HAVE_MPI) && defined(UTILIB_VALIDATING) && defined(UTILIB_HAVE_MPE)) 

#include <values.h>
#include <pebbl/utilib/mpiUtil.h>
#include <mpe.h>
#include <pebbl/utilib/ParameterSet.h>

namespace utilib {

/**
 * Helper class for keeping track of states,
 */

  class logStateObject 
  {
  public:

    int start;
    int end;

  logStateObject() :
    start(0),
      end(0)
	{ };

    void define(const char* description,const char* color)
    {
      MPE_Log_get_state_eventIDs(&start,&end);
      if (uMPI::rank == 0)
	MPE_Describe_state(start,end,description,color);
    }

  };


/**
 * Routines for doing event logging/tracing of parallel codes using MPE.
 */
class logEvent : virtual public ParameterSet
{
public:

  /// Constructor (defined in .cpp file)
  logEvent();

  /// Initializes the MPI event logging capability.
  static void init()
   {
     uMPI::barrier();
     MPE_Init_log(); 
   };

  static void defineEvent(int*        eNumAddress,
			  const char* description,
			  const char* color)
  {
    MPE_Log_get_solo_eventID(eNumAddress);
    MPE_Describe_event(*eNumAddress,description,color);
  };

  /// Log event \c eventNumber.
  static void event(int eventNumber) 
  { 
    MPE_Log_event(eventNumber,0,(char *) ""); 
  };

  static void finish()
  { 
    MPE_Finish_log("event");
  };

  
protected:

  /// The event logging level

  int eventLog;

};


/// Code that is executed if MPE logging is in use.
#define UTILIB_IF_LOGGING_COMPILED(arbitraryCode) arbitraryCode

/// Executes \p action if the event log is greater or equal to \p level.
#define UTILIB_IF_LOGGING_EVENTS(level,action) \
   if (eventLog >= level) action

/// Executes \p action if the event log is greater or equal to \p
/// level.  Finds event log level through pointer p
#define UTILIB_IF_LOGGING_EVENTSX(p,level,action)	\
   if (p->eventLog >= level) action

/**
 * Wraps a call to \c log_<how> using \p state if the logging level is 
 * greater or equal to \p level. */
#define UTILIB_LOG_EVENT(level,startOrEnd,state) \
   UTILIB_IF_LOGGING_EVENTS(level,logEvent::event(state.startOrEnd);)

#define UTILIB_LOG_POINT_EVENT(level,state) \
   UTILIB_IF_LOGGING_EVENTS(level,logEvent::event(state);)

/**
 * Similar, finds the event log level through the pointer p
 */
#define UTILIB_LOG_EVENTX(p,level,startOrEnd,state)			\
  UTILIB_IF_LOGGING_EVENTSX(p,level,p->event(state.startOrEnd);)

#define UTILIB_LOG_POINT_EVENTX(p,level,state)			\
  UTILIB_IF_LOGGING_EVENTSX(p,level,p->event(state);)

/// Wraps log of <startOrEnd> of \p state if \p cond is satisfied.
#define UTILIB_LOG_EVENT_CONDITIONAL(level,cond,startOrEnd,state) \
   UTILIB_IF_LOGGING_EVENTS(level,if (cond) logEvent::event(state.startOrEnd);)

#define UTILIB_LOG_EVENT_CONDITIONALX(p,level,cond,startOrEnd,state) \
   UTILIB_IF_LOGGING_EVENTSX(p,level,if (cond) p->event(state.startOrEnd);)


  // Create "point" versions of the last two above later if needed.

/// Indicates that the logging macros are not empty.
#define EVENT_LOGGING_PRESENT 1

} // namespace utilib

#else


#define UTILIB_IF_LOGGING_COMPILED(arbitraryCode)                    /* Nothing */
#define UTILIB_IF_LOGGING_EVENTS(level,action)                       /* Nothing */
#define UTILIB_IF_LOGGING_EVENTSX(p,level,action)                    /* Nothing */
#define UTILIB_LOG_EVENT(level,startOrEnd,state)                     /* Nothing */
#define UTILIB_LOG_POINT_EVENT(level,state)                          /* Nothing */
#define UTILIB_LOG_EVENTX(p,level,startOrEnd,state)                  /* Nothing */
#define UTILIB_LOG_POINT_EVENTX(p,level,state)                       /* Nothing */
#define UTILIB_LOG_EVENT_CONDITIONAL(level,cond,startOrEnd,state)    /* Nothing */
#define UTILIB_LOG_EVENT_CONDITIONALX(p,level,cond,startOrEnd,state) /* Nothing */
#undef  EVENT_LOGGING_PRESENT


#endif


#endif
