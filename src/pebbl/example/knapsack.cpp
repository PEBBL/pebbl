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
#ifdef ACRO_HAVE_MPI
#include <pebbl/example/parKnapsack.h>
#include <pebbl/pbb/parTeamBranching.h>

using namespace pebbl;
using namespace std;


// Forward declaration

class teamBinKnapSub;


// Branching class for team knapsack

class teamBinaryKnapsack : 
virtual public binaryKnapsack,  
virtual public teamBranching {

protected:

  branchSub* blankSub();  // Overrides the version in binaryKnapsack to make teams version

  // teamOrganize is basically a dummy here, but demonstrate use of teamComm
  // Just (very inefficiently) compute the size of the team communicator
  void teamOrganize() 
  { 
    ucout << "teamBinaryKnapsack teamOrganize called\n";
    int one = 1;
    int countedSize = -1;
    teamComm.reduceCast(&one,&countedSize,1,MPI_INT,MPI_SUM);
    ucout << "teamComm current appears to have " << countedSize << " MPI ranks\n";
  }

  // Utility routine for minion operations

  void printSerial(const string& descrip)
  {
    int serialNumber = -1;
    teamComm.broadcast(&serialNumber,1,MPI_INT,0);
    ucout << "Minion " << teamComm.myRank() << ' ' << descrip << " subproblem " 
          << serialNumber << std::endl;
   
  }

  // In this test application, the minions don't really do anything; they just
  // announce their existence and print the serial number of the subproblem they're
  // bounding

  void minionBound() {
    printSerial("bounding");
  }

  void minionSplit() {
    printSerial("separating");
  }

  void minionMakeChild() {
    printSerial("making child of");
  }

public:

  // Main constructor to call when just doing the team application

  teamBinaryKnapsack(MPI_Comm _comm) :
    binaryKnapsack(),
    teamBranching(_comm) { };

  // Empty constructor to be called from the teamParallel class
  teamBinaryKnapsack()
  { };

  // Reset should reset both base classes
  void reset()
  {
     binaryKnapsack::reset();
     teamBranching::reset();
  }



  ~teamBinaryKnapsack() { };

};


// A class for subproblems that just do dummy alerts for each operation

class teamBinKnapSub : virtual public binKnapSub
{
  friend class teamBinaryKnapsack;
  friend class binKnapSub;
  friend class binaryKnapsack;

protected:
  // Pointer to the team branching object
  teamBinaryKnapsack* tkGlobal;

public:
  // Constructor does nothing special
  teamBinKnapSub() { };

  // Destructor also does nothing special
  ~teamBinKnapSub() { };

  // We have our own new kind of global pointer
  teamBinaryKnapsack* global() const { return tkGlobal; };

  // Routine to set this pointer
  void setGlobalInfo(teamBinaryKnapsack* tkGlobal_)
  {
    tkGlobal = tkGlobal_;
    binKnapSubFromKnapsack(tkGlobal_);
  }

  // Redefine bGlobal to use the team pointer
  branching* bGlobal() const { return global(); };

  // boundComputation alerts the minions (who just print something since this is dummy app)
  void boundComputation(double* controlParam)
  {
    global()->alertBound();
    global()->teamComm.broadcast(&(id.serial),1,MPI_INT,0);
    binKnapSub::boundComputation(controlParam);
  }

  // Same for separation
  int splitComputation()
  {
    global()->alertSeparate();
    global()->teamComm.broadcast(&(id.serial),1,MPI_INT,0);
    return binKnapSub::splitComputation();
  }

  // Same for makeChild, but here we need to be careful to make a new teamBinKnapSub*
  branchSub* makeChild(int whichChild)
  {
    global()->alertMakeChild();
    global()->teamComm.broadcast(&(id.serial),1,MPI_INT,0);
    teamBinKnapSub* temp = new teamBinKnapSub();
    temp->tkGlobal = global();
    // This replicates the logic inside the serial makeChild
    temp->binKnapSubAsChildOf(this,whichChild);
    return temp;
  }

};


// This routine causes team knapsack children to be generated

branchSub* teamBinaryKnapsack::blankSub()
{
  teamBinKnapSub* newSP = new teamBinKnapSub();
  newSP->setGlobalInfo(this);
  return newSP;
};


// Team parallel version

// Forward declaration
class parTeamBinKnapSub;

