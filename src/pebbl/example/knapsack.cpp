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


// If not parallel, make a dummy definition of the parallelBranching class

#ifndef ACRO_HAVE_MPI
typedef void parallelBinaryKnapsack;
#endif

// These aren't defined yet
typedef parallelBinaryKnapsack parallelTeamBinaryKnapsack;
typedef parallelBinaryKnapsack teamBinaryKnapsack;

/*
int main(int argc, char* argv[])
{
  return driver<binaryKnapsack,parallelBinaryKnapsack>(argc, argv);
}
*/

int main(int argc, char* argv[])
{
#ifdef ACRO_HAVE_MPI
  return driver<binaryKnapsack, parallelBinaryKnapsack, teamBinaryKnapsack, parallelTeamBinaryKnapsack>(argc, argv, MPI_COMM_WORLD);
#else
  return driver<binaryKnapsack, parallelBinaryKnapsack, teamBinaryKnapsack, parallelTeamBinaryKnapsack>(argc, argv, 0);
#endif
}
