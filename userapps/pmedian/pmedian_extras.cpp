// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
// This is an implementation of the Volume algorithm for uncapacitated
// facility location problems. See the references
// F. Barahona, R. Anbil, "The Volume algorithm: producing primal solutions
// with a subgradient method," IBM report RC 21103, 1998.
// F. Barahona, F. Chudak, "Solving large scale uncapacitated facility
// location problems," IBM report RC 21515, 1999.

// Changes:
// Sparse version by Erik Boman and Jon Berry, Oct 2006
// p-median option added by Erik Boman, Oct 2006.
// These changes to the original code are copyrighted Sandia Corporation, 2006.

//#include <valarray>
#include <cstdio>
#include <set>
#include <list>
#include <vector>
#include <cmath>
#ifndef WIN32
#include <sys/times.h>
#endif

#include "pmedian_extras.h"
#include "ufl.h"

namespace pmedian {

void MILPNode::boundComputation(double* controlParam)
{
   *controlParam = 1;                 // 1 problem
   std::vector<int> sensorLocations;  // solution

   UFL_parms ufl_par("ufl.par");           // parameters
   
   UFL  ufl_data;                          // data
   VOL_dvector& fcost = ufl_data.fcost;

   // Maps to map indices from problem to contiguous integers 
   std::map<int,int> loc_map;
   std::map<int,int> cust_map;

   int nLocations = numNetworkLocations(); // number of locations 
   int nCustomers = numEvents(mc);         // number of events 
   ufl_data._p = GoalBound(ns, total);     // number of sensors

   GoalMeasurement_const_iterator curr = GoalMeasurement_valid().begin();
   GoalMeasurement_const_iterator last = GoalMeasurement_valid().end();

   int npairs = 0;
   int nloc = 0;
   int ncust = 0;
   bool dummy = false;

   while (curr != last){         // iterate through impacts
     int customer = curr->val2;
     int location = curr->val3;
     if (!dummy) dummy = (location < 0);

     if (loc_map.find(location) == loc_map.end()){
       loc_map[location] = nloc++;
       ufl_data.problem_loc.insert(ufl_data.problem_loc.end(), location);
     }
     if (cust_map.find(customer) == cust_map.end()){
       cust_map[customer] = ncust++;
       ufl_data.problem_cust.insert(ufl_data.problem_cust.end(), customer);
     }
     if (ufl_data._dist.find(loc_map[location]) == ufl_data._dist.end()) {
     	std::map<int,double> row;
	ufl_data._dist[loc_map[location]] = row;
     }
     ufl_data._dist[loc_map[location]][cust_map[customer]] = 
       GoalMeasurement(*curr);

     npairs++;
     curr++;
   }

   if (dummy) 
     ufl_data._p++; // One sensor will be placed at dummy location

   if ((nloc > nLocations) || (ncust != nCustomers)){
     cout << "Invalid problem data in UFL_read_data" << endl;
     exit(1);
   }

   ufl_data.nloc = nloc;
   ufl_data.ncust = ncust;
   ufl_data.npairs = npairs;

   if (ufl_data._p > 0){
     // No facility open costs; set them to zero.
     fcost.allocate(nloc);
     fcost= 0.0;
   }
   // No fixed facilities
   ufl_data.fix.allocate(nloc);
   ufl_data.fix = -1;


   // create the VOL_problem from the parameter file
   VOL_problem volp("ufl.par");
   // volp.psize = ufl_data.nloc + ufl_data.nloc*ufl_data.ncust;
   volp.psize = ufl_data.nloc + ufl_data.npairs;
   volp.dsize = ufl_data.ncust;
   bool ifdual = false;
   if (ufl_par.dualfile.length() > 0) {
     // read dual solution
      ifdual = true;
      VOL_dvector& dinit = volp.dsol;
      dinit.allocate(volp.dsize);
      // read from file
      FILE * file = fopen(ufl_par.dualfile.c_str(), "r");
      if (!file) {
	 printf("Failure to open file: %s\n ", ufl_par.dualfile.c_str());
	 abort();
      }
      const int dsize = volp.dsize;
      int idummy;
      for (int i = 0; i < dsize; ++i) {
	 fscanf(file, "%i%lf", &idummy, &dinit[i]);
      }
      fclose(file);
   }

#if 0
   // This would be the right place to set bounds on the dual variables
   // For UFL all the relaxed constraints are equalities, so the bounds are 
   // -/+inf, which happens to be the Volume default, so we don't have to do 
   // anything.
   // Otherwise the code to change the bounds would look something like this:

   // first the lower bounds to -inf, upper bounds to inf
   volp.dual_lb.allocate(volp.dsize);
   volp.dual_lb = -1e31;
   volp.dual_ub.allocate(volp.dsize);
   volp.dual_ub = 1e31;
   // now go through the relaxed constraints and change the lb of the ax >= b 
   // constrains to 0, and change the ub of the ax <= b constrains to 0.
   for (i = 0; i < volp.dsize; ++i) {
     if ("constraint i is '<=' ") {
       volp.dual_ub[i] = 0;
     }
     if ("constraint i is '>=' ") {
       volp.dual_lb[i] = 0;
     }
   }
#endif

#ifndef WIN32
   // start time measurement
   double t0;
   struct tms timearr; clock_t tres;
   tres = times(&timearr); 
   t0 = timearr.tms_utime; 
#endif
   int flag=0;
   // invoke volume algorithm
   if (volp.solve(ufl_data, ifdual) < 0) {
      printf("solve failed...\n");
      setState(dead, globalPtr);
      flag=1;
   } else {
      // recompute the violation
      const int n = ufl_data.nloc;
      const int m = ufl_data.ncust;

      VOL_dvector v(volp.dsize);
      const VOL_dvector& psol = volp.psol;
      v = 1;

/***  JWB
      int i,j,k=n;
      for (j = 0; j < n; ++j){
	for (i = 0; i < m; ++i) {
	  v[i] -= psol[k];
	  ++k;
	}
      }
***/

   // ** JWB sparse replacement code
   int k=0;
   for (int i=0; i < n; i++){
     std::map<int,double>::iterator col_it = ufl_data._dist[i].begin();
     for (; col_it!=ufl_data._dist[i].end(); col_it++) {
       std::pair<int,double> p = *col_it;
       int j = p.first;
       v[j]-=psol[n+k];
       ++k;
     }
   }

      double vc = 0.0;
      for (int i = 0; i < m; ++i)
         // MS Visual C++ has difficulty compiling 
         //   vc += std::abs(v[i]);
         // so just use standard fabs.
         vc += fabs(v[i]);
      vc /= m;
      DEBUGPR(2, 
       ucout << " Average violation of final solution: " << vc << endl);

      if (ufl_par.dual_savefile.length() > 0) {
	// save dual solution
	 FILE* file = fopen(ufl_par.dual_savefile.c_str(), "w");
	 const VOL_dvector& u = volp.dsol;
	 int n = u.size();
	 int i;
	 for (i = 0; i < n; ++i) {
	    fprintf(file, "%8i  %f\n", i+1, u[i]);
	 }
	 fclose(file);
      }

      // run a couple more heuristics
      double heur_val;
      for (int i = 0; i < ufl_par.h_iter; ++i) {
	 heur_val = DBL_MAX;
	 ufl_data.heuristics(volp, psol, heur_val);
      }
      // save integer solution

      for (int i = 0; i < ufl_data.nloc; ++i) {
        if ( (ufl_data.ix[i]==1 ) && (ufl_data.problem_loc[i] >= 0) ){
          sensorLocations.insert(sensorLocations.end(), 
                                ufl_data.problem_loc[i]);
        }
      }

      if (ufl_par.int_savefile.length() > 0) {
	 FILE* file = fopen(ufl_par.int_savefile.c_str(), "w");
	 const VOL_ivector& x = ufl_data.ix;
	 const int n = ufl_data.nloc;
	 const int m = ufl_data.ncust;
	 int i,j,k=n;
	 fprintf(file, "Open locations\n");
	 for (i = 0; i < n; ++i) {
	   if ( x[i]==1 ){
	     fprintf(file, "%8i\n", ufl_data.problem_loc[i]);
           }
	 }
	 fprintf(file, "Assignment of customers\n");
	 //for (i = 0; i < n; ++i) {
	   //for (j = 0; j < m; ++j) {
	     //if ( x[k]==1 ) 
	       //fprintf(file, "customer %i  location %i\n", j+1, i+1);
	     //++k;
	   //}
	 //}
         // sparse replacement code
         for (int i=0; i < n; i++){
           std::map<int,double>::iterator col_it = ufl_data._dist[i].begin();
           for (; col_it!=ufl_data._dist[i].end(); col_it++) {
             std::pair<int,double> p = *col_it;
             int j = p.first;
             if ( x[k]==1 )
               //fprintf(file, "customer %i  location %i\n", j+1, i+1);
               fprintf(file, "customer %i  location %i\n", 
                     ufl_data.problem_cust[j], ufl_data.problem_loc[i]);
             ++k;
           }
         }

	 fclose(file);
      }
   }

   if (!flag){
     cout << " Best integer solution value: " << ufl_data.icost << endl;
     cout << " Lower bound: " << volp.value << endl;
     cout << " Chosen sensor locations: " << endl;
     for (int i=0; i<sensorLocations.size(); i++){
       cout << " " << sensorLocations[i] ;
       if (i && (i%10 == 0)){
          cout << endl;
       }
     }
     cout << endl;

     // Number of variables that are fractional, set here so
     // that branchSub::candidateSolution() will return TRUE.
     // This indicates we have a valid solution.

     nFrac = 0; 
     setState(bounded);

     // Solution: I'm listing the location IDs where sensors should
     // be placed.  But this isn't really meaningful to PICO.

     DoubleVector vec(sensorLocations.size());
     double value = static_cast<double>(ufl_data.icost);

     for (int i=0; i<sensorLocations.size(); i++){
       vec[i] = static_cast<double>(sensorLocations[i]);
     }

     bcGlobal()->updateIncumbent(vec, value);
   }

#ifndef WIN32
   // end time measurement
   tres = times(&timearr);
   double t = (timearr.tms_utime-t0)/100.;
   printf(" Total Time: %f secs\n", t);
#endif

   return ;
}
void MILPNode::updateIncumbent()
{
  setState(dead);
};



// Verify that the map and the dvector contain identical info.
bool verifyMap(VOL_dvector& d, int ncust, std::map<int, std::map<int,double> >& m)
{
	std::map<int, std::map<int,double> >::iterator row_it = m.begin();
	for (; row_it!=m.end(); row_it++) {
		std::pair<int,std::map<int,double> > p = *row_it;
		int i = p.first;
		std::map<int,double> row = p.second;
		std::map<int,double>::iterator col_it = row.begin();
		for (; col_it!=row.end(); col_it++) {
			std::pair<int,double> p = *col_it;
			int j = p.first;
			double value = p.second;
			if (value != d[i*ncust + j])
				return false;
		}
	}
	return true;
}
			
// Convert from map to compressed dvector.	
// Also create pair vector to track indices i and j. 
bool map2dvector(VOL_dvector& d, std::map<int, std::map<int,double> >& m, std::vector<std::pair<int,int> >& ind)
{
	int k=0;
	std::map<int, std::map<int,double> >::iterator row_it = m.begin();
	for (; row_it!=m.end(); row_it++) {
		std::pair<int,std::map<int,double> > p = *row_it;
		int i = p.first;
		std::map<int,double> row = p.second;
		std::map<int,double>::iterator col_it = row.begin();
		for (; col_it!=row.end(); col_it++) {
			std::pair<int,double> p = *col_it;
			int j = p.first;
			double value = p.second;
                        if (k<d.size()){
				std::pair<int,int> p;
				p.first = i;
				p.second = j;
				ind.insert(ind.end(), p);
				// ind[k].first = i;
				// ind[k].second = j;
                        	d[k] = value;
				k++;
			}
                        else
				// Index out of bounds
				return false;
		}
	}
	return true;
}
			
// Convert from compressed dvector to map.
bool dvector2map(VOL_dvector& d, std::map<int, std::map<int,double> >& m, std::vector<std::pair<int,int> >& ind)
{
	int k=0;
	std::pair<int,int> p;
	std::vector<std::pair<int,int> >::iterator vec_it = ind.begin();
	for (; vec_it!=ind.end(); vec_it++) {
		int i = vec_it->first;
		int j = vec_it->second;
		m[i][j] = d[k++];
	}
	return true;
}
			

void MILP::newIncumbentEffect(double new_value)
{
  problemFeasible = true;
};

void MILP::serialPrintSolution(const char* header,
			     const char* footer,
			     ostream& outStream)
{
branching::serialPrintSolution(header,"",outStream);  // prints obj value
 size_type numSensors = incumbent.size();
 outStream << "Final set of Sensors: \n";
 for (size_type i = 0; i < numSensors; i++)
   {
     outStream << " " << incumbent[i] ;
     if (i && (i%10 == 0)){
       outStream << endl;
     }
   }
 outStream << endl;
}

} //namespace


