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
 * \file followerTree.h
 * \author Jonathan Eckstein
 *
 * Organize the followers in a group o clusters into a tree.  
 * Used for pseudocost broadcast
 */

#ifndef pebbl_followerTree_h
#define pebbl_followerTree_h

#include <pebbl_config.h>

//  JE allows MPI compiles with partially developed CMake
#if     defined(HAVE_MPI) && !defined(ACRO_HAVE_MPI)
#define ACRO_HAVE_MPI
#endif

#ifdef ACRO_HAVE_MPI

#include <pebbl/misc/clustering.h>
#include <pebbl/misc/treeTopology.h>

namespace pebbl {


class nAryFollowerTree : public nAryTreeRememberParent
{
private:
  
  clusterObj* cluster;

 
public:

  int currentChild()
    {
      return cluster->globalFollower(nAryTree::currentChild());
    };

  int parent()
    {
      return cluster->globalFollower(nAryTreeRememberParent::parent());
    };

  int operator++(int i)
    {
      return cluster->globalFollower(nAryTree::operator++(i));
    };

  int validChild(int where)
    {
      return nAryTree::validChild(cluster->followerRank(where));
    };

  int whichChild(int where)
    {
      return nAryTree::whichChild(cluster->followerRank(where));
    };

  nAryFollowerTree(clusterObj& cluster_,
		   int root_ = 0,
		   int radix_ = 2) :
  nAryTreeRememberParent(radix_,
			 root_,
			 cluster_.myGlobalFollowerRank,
			 cluster_.totalNumFollowers
			 ),
  cluster(&cluster_)
    { };

};

} // namespace pebbl

#endif

#endif
