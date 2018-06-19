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

#ifndef utilib_mpiComm_H
#define utilib_mpiComm_H

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

//
// To compile code for logging of all messages.
//

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
  MPI_Comm myComm()   { return comm; };

  /// Rank within current communicator
  int myRank() { return rank; };

  /// Size of current communicator
  int mySize() { return size; };

  /// Rank of of IO-enabled processor in current communicator
  int myIoProc() { return ioProc; };

  /// Returns \c TRUE if current rank can do IO
  bool iDoIO() { return ioFlag; };

  /// The error code from the previous MPI call.
  int mpiErrorCode() { return errorCode; };

  /// Constructor taking an MPI communicator argument
  mpiComm(MPI_Comm comm_=MPI_COMM_WORLD);

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

  /// Computes a parallel sum of integers.
  int sum(int value,int root = ioProc)
    {
        int result = 0;
        errorCode = MPI_Reduce(&value,&result,1,MPI_INT,MPI_SUM,root,comm);
        if (errorCode)
            EXCEPTION_MNGR(std::runtime_error, "MPI_Reduce failed, code " << errorCode);
        return result;
    }

  /// Computes a parallel sum of doubles.
  double sum(double value,int root = ioProc)
    {
        double result = 0;
        errorCode = MPI_Reduce(&value,&result,1,MPI_DOUBLE,MPI_SUM,root, comm);
        if (errorCode)
            EXCEPTION_MNGR(std::runtime_error, "MPI_Reduce failed, code " << errorCode);
        return result;
    }

  /// Computes a parallel max of integers.
  int max(int value,int root = ioProc)
    {
        int result = 0;
        errorCode = MPI_Reduce(&value,&result,1,MPI_INT,MPI_MAX,root,comm);
        if (errorCode)
            EXCEPTION_MNGR(std::runtime_error, "MPI_Reduce failed, code " << errorCode);
        return result;
        }

  /// Computes a parallel max of doubles.
  double max(double value,int root = ioProc)
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
        requestFree(&request);
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

  // /// Get the count from a data type.
  // int getCount(MPI_Status* status,MPI_Datatype datatype)
  //   {
  //   int count;
  //   errorCode = MPI_Get_count(status,datatype,&count);
  //   if (errorCode)
  //      EXCEPTION_MNGR(std::runtime_error, "MPI_Get_count failed, code "
  //                               << errorCode);
  //   return count;
  //   }

//   /// Test a request, returning the result in argument \a flag.
//   void test(MPI_Request* request,int* flag,MPI_Status* status)
//     {
//     errorCode = MPI_Test(request,flag,status);
//     if (errorCode)
//        EXCEPTION_MNGR(std::runtime_error, "MPI_Test failed, code "
//                                 << errorCode);
// #ifdef UTILIB_HAVE_MPI
//     if (*flag)
//        LOG_RECV(status);
// #endif
//     }

  // /// Test a send request, returning the result in argument \a flag.
  // /// Similar to test, but does not attempt logging.
  // void testSend(MPI_Request* request,int* flag,MPI_Status* status)
  //   {
  //   errorCode = MPI_Test(request,flag,status);
  //   if (errorCode)
  //      EXCEPTION_MNGR(std::runtime_error, "MPI_Test failed, code "
  //                               << errorCode);
  //   }

  // /// Test a request, returning the result.
  // int test(MPI_Request* request,MPI_Status* status)
  //   {
  //   int flag;
  //   test(request,&flag,status);
  //   return flag;
  //   }

  // /// Test a send request, returning the result.  Similar to above,
  // /// but will not attempt logging.
  // int testSend(MPI_Request* request,MPI_Status* status)
  //   {
  //   int flag;
  //   testSend(request,&flag,status);
  //   return flag;
  //   }

  // /// Test a request, returning the result and ignoring the status.
  // int test(MPI_Request* request)
  //   {
  //   MPI_Status status;
  //   return test(request,&status);
  //   }

  // /// Test a send request, returning the result and ignoring the
  // /// status.  Similar to previous call, but will not attempt logging.
  // int testSend(MPI_Request* request)
  //   {
  //   MPI_Status status;
  //   return testSend(request,&status);
  //   }

//   /// Call Testsome.
//   int testsome(int incount, MPI_Request* request_array,
//         int& outcount, int* array_of_indeces, MPI_Status* status_array)
//     {
//     errorCode = MPI_Testsome(incount,request_array,&outcount,
//                 array_of_indeces,status_array);
//     if (errorCode != MPI_SUCCESS)
//        EXCEPTION_MNGR(std::runtime_error, "MPI_Testsome failed, code "
//                                 << errorCode);
// #ifdef UTILIB_HAVE_MPI
//     for (int i=0; i<outcount; i++)
//       LOG_RECV(status_array+i);
// #endif
//     return (outcount > 0);
//     }

  // /// Cancel a request.
  // void cancel(MPI_Request* request)
  //   {
  //   errorCode = MPI_Cancel(request);
  //   if (errorCode)
  //      EXCEPTION_MNGR(std::runtime_error, "MPI_Cancel failed, code "
  //                               << errorCode);
  //       }

  // /// Wait on a request.
  // void wait(MPI_Request* request,MPI_Status* status,bool killing=false)
  //   {
  //   errorCode = MPI_Wait(request,status);
  //   if (errorCode)
  //   EXCEPTION_MNGR(std::runtime_error, 
  //              "MPI_Wait failed, code " << errorCode);
  //   if (!killing)
  //     {
  //       LOG_RECV(status);
  //     }
  //   }

  // /// Cancel send requests.
  // void killSendRequest(MPI_Request* request);

  // /// Cancel receive requests.
  // void killRecvRequest(MPI_Request* request);

#endif

};

}

#endif