//############################################################################

//###### USER HOOKS
// compute reduced costs
int
UFL::compute_rc(const VOL_dvector& u, VOL_dvector& rc)
{
   int i,j,k=0;
   for ( i=0; i < nloc; i++){
     rc[i]=fcost[i];
     // for (j = 0; j < ncust; ++j) {
     std::map<int,double>::iterator col_it = _dist[i].begin();
     for (; col_it!=_dist[i].end(); col_it++) {
       std::pair<int,double> p = *col_it;
       int j = p.first;
       double distij = p.second;
       //rc[nloc+k]= dist[k] - u[j];
       rc[nloc+k]= distij - u[j];
       ++k;
     }
   }
   return 0;
}

// IN: dual vector u
// OUT: primal solution to the Lagrangian subproblem (x)
//      optimal value of Lagrangian subproblem (lcost)
//      v = difference between the rhs and lhs when substituting
//                  x into the relaxed constraints (v)
//      objective value of x substituted into the original problem (pcost)
//      xrc
// return value: -1 (volume should quit) 0 ow

int 
UFL::solve_subproblem(const VOL_dvector& u, const VOL_dvector& rc,
		      double& lcost, 
		      VOL_dvector& x, VOL_dvector& v, double& pcost)
{
   int i,j;

   lcost = 0.0;
   for (i = 0; i < ncust; ++i) {
      lcost += u[i];
      v[i]=1;
   }

   // VOL_ivector sol(nloc + nloc*ncust);
   VOL_ivector sol(x.size());

   // Produce a primal solution of the relaxed problem
   // For p-median, we can only open p facilities
 
   const double * rdist = rc.v + nloc;
   double sum;
   int k=0, k1=0;
   double value=0.;
   int xi;
   std::map<int,double>::iterator col_it;

   if (_p == 0) { // UFL
     for ( i=0; i < nloc; ++i ) {
       sum=0.;
       // for ( j=0; j < ncust; ++j ) {
       for (col_it = _dist[i].begin(); col_it!=_dist[i].end(); col_it++) {
         // std::pair<int,double> p = *col_it;
         // int j = p.first;
         // rdist are the "reduced" distance costs in compressed form
         if ( rdist[k]<0. ) sum+=rdist[k];
         ++k;
       }
       if (fix[i]==0) xi=0;
       else 
         if (fix[i]==1) xi=1;
         else 
  	 if ( fcost[i]+sum >= 0. ) xi=0;
  	 else xi=1;
       sol[i]=xi;
       value+=(fcost[i]+sum)*xi;
       // for ( j=0; j < ncust; ++j ) {
       for (col_it = _dist[i].begin(); col_it!=_dist[i].end(); col_it++) {
         // std::pair<int,double> p = *col_it;
         // int j = p.first;
         if ( rdist[k1] < 0. ) sol[nloc+k1]=xi;
         else sol[nloc+k1]=0;
         ++k1;
       }
     }
   }
   else { // _p>0, p-median
     std::pair<int,double> rho[_p+1];  // p lowest rho, as in Avella et al.
                                       // (index, value) pairs
     std::pair<int,double> rho_temp; 
     int q;

     for (q=0; q<_p; q++) {
       rho[q].first = -1;
       rho[q].second = 0;
     }

     for ( i=0; i < nloc; ++i ) {
       sum=0.;
       // for ( j=0; j < ncust; ++j ) {
       for (col_it = _dist[i].begin(); col_it!=_dist[i].end(); col_it++) {
         // std::pair<int,double> p = *col_it;
         // int j = p.first;
         // rdist are the "reduced" distance costs in compressed form
         if ( rdist[k]<0. ) sum+=rdist[k];
         ++k;
       }

       // Save p lowest rho values
       rho_temp.first = i;
       // rho_temp.second = sum - u[i]; // Avelli et al. use y[j] for x[j,j]
       rho_temp.second = sum;
       rho[_p] = rho_temp;
       for (q=_p; q>0; q--){
         if (rho[q-1].first == -1 || (rho[q].second < rho[q-1].second)){
           rho_temp = rho[q-1];
           rho[q-1] = rho[q];
           rho[q] = rho_temp;
         }
         else
           break;
       }
     }

     // Set up start array for solution indices (Need only be done once)
     int start[nloc];
     k=nloc;
     for ( i=0; i < nloc; ++i ) {
       start[i] = k;
       for (col_it = _dist[i].begin(); col_it!=_dist[i].end(); col_it++) {
         ++k;
       }
     }

     // Compute x and function value based on smallest rho values
     value = 0;
     for ( i=0; i < nloc+npairs; ++i )
       sol[i] = 0;
     // Loop over open locations
     int ii;
     for ( ii=0; ii < _p; ++ii ) {
       i = rho[ii].first;
       // Open a facility at location i
       sol[i]=1;
       // printf("Debug: Open facility at location %d, rho= %f\n",
         // i, rho[ii].second);
       value+= rho[ii].second;
       // for ( j=0; j < ncust; ++j ) {
         // k1 = nloc+rho_ind[i]*ncust+j;
       k1 = start[i];
       for (col_it = _dist[i].begin(); col_it!=_dist[i].end(); col_it++) {
         // Set customer-facility variables
         if ( rdist[k1-nloc] < 0. ) sol[k1] = 1;
         ++k1;
       }
     }
   }

   lcost += value; 

   pcost = 0.0;
   x = 0.0;
   for (i = 0; i < nloc; ++i) {
     if (_p == 0) // Include facility open costs for UFL, not p-median
       pcost += fcost[i] * sol[i];
     x[i] = sol[i];
   }

   k = 0;
   for ( i=0; i < nloc; i++){
     // for ( j=0; j < ncust; j++){
     std::map<int,double>::iterator col_it = _dist[i].begin();
     for (; col_it!=_dist[i].end(); col_it++) {
       std::pair<int,double> p = *col_it;
       int j = p.first;
       x[nloc+k]=sol[nloc+k];
       //pcost+= dist[k]*sol[nloc+k];
       pcost+= _dist[i][j]*sol[nloc+k];
       v[j]-=sol[nloc+k];
       ++k;
     }
   }
   
   return 0;
}


