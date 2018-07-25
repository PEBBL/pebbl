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

#include <pebbl_config.h>
#include <pebbl/example/parKnapsack.h>

using namespace pebbl;
using namespace std;




int main(int argc, char* argv[])
{
#ifdef ACRO_HAVE_MPI
  uMPI::init(&argc,&argv,MPI_COMM_WORLD);
  CommonIO::begin();
  CommonIO::setIOFlush(1);
  CommonIO::begin_tagging();
  int P = uMPI::size;
  if (uMPI::iDoIO)
  	cout << P << " processors.\n";
  MPI_Group bigGroup;
  MPI_Comm_group(MPI_COMM_WORLD,&bigGroup);
  int halfway = P/2;
  int range[1][3];
  range[0][0] = halfway;
  range[0][1] = P - 1;
  range[0][2] = 1;
  MPI_Group smallerGroup;
  MPI_Group_range_incl(bigGroup,1,range,&smallerGroup);
  MPI_Comm smallerComm;
  MPI_Comm_create(MPI_COMM_WORLD,smallerGroup,&smallerComm);
  if (uMPI::rank >= halfway)
  {
    parallelBinaryKnapsack instance(smallerComm);
  	utilib::exception_mngr::set_stack_trace(false);
  	bool flag = instance.setup(argc,argv);
  	utilib::exception_mngr::set_stack_trace(true);
  	if (flag)
  	{
  	  instance.reset();
	  instance.printConfiguration();
	  instance.solve();
    }
  }
  CommonIO::end();
  uMPI::done();
#endif
  return 0;
}


