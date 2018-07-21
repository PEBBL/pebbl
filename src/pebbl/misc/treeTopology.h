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
 * \file treeTopology.h
 * \author Jonathan Eckstein
 *
 * Defines the pebbl::treeTopology class.
 */

#ifndef pebbl_treeTopology_h
#define pebbl_treeTopology_h

#include <pebbl_config.h>

//  JE allows MPI compiles with partially developed CMake
#if     defined(HAVE_MPI) && !defined(ACRO_HAVE_MPI)
#define ACRO_HAVE_MPI
#endif

#ifdef ACRO_HAVE_MPI

// Should not need mpiUtil any more
// #include <pebbl/utilib/mpiUtil.h>

#include <pebbl/misc/fundamentals.h>

namespace pebbl {

// Should not need this any more
// using utilib::uMPI;

///
/// treeTopology is an abstract class for handling trees of processors.
/// nAryTree is a derived class describing a balanced tree with arbitrary
/// root and radix.
///
class treeTopology 
{
protected:

  int node;
  int root;
  int size;

public:
  virtual int numChildren()         = 0;
  virtual int parent()              = 0;
  virtual int childrenLeft()        = 0;
  virtual int validChild(int where) = 0;
  virtual int whichChild(int where) = 0;
  virtual int currentChild()        = 0;
  virtual int advanceChild()        = 0;
  virtual void resetChildren()      = 0;

  virtual bool isRoot()   { return node == root; };
  virtual int  rootNode() { return root; };

  bool isLeaf() { return numChildren() == 0; };

  int operator++(int) { return advanceChild(); };  // postfix ++

  treeTopology(int root_,    // Formerly defaulted to 0,
	       int node_,    // Formerly defaulted to uMPI::rank,
	       int size_     // Formerly defaulted to uMPI::size
               ) :
     node(node_), root(root_), size(size_) 
       { };

  virtual ~treeTopology() { };
      
};




class nAryTree : public treeTopology
{

protected:

  int radix;
  int children;
  int position;
  int firstChild;
  int count;
  int relNode;
  int relFirstChild;

  int relPosition(int where)
    {
      where -= root;
      if (where < 0)
	where += size;
      return where;
    };

public:

  int numChildren()  { return children; };
  int parent()       { return ((relNode - 1)/radix + root) % size; };
  int childrenLeft() { return count; };
  int currentChild() { return position; };

  void resetChildren() 
    {
      count = children;
      position = firstChild;
    };
  
  int advanceChild()
    {
      int temp = position++;
      if (position == size)
	position = 0;
      count--;
      return temp;
    };
  
  int validChild(int where) 
    { 
      int rw = relPosition(where);
      return (rw >= relFirstChild) && (rw < relFirstChild + children);
    };
  
  int whichChild(int where) { return (relPosition(where) - 1) % radix; };

  nAryTree(int radix_, //  Formerly defaulted to 2,
	   int root_,  //  Formerly  0,
	   int node_,  //  Formerly defaulted to uMPI::rank,
	   int size_   //  Formerly defaulted to uMPI::size
           ) :
  treeTopology(root_,node_,size_),
  radix(radix_)
    {
      relNode = relPosition(node);
      relFirstChild = relNode*radix + 1;
      children = std::min(radix,std::max(size - relFirstChild,0));
      firstChild = (relFirstChild + root) % size;
      resetChildren();
    };

};


class nAryTreeRememberParent : public nAryTree
{
private:
  int myParent;
public:

  int parent() { return myParent; };

  nAryTreeRememberParent(int radix_, //= 2,
			 int root_,  //= 0,
			 int node_,  //= uMPI::rank,
			 int size_   //= uMPI::size
                         ) :
  nAryTree(radix_,root_,node_,size_)
    {
      myParent = nAryTree::parent();
    };
};


} // namespace pebbl

#endif

#endif
