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

//
// A simple test for CommonIO
//
#include <utilib_config.h>
#include <utilib/_math.h>
#ifndef UTILIB_DISABLE_COMMONIO

#ifndef UTILIB_VALIDATING
#define UTILIB_VALIDATING
#endif

#include <utilib/CommonIO.h>
#include <utilib/mpiUtil.h>
#ifdef UTILIB_HAVE_MPI
#include <mpi.h>
#endif

using namespace std;
using namespace utilib;

int test_commonio_debug(int argc,char** argv)
{
#ifdef UTILIB_HAVE_MPI
uMPI::init(&argc,&argv,MPI_COMM_WORLD);
#else
// Avoid compiler warnings
argc=0;
argv=0;
#endif

/********************* Comment out ************
#if 1
#ifdef UTILIB_HAVE_MPI
int rank,size;
MPI_Comm_rank(MPI_COMM_WORLD,&rank);
MPI_Comm_size(MPI_COMM_WORLD,&size);

cout << "Basic Data: " << rank << " " << size << endl;
cout << "uMPI Data:  " << uMPI::rank << " " << uMPI::size << " " << uMPI::running() << endl;
int foo=-1;
cin >> foo;
cout << "FOO=" << foo << endl;
#endif
#endif
******************** End comment out ************/

   CommonIO::begin();
   ucout << "First line of text" << std::endl;
   ucout << Flush;
   CommonIO::end();

   CommonIO::begin();
   CommonIO::begin_tagging();
   ucout << "Second line of text" << std::endl;
   ucout << Flush;
   CommonIO::end_tagging();
   CommonIO::end();

   CommonIO::begin();
   CommonIO::begin_tagging();
   ucout << "Third line of text" << std::endl;
   ucout << Flush;
   double x = 123.4567;
   ucout << pscientific(x) << std::endl;
   for(int i=1; i<=4; i++)
     {
       ucout.precision(i);

       cout << utilib::CommonIO::MapCout->precision() << " " << cout.precision() << " " << (&ucout)->precision() << std::endl;
       ucout << "UCOUT: " << pscientific(x) << std::endl << Flush;
       ucout << "UCOUT: " << pscientific(x) << std::endl << Flush;
     }
   for(int i=1; i<=4; i++) {
       cout.precision(i);
       cout <<  "COUT:  " << pscientific(x) << std::endl << flush;
     }

   CommonIO::end_tagging();
   CommonIO::end();

#ifdef UTILIB_HAVE_MPI
uMPI::done();
#endif
return 0;
}

#else
int test_commonio_debug(int,char**) {return 0;}
#endif
