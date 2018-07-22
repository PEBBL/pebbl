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
 * \file mpiComm.h
 *
 * Defines the mpiComm class, which provides wrappers to MPI and also 
 * stores a communicator handle
 *
 * \author Jonathan Eckstein
 */

#ifndef pebbl_mpiComm_H
#define pebbl_mpiComm_H

#include <pebbl_config.h>

//  JE allows MPI compiles with partially developed CMake
#if     defined(HAVE_MPI) && !defined(UTILIB_HAVE_MPI)
#define UTILIB_HAVE_MPI
#endif
 
#ifdef UTILIB_HAVE_MPI
#include <mpi.h>
#endif

#include <pebbl/utilib/_generic.h>
#include <pebbl/utilib/exception_mngr.h>
#include <pebbl/utilib/mpiUtil.h>

//
// To compile code for logging of all messages.
//

using namespace utilib;

namespace pebbl {

#if defined(UTILIB_HAVE_MPI)


class mpiComm 
{
protected:
    MPI_Comm comm;
    int      rank;
    int      size;
    int      ioProc;
    bool     ioFlag;
    int      errorCode;

public:
  /// Communicator handle
  MPI_Comm myComm() { return comm; };

  /// Rank within current communicator
  int myRank() { return rank; };

  /// Size of current communicator
  int mySize() { return size; };

  /// Rank of of IO-enabled processor in current communicator
  int myIoProc() { return ioProc; };

  /// Returns \c TRUE if current rank can do IO
  bool iDoIO() { return ioFlag; };

  /// Return a pointer to the current mpiComm object
  mpiComm* mpiCommObj() { return this; };

  /// The error code from the previous MPI call.
  int mpiErrorCode() { return errorCode; };

  /// Constructor taking an MPI communicator argument
  mpiComm(MPI_Comm comm_);  //=MPI_COMM_WORLD);

  /// Copy constructor
  mpiComm(mpiComm* ptr) :
    comm(ptr->comm),
    rank(ptr->rank),
    size(ptr->size),
    ioProc(ptr->ioProc),
    ioFlag(ptr->ioFlag)
    { };
  
  /// Executes a synchronous barrier command.
  void barrier()
    {
        errorCode = MPI_Barrier(comm);
        if (errorCode)
        EXCEPTION_MNGR(std::runtime_error, "MPI_Barrier failed, code " << errorCode);
    }

  /// Executes a parallel reduction.
  void reduce(void* sendbuf,void* recvbuf,int count,
              MPI_Datatype datatype,MPI_Op op,int root)
    {
        errorCode = MPI_Reduce(sendbuf,recvbuf,count,datatype,op,root,comm);
        if (errorCode)
            EXCEPTION_MNGR(std::runtime_error, "MPI_Reduce failed, code " << errorCode);
    }

  /// Computes a parallel sum of integers to ioProc
  int sumReduce(int value)
    {
        return sumReduce(value,ioProc);
    }

 /// Computes a parallel sum of integers to 'root'.
 int sumReduce(int value,int root)
    {
        int result = 0;
        errorCode = MPI_Reduce(&value,&result,1,MPI_INT,MPI_SUM,root,comm);
        if (errorCode)
            EXCEPTION_MNGR(std::runtime_error, 
                           "MPI_Reduce failed, code " << errorCode);
        return result;
    }


  /// Computes a parallel sum of doubles to ioproc
  double sumReduce(double value)
     {
        return sumReduce(value,ioProc);
     }

  /// Computes a parallel sum of doubles to 'root'.
  double sumReduce(double value,int root)
    {
        double result = 0;
        errorCode = MPI_Reduce(&value,&result,1,MPI_DOUBLE,MPI_SUM,root, comm);
        if (errorCode)
            EXCEPTION_MNGR(std::runtime_error, "MPI_Reduce failed, code " << errorCode);
        return result;
    }

  /// Computes a parallel max of integers to ioProc.
  int maxReduce(int value)
    {
        return maxReduce(value,ioProc);
    }

  /// Computes a parallel max of integers to 'root'
  int maxReduce(int value,int root)
    {
        int result = 0;
        errorCode = MPI_Reduce(&value,&result,1,MPI_INT,MPI_MAX,root,comm);
        if (errorCode)
            EXCEPTION_MNGR(std::runtime_error, "MPI_Reduce failed, code " << errorCode);
        return result;
        }

  /// Computes a parallel max of doubles to ioProc.
  double maxReduce(double value)
     {
        return maxReduce(value,ioProc);
     }

