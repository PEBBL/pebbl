#include <simple_sched_extras.h>
namespace sucasa_simple_sched {

using utilib::Tuple2;
using utilib::order;

void MILPNode::incumbentHeuristic()
{

  int numVars = mip()->numAllVars();
  DoubleVector LPSolution(numVars);
  lp->getColSolution(LPSolution.data());
  register_var_values(LPSolution);

int njobs = n();
 DEBUGPR(10, ucout << "simple_sched incumbent finder called" << endl);
 SSSolution *heurSol = new SSSolution(globalPtr, njobs);
 // Assumed initialized to zero (utilib does this)
DoubleVector frac(njobs);
IntVector    time(njobs);
for (int i=0; i<njobs; i++)
	time[i] = -1;

int curr_job = -1;
symbol_const_iterator curr = finish_indices().begin();
symbol_const_iterator last = finish_indices().end();
// This should work regardless of the ordering of the tuples
while (curr != last) {
  SymbInfo::tuple_t tup = *curr;
  if (frac[tup[0]] < 0.5)
    {
      frac[tup[0]] += finish(tup);
      if (frac[tup[0]] >= 0.5) {
	time[tup[0]] = tup[1];
	DEBUGPR(50, cout << "job " << tup[0] << " alpha point: " << tup[1] << endl);
      }
      curr++;
    }
   else
     curr++;
  }

if (min(time) < 0) {
   EXCEPTION_MNGR(std::runtime_error, "Time is not set for job " << argmin(time));
   }

IntVector index(njobs);
double value = 0.0;

order(index,time);

for (int i=0; i<njobs; i++) {
  //     	cout << "job " << index[i] << " rank: " << i << endl;
	heurSol->finishTimes[i] = 0;
}

value = p(index[0])*w(index[0]);
heurSol->finishTimes[index[0]] = p(index[0]);
// cout << "rank: 0, p: " << p(index[0]) << " w: " << w(index[0]) << endl;
 DEBUGPR(50, cout << "heurSol->finishTimes[" << index[0] << "]: " << heurSol->finishTimes[index[0]] << endl);

for (int i=1; i<njobs; i++) {
  heurSol->finishTimes[index[i]] = heurSol->finishTimes[index[i-1]] + p(index[i]);
  //cout << "rank: " << i << ", p: " << p(index[i]) << " w: " << w(index[i]) << endl;
  DEBUGPR(50, cout << "heurSol->finishTimes[" << index[i] << "]: " << heurSol->finishTimes[index[i]] << endl);
  value += heurSol->finishTimes[index[i]] * w(index[i]);
  }

 DEBUGPR(10, ucout << "simple_sched heuristic solution value: " << value);
 heurSol->value = value;
 foundSolution(heurSol);
};

  // If we find an integer feasible solution through solving an LP,
  // translate to a solution in our form
  // NOTE: not carefully debugged.
pebbl::solution*  MILPNode::extractSolution()
{
  int numVars = mip()->numAllVars();
  DoubleVector FullSolution(numVars);
  lp->getColSolution(FullSolution.data());
  register_var_values(FullSolution);
  int njobs = n();
  SSSolution *newSol = new SSSolution(globalPtr, njobs);
  for (size_type i=0; i < njobs; i++)
    newSol->finishTimes[i] = -1;
  symbol_const_iterator curr = finish_indices().begin();
  symbol_const_iterator last = finish_indices().end();
  while (curr != last) {
    SymbInfo::tuple_t tup = *curr;
    if (finish(tup) > 1.0 - pebbl::pebblBase::integerTolerance)
      newSol->finishTimes[tup[0]] = tup[1];
    curr++;
  }
  // In general, you could call a method to verify that the solution is
  // correct.  In this case, make sure that all jobs have finish times,
  // no two jobs are running at the same time, etc.
  return(newSol);
};

} // namespace
