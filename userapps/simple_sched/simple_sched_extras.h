#ifndef SIMPLE_SCHED_EXTRAS_H
#define SIMPLE_SCHED_EXTRAS_H

#include <utilib/BasicArray.h>
#include <utilib/IntVector.h>
#include <utilib/Tuple.h>
#include <utilib/sort.h>
#include <pico/BCMip.h>
#include <simple_sched_info.h>
#include <simple_sched_milp.h>

using namespace std;
using namespace utilib;

namespace sucasa_simple_sched{

// We reprent the solution as a completion time for each job.
// It's easier than having to pull it from the sparsely-stored
// MIP solution.
class SSSolution:
public pebbl::solution
{
 public:
  IntVector finishTimes;

  // For now, we will just fill in the finish times after construction.
  SSSolution() {};

  // Constructor that takes the MILP object (to set serial number, etc)

  SSSolution(MILP* thisMILP):
    solution(thisMILP)
    {};

  // Constructor that takes the MILP object (to set serial number, etc)
  // and allocates space for the solution.

  SSSolution(MILP* thisMILP, int numJobs):
    solution(thisMILP), finishTimes(numJobs)
    {};

  const char* typeDescription() const{ return "Simple Sched solution"; };

  /*
  bool operator==(const SSSolution& other) const
    {
      size_type numJobs := finishTimes.size();
      for (size_type i=0; i < numJobs; i++)
	{
	  if (finishTimes[i] != other.finishTimes[i])
	    return(false);
	}
      return(true);
    };

  void printContents(std::ostream& s)
    {
      size_type numJobs := finishTimes.size();
      s << "Finish times:\n";
      for (size_type i = 0; i < numJobs; i++)
	s << "Job " << i << ": " << finishTimes[i] << endl;
    };
  */

 protected:
  // The sequence representation of a solution for computing a
  // hash value.  Just the list of finish times (in order)

  // Obviously this will only work once there is a real
  // solution here.  But we don't compute hashvalues till
  // there is a solution stored.
  size_type sequenceLength() {return finishTimes.size();};
  // Return the item of the sequence indicated by the sequenceCursor
  // (and move the cursor along).
  double sequenceData() {return((double)finishTimes[sequenceCursor++]);};
};

} // namespace sucasa_simple_sched
#endif