class parallelTeamBinaryKnapsack : 
  virtual public parallelBinaryKnapsack, 
  virtual public parallelTeamBranching,
  virtual public teamBinaryKnapsack {

  // This will be defined below and is what causes team subproblem objects
  // to be made
  parallelBranchSub* blankParallelSub();

  // To allow teams to organize.  Make sure inheritance works as expected,
  // but then just do the same thing as in the team-only version
  void teamOrganize() 
  { 
    DEBUGPR(1,ucout << "parallelTeamBinaryKnapsack teamOrganize called\n");
    teamBinaryKnapsack::teamOrganize();
  }

  void printOpAndID(const string& opDescrip)
  {
    int idArray[2];
    teamComm.broadcast(idArray,2,MPI_INT,0);
    ucout << "Parallel minion " << teamComm.myRank() << ' ' << opDescrip
          << " subproblem " << idArray[0] << ':' << idArray[1] << std::endl;
  }

  // In this test application, the minions don't really do anything; they just
  // announce their existence
  
  void minionBound() {
    printOpAndID("bounding");
  }

  void minionSplit() {
    printOpAndID("separating");
  }

  void minionMakeChild() {
    printOpAndID("making child of");
  }

  // Test ramp-up start and ramp-up end functions

  void rampUpSetup()
  {
    ucout << "Ramp-up setup called\n";
    if (inATeam())
      teamComm.barrier();
  }

  void rampUpCleanUp()
  {
    ucout << "Ramp-up clean-up called\n";
    if (inATeam())
      teamComm.barrier();
  }

  public:
    parallelTeamBinaryKnapsack(MPI_Comm _comm) :
      parallelTeamBranching(_comm), 
      parallelBinaryKnapsack(_comm)
    { };

   // Destructor
  ~parallelTeamBinaryKnapsack() 
  { };

   void reset(bool VBflag=true)
    {
      binaryKnapsack::reset();
      if (iAmSearcher())
        registerFirstSolution(new binKnapSolution(this));
      parallelTeamBranching::reset();
    }

    bool setup(int& argc, char**& argv){
      return parallelTeamBranching::setup(argc, argv);
    }

    virtual void printSolution(const char* header,
		      const char* footer,
		      std::ostream& outStream){
      parallelBinaryKnapsack::printSolution(header, footer, outStream);
    }

};


// The subproblem class for parallel + teams

class parTeamBinKnapSub :
  virtual public parBinKnapSub
  {
public:

  parTeamBinKnapSub() { };
  
  void parTeamBinKnapSubFromGlobal(parallelTeamBinaryKnapsack* master_)
    {
      ptkGlobal = master_;
      parBinKnapSubFromParKnapsack(master_);
    };

  void parTeamBinKnapSubAsChildOf(parTeamBinKnapSub* parent,int whichChild)
    {
      ptkGlobal = parent->global();
      parBinKnapSub::globalPtr = parent->globalPtr;
      // This will initialize earlier pieces
      binKnapSubAsChildOf(parent,whichChild);
    };

  ~parTeamBinKnapSub() 
    { };

  parallelTeamBinaryKnapsack* global()   const { return ptkGlobal; };
  parallelTeamBranching*      ptGlobal() const { return ptkGlobal; };

  void sendId()
  {
    int sendArray[2];
    sendArray[0] = id.creatingProcessor;
    sendArray[1] = id.serial;
    global()->teamComm.broadcast(sendArray,2,MPI_INT,0);
  }

  // The bound, split, and makeChild operations send the relevant code to the 
  // team members, along with the current subproblem ID, just to give them some
  // to print for validation purposes

  void boundComputation(double* controlParam)
  {
    global()->alertBound();
    sendId();
    parBinKnapSub::boundComputation(controlParam);
  }

  // Same for separation
  int splitComputation()
  {
    global()->alertSeparate();
    sendId();
    return parBinKnapSub::splitComputation();
  }

  parallelBranchSub* makeParallelChild(int whichChild)
    {
      global()->alertMakeChild();
      sendId();
      parTeamBinKnapSub* temp = new parTeamBinKnapSub;
      temp->parTeamBinKnapSubAsChildOf(this,whichChild);
      return temp;
    };

 protected:

  parallelTeamBinaryKnapsack* ptkGlobal;

};


// Makes the root problem for parallel + teams

parallelBranchSub* parallelTeamBinaryKnapsack::blankParallelSub()
{
  parTeamBinKnapSub* newSP = new parTeamBinKnapSub();
  newSP->parTeamBinKnapSubFromGlobal(this);
  return newSP;
};



// Main program

int main(int argc, char* argv[])
{
  return driver<binaryKnapsack, parallelBinaryKnapsack, 
                teamBinaryKnapsack, parallelTeamBinaryKnapsack>(argc, argv);
}


#endif