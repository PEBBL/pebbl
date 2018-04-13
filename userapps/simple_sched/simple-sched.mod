##
## MIP formulation for 1 | prec | sum w_jC_j used for Alenex talk
##

## FIXED PARAMETERS

param n > 0 integer;			## Number of jobs

set Jobs := 0..n-1;

param p {Jobs} >= 0 integer; 			## Job Duration (units)

param w {Jobs} >= 0 integer;		## Job weight (for objective)

			## Pairs of jobs where precedence constraints apply
			## (pred, succ)

set Precedence within Jobs cross Jobs;  

## INDUCED PARAMETERS

# The makespan of the schedule (and latest finish time)
param T := sum{j in Jobs} p[j];

# Transitive closure of all the precedence relations. This code is
# taken from the AMPL manual, except they had n-1 iterations

set step{s in 1..ceil((n-1)/2)} dimen 2 :=
  if s==1
    then Precedence
    else step[s-1] union setof {k in Jobs, (i,k) in step[s-1], (k,j) in step[s-1]} (i,j);

set TransPrec := step[ceil((n-1)/2)];

# Earliest finish time
param EFT{j in Jobs} := p[j] + sum{k in Jobs: (k,j) in TransPrec} p[k];

# Latest finish time
param LFT{j in Jobs} := T - sum{k in Jobs: (j,k) in TransPrec} p[k];

# Set of possible finish times

set FinishWindow{j in Jobs} := EFT[j]..LFT[j];

## VARIABLES

var finish {j in Jobs, t in FinishWindow[j]} binary;

## OBJECTIVE

minimize WCT:
	sum{j in Jobs, t in FinishWindow[j]} w[j] * t * finish[j,t];

##
## CONSTRAINTS
##

subject to TaskDone {j in Jobs}:
     sum{t in FinishWindow[j]} finish[j,t] = 1;


subject to UnitWork {tu in 1..T}:
	sum{j in Jobs,  t in tu..tu+p[j]-1 inter FinishWindow[j]} 
			finish[j,t] <= 1;

# Job k must start after predecessor j has ended.

subject to PrecConstraint{j in Jobs,k in Jobs,tu in FinishWindow[j]: (j,k) in Precedence}:
	sum{t in EFT[j]..tu}
		finish[j,t] >=  sum{t in EFT[k]..tu + p[k]} finish[k,t];


# SUCASA SYMBOLS: n p w EFT LFT T