  /// Computes a parallel max of doubles to 'root'
  double maxReduce(double value,int root)
    {
        double result = 0;
        errorCode = MPI_Reduce(&value,&result,1,MPI_DOUBLE,MPI_MAX,root, comm);
        if (errorCode)
            EXCEPTION_MNGR(std::runtime_error, "MPI_Reduce failed, code " << errorCode);
        return result;
        }

  /// Perform a broadcast.
  void broadcast(void* buffer,int count,MPI_Datatype datatype,int root)
    {
        errorCode = MPI_Bcast(buffer,count,datatype,root,comm);
        if (errorCode)
            EXCEPTION_MNGR(std::runtime_error, "MPI_Broadcast failed, code " << errorCode);
    }

  /// Perform a reduction followed by a broadcast of the result.
  void reduceCast(void* sendbuf,void* recvbuf,int count,
                  MPI_Datatype datatype,MPI_Op op)
    {
      errorCode = MPI_Allreduce(sendbuf,recvbuf,count,datatype,op,comm);
      if (errorCode)
        EXCEPTION_MNGR(std::runtime_error,"MPI_Allreduce failed, code " << errorCode);
    }
      
  /// Perform MPI_Isend
  void isend(void* buffer,int count,MPI_Datatype datatype,int dest,int tag,
             MPI_Request* request)
    {
        LOG_SEND(dest,tag,count,datatype);
        errorCode = MPI_Isend(buffer,count,datatype,dest,tag,comm,request);
        if (errorCode)
            EXCEPTION_MNGR(std::runtime_error, "MPI_Isend failed, code " << errorCode);
    }

  /**
   * Perform an Isend without a user-supplied request object.
   * This method automatically frees the request object.
   */
  void isend(void* buffer,int count,MPI_Datatype datatype,int dest,int tag)
    {
        MPI_Request request;
        isend(buffer,count,datatype,dest,tag,&request);
        uMPI::requestFree(&request);
    }

  /// Perform a Send.
  void send(void* buffer,int count,MPI_Datatype datatype,int dest,int tag)
    {
        LOG_SEND(dest,tag,count,datatype);
        errorCode = MPI_Send(buffer,count,datatype,dest,tag,comm);
        if (errorCode)
            EXCEPTION_MNGR(std::runtime_error, "MPI_Send failed, code " << errorCode);
    }

  /// Perform a Ssend.
  void ssend(void* buffer,int count,MPI_Datatype datatype,int dest,int tag)
    {
        LOG_SEND(dest,tag,count,datatype);
        errorCode = MPI_Ssend(buffer,count,datatype,dest,tag,comm);
        if (errorCode)
            EXCEPTION_MNGR(std::runtime_error, "MPI_Ssend failed, code " << errorCode);
    }

  // /// Free a request object.
  // void requestFree(MPI_Request* request)
  //   {
  //       errorCode = MPI_Request_free(request);
  //       if (errorCode)
  //           EXCEPTION_MNGR(std::runtime_error, "MPI_Request_free failed, code "
  //                          << errorCode);
  //   }

  /// Perform an Issend.
  void issend(void* buffer,int count,MPI_Datatype datatype,int dest,int tag,
              MPI_Request* request)
    {
        LOG_SEND(dest,tag,count,datatype);
        errorCode = MPI_Issend(buffer,count,datatype,dest,tag,comm,request);
        if (errorCode)
            EXCEPTION_MNGR(std::runtime_error, "MPI_Issend failed, code " << errorCode);
        }

  /// Perform an Irecv.
  void irecv(void* buffer,int count,MPI_Datatype datatype,int source,int tag,
             MPI_Request* request)
    {
        errorCode = MPI_Irecv(buffer,count,datatype,source,tag,comm,request);
        if (errorCode)
            EXCEPTION_MNGR(std::runtime_error, "MPI_Irecv failed, code " << errorCode);
    }

  /// Perform a Recv.
  void recv(void* buffer,int count,MPI_Datatype datatype,int source,int tag,
            MPI_Status* status)
    {
        errorCode = MPI_Recv(buffer,count,datatype,source,tag,comm,status);
        if (errorCode)
            EXCEPTION_MNGR(std::runtime_error, "MPI_Recv failed, code " << errorCode);
        LOG_RECV(status);
    }

    // The remaining functions in mpiUtil didn't require knowledge of a communicator
    // so we can still use mpiUtil for them.

#endif

};

}

#endif