// IN:  fractional primal solution (x),
//      best feasible soln value so far (icost)
// OUT: integral primal solution (ix) if better than best so far
//      and primal value (icost)
// returns -1 if Volume should stop, 0/1 if feasible solution wasn't/was
//         found.
// We use randomized rounding. We look at x[i] as the probability
// of opening facility i.

int
UFL::heuristics(const VOL_problem& p,
		const VOL_dvector& x, double& new_icost)
{
   // VOL_ivector nsol(nloc + nloc*ncust);
   VOL_ivector nsol(nloc + npairs);
   nsol=0;
   int i,j;
   double r,value=0;
   int nopen=0;

   for ( i=0; i < nloc; ++i){ // open or close facilities
#ifndef WIN32
     r=drand48();
#else
     r=rand();
#endif
     // Simple randomized rounding
     // TODO: Improve for p-median case (Jon & Cindy)
     if (r < x[i]) 
       if (_p >0){ // p-median
         if (nopen < _p){
           nsol[i]=1;
           ++nopen;
         }
       }
       else 
         nsol[i]=1;
     if (_p == 0)
       value+=fcost[i]*nsol[i]; // Ignore this term for p-median.
   }

   /* OLD CODE
   double xmin;
   int imin;
   for ( j=0; j < ncust; ++j){ // assign customers to locations
     xmin=1.e31;
     imin=-1;
     for ( i=0; i < nloc; ++i){
       if ( nsol[i]==0 ) continue;
       if ( dist[i*ncust+j] < xmin ){ xmin=dist[i*ncust+j]; imin=i; }
       if ( _dist[i][j] < xmin ){ xmin=_dist[i][j]; imin=i; }
     }
     value+=xmin;
     if ( imin >=0 ) nsol[nloc+imin*ncust+j]=1;
   }
   */

   // We assign customers to locations.
   double xmin[ncust];
   int    kmin[ncust];
   int k=0;

   for ( j=0; j < ncust; ++j){ 
     xmin[j]= 1.e31; // huge 
     kmin[j]= -1;
   }

   // Sparse code loops over rows (locations) first.
   std::map<int, std::map<int,double> >::iterator row_it = _dist.begin();
   for (; row_it!=_dist.end(); row_it++) {
     std::pair<int,std::map<int,double> > p = *row_it;
     int i = p.first;
     std::map<int,double> row = p.second;
     std::map<int,double>::iterator col_it = row.begin();
     for (; col_it!=row.end(); col_it++) {
       std::pair<int,double> p = *col_it;
       int j = p.first;
       double distij = p.second;
       if ( nsol[i] && (distij < xmin[j]) ){ 
         xmin[j]= distij;
         kmin[j]= k; 
       }
       ++k;
     }
   }

   // Now xmin and kmin tell us how to assign customers
   for ( j=0; j < ncust; ++j){ 
     value += xmin[j];
     if (kmin[j] >= 0)
       nsol[nloc+kmin[j]]= 1;
     //else
       //printf("Warning: heuristic could not serve customer %d\n", j);
   }


   new_icost = value;
   if (value < icost) {
      icost = value;
      ix = nsol;
   }
  // printf(" int sol %f\n",new_icost);

   return 0;
}
