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
//
//  gRandom.cpp
//
//  A global uniform random number -- coding convenience.
//
// Jonathan Eckstein
//


#include <pebbl_config.h>

//  JE allows MPI compiles with partially developed CMake
#if     defined(HAVE_MPI) && !defined(ACRO_HAVE_MPI)
#define ACRO_HAVE_MPI
#endif

#ifdef ACRO_HAVE_MPI
#include <pebbl/utilib/mpiUtil.h>
#endif
#include <pebbl/utilib/math_basic.h>
#include <pebbl/utilib/_generic.h>
#include <pebbl/utilib/Uniform.h>
#include <pebbl/utilib/PM_LCG.h>

using namespace utilib;

namespace pebbl {


PM_LCG  gRandomLCG(1);
Uniform gRandom(&gRandomLCG);


seed_t randomSeed=1;


RNG* gRandomRNG() {return &gRandomLCG;}

void gRandomReSeed(seed_t seed, 
#ifdef ACRO_HAVE_MPI
        bool processorVariation)
#else
        bool )
#endif
{
#ifdef ACRO_HAVE_MPI
  if (processorVariation && uMPI::running())
    {
      PM_LCG* rng = new PM_LCG(std::max((int)seed,1) + uMPI::rank);
      int numTwiddles = rng->asLong() % 10;
      for (int i=0; i<numTwiddles; i++)
        rng->asLong();
      seed = (seed_t) rng->asLong();
      delete rng;
    }
#endif
  gRandomLCG.set_seed(std::max(seed,static_cast<utilib::seed_t>(1)));
}

void gRandomReSeed()
{ 
  gRandomReSeed(randomSeed,true/*processor variation*/); 
}

} // namespace pebbl
