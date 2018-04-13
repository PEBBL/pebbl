#
# GeneralSP.mod
#
# AMPL model for sensor placement that uses general impact factors and cost measures.
# Adapted from Dynamic.mod
#
# This MILP model for sensor placement in water networks generalizes the p-median formulation in
# Dynamic.mod to allow for an arbitrary specification of the objective function as well as 
# constraints.  This model is only meaningful if either the objective is to minimize total cost (e.g.
# number of sensors, or if the model is constrained by total cost.
#
# This model allows for the following performance goals:
#	cost
#	ec
#	mc
#	nfd
#	ns
#	pe
#	pk
#	td
#	vc
#       awd
# Note: we want to minimize all of these goals!
#
# This model allows for the computation of a variety of performance measures for these goals:
#	mean performance
#	var (value-at-risk)
#	cvar (conditional value-at-risk)
#	worst performance
#

##
## Objective information
##
#
# The set of allowable performance goals
#
set goals := {"cost", "ec", "mc", "nfd", "ns", "pe", "pk", "td", "vc", "awd"};
#
# Goal measures
# 
set measures := {"mean", "var", "cvar", "worst", "total"};
#
# The optimization objective's goal
#
param objectiveGoal symbolic in goals;
#
# The optimization objective's measure
#
param objectiveMeasure symbolic in measures default "mean";
#
# The goals used for constraints
#
set goalConstraints within goals cross measures diff {(objectiveGoal, objectiveMeasure)};
#
# Active goals
#
set ActiveGoals := ((setof {(g,m) in goalConstraints} g) union {objectiveGoal}) diff {"ns","cost","awd"};
#
# The active goal/measure pairs
#
set ActiveGMPair := goalConstraints union {(objectiveGoal,objectiveMeasure)};
#
# Thresholds used for computing superlocations.
# NOTE: this information is not required for optimization, but it provides information about how
# aggregation is done.
#
param slThreshold {g in ActiveGoals}, >= 0.0;
#
# Ratio of best member in all superlocations to the superlocation value.
# NOTE: this information is not required for optimization, but it can be used to compute
# a valid lower bound when no side-constraints are used.
#
param slAggregationRatio {g in ActiveGoals}, >= 0.0 <= 1.0;
#
# The number of weight categories used for the awd measure
#
param NumWeightCategories default 1;
#
# The target weighted distribution
#
param TargetWeightDistribution {1..NumWeightCategories}, >= 0.0 <= 1.0 default 1.0;
#
# CHECK ERRORS
#
check {(g,m) in ActiveGMPair}: ((m = "total") && (g in {"ns","cost","awd"})) || 
((m != "total") && !(g in {"ns","cost","awd"}));

##
## The water network elements. These are locations in the network where we can
## put sensors.  For now just number them consecutively.  
## Network elements are consecutive integers.  These are the node indices;
##
#
# Number of network locations
#
param numNetworkLocations, integer, > 0;
#
# A dummy location where that all events impact
#
param dummyLocation := -1;
#
# Places where sensors can be placed.
#
set NetworkLocations := 1 .. numNetworkLocations;
#
# Predetermined fixed sensor locations
#
set FixedSensorLocations within NetworkLocations default {};
#
# Predetermined locations where sensors cannot be placed
#
set InfeasibleLocations within NetworkLocations default {};
#
# The set of feasible sensor locations
#
set FeasibleLocations := NetworkLocations diff InfeasibleLocations;

#
# FEASIBILITY CHECKS
#
#check: FixedSensorLocations intersect InfeasibleLocations within {};


##
## Constraint information
##
#
# Bounds
#
param GoalBound {goalConstraints};
#
# The confidence interval, e.g. consider the worst 5% of cases
#
param gamma > 0.0 <= 1.0 default 0.05;
#
# The per-location weight distribution
#
param LocationWeightDistribution {FeasibleLocations,1..NumWeightCategories} >= 0 <= 1 default 1;


##
## Event scenarios are defined by an event location (for us, this will always be a node)
## and a time of day.  However, this model doesn't need to know these, so we'll
## just label events consecutively
##
#
# Number of events
#
param numEvents {ActiveGoals} integer, > 0;
#
# The set of event IDs
#
set Events {g in ActiveGoals} := 1 .. numEvents[g];
#
# Event weights.  You can think of this as a probability that the
# (single-location) event occurs at this location and time
#
param EventWeight {g in ActiveGoals, a in Events[g]} >= 0.0 <= 1.0 default 1.0/numEvents[g];
#
# Event location information (used only for printing sensible outputs (e.g. debugging))
#
param EventLoc  {g in ActiveGoals, Events[g]} default 0;
#
# Event time information (used only for printing sensible outputs (e.g. debugging))
#
param EventTime {g in ActiveGoals, Events[g]} default 0;

##
## Revised location information using event information
##
#
# Locations that could actually see the impact of each event
# Remember that the dummyLocation is touched by all events.
#
set EventTouchedLocations {g in ActiveGoals, a in Events[g]} within NetworkLocations union {dummyLocation};
#
# The IDs of aggregated network location classes.
# NOTE: these are independent of the event scenario, to facilitate 
# data compression.
#
set SuperLocations;
#
# The set of network locations that correspond to each super location 
#
set SuperLocationMembers {SuperLocations} within NetworkLocations union {dummyLocation};
#
# For each goal and event, the set of SuperLocation IDs that define that event
#
set EventSuperLocations {g in ActiveGoals, a in Events[g]} within SuperLocations;
#
# TODO: add checks here?
#

##
## Goal information
##
#
# The goal values used in either the objective or constraints.
#
param GoalMeasurement {g in ActiveGoals, a in Events[g], EventTouchedLocations[g,a]};
#
# The cost information used to compute the 'cost' goal
#
param PlacementCost {FeasibleLocations} default 0.0;
#
# ID of the element of a superlocation set that has the worst value
#
param WorstSuperLocationMember {g in ActiveGoals, a in Events[g], L in SuperLocations} in SuperLocationMembers[L];
#
# Some data checks
#
#check {g in ActiveGoals, a in Events[g], L in EventSuperLocations[g,a], LL in EventSuperLocations[g,a]}: (L == LL) or (L != LL) and SuperLocationMembers[L] intersect SuperLocationMembers[LL] within {};

##
## Decision variables
##
#
# s[loc] = 1 if there's a sensor at location loc, 0 otherwise.
#
var s {FeasibleLocations} binary;
#
# FirstSensorForEvent[EventTouchedLocations] = 1 if and only if this
# touched location is the first sensor hit for this event (among
# locations chosen to receive sensors, this one is the first hit by
# contamination among all those hit by this event.  These will be
# binary in practice (as needed) as long as the sensor placement
# decisions are binary.
#
var FirstSensorForEvent{g in ActiveGoals, a in Events[g], EventSuperLocations[g,a]} >= 0 <= 1; #
#
# Goal variable values
#
var GoalValue {ActiveGMPair};
#
# Variables used to compute the worst-case goal measurement
#
var WorstGoalMeasurement {g in ActiveGoals : (g,"worst") in goalConstraints};
#
# Variables used to compute the Absolute Deviation goal
#
var AbsDevWeightDistribution {1..NumWeightCategories} >=0;

#
# For CVaR we need to know both Value-At-Risk, VaR, and the extent to which VaR
# is exceeded in a given event, y. By definition, VaR <= CVaR and, for small
# Gamma ( < 50% ), VaR >= expected value.  Note, we need these variables for
# each goal/cvar pair.
#
var VaR {g in ActiveGoals : (g,"cvar") in ActiveGMPair};
var y {g in ActiveGoals, a in Events[g] : (g,"cvar") in ActiveGMPair} >= 0;

##
## OBJECTIVE
##
minimize Objective:
	GoalValue[objectiveGoal,objectiveMeasure];

##
## Set the fixed locations
##
subject to FixedSensors{l in FixedSensorLocations}:
	s[l] = 1;
	
##
## For each event, one sensor is tripped first (this could be the dummy, indicating no real sensor
## detects the event
##
subject to pickFirstSensor {g in ActiveGoals, a in Events[g]}:
        sum {L in EventSuperLocations[g,a]} FirstSensorForEvent[g,a,L] = 1;

##
## A sensor on a real location can't be the first tripped if that location isn't selected
##
subject to onlyPickReal {g in ActiveGoals, a in Events[g], L in EventSuperLocations[g,a] : dummyLocation not in SuperLocationMembers[L]}:
        FirstSensorForEvent[g,a,L] <= sum {l in SuperLocationMembers[L]} s[l];

##
## Mean Goal Value
##
subject to meanGoals {(g,m) in ActiveGMPair : m == "mean"}:
	GoalValue[g,m] = sum{a in Events[g]} EventWeight[g,a] * 
	   			sum{L in EventSuperLocations[g,a]} 
					GoalMeasurement[g,a,WorstSuperLocationMember[g,a,L]]*FirstSensorForEvent[g,a,L];

##
## Worst Goal Value
##
subject to worstGoals {(g,m) in ActiveGMPair diff {("ns","total"), ("cost","total"), ("awd","total")}, a in Events[g] : m == "worst"}:
	sum{L in EventSuperLocations[g,a]} GoalMeasurement[g,a,WorstSuperLocationMember[g,a,L]]*FirstSensorForEvent[g,a,L] <= GoalValue[g,m];

##
## total ns Goal Value
##
subject to nsGoal:
	if (("ns","total") in ActiveGMPair)
		then GoalValue["ns","total"] 
		else 0.0
	>= 
	if (("ns","total") in ActiveGMPair)
		then sum{l in FeasibleLocations} s[l]
		else 0.0;

##
## total cost Goal Value
##
subject to costGoal:
	if (("cost","total") in ActiveGMPair)
		then GoalValue["cost","total"]
		else sum{l in FeasibleLocations} PlacementCost[l]
	>= sum{l in FeasibleLocations} PlacementCost[l]*s[l];

##
## Per-location absolute deviation of weight distribution
##
subject to awdCalc1{i in 1..NumWeightCategories}:
	if (("ns","total") in goalConstraints)
	   then AbsDevWeightDistribution[i] 
	   else 0.0
	>=
	if (("ns","total") in goalConstraints)
	   then TargetWeightDistribution[i] - (sum{l in FeasibleLocations}s[l]*LocationWeightDistribution[l,i])/GoalBound["ns","total"]
	   else 0.0;

subject to awdCalc2{i in 1..NumWeightCategories}:
	if (("ns","total") in goalConstraints)
	   then AbsDevWeightDistribution[i]
	   else 0.0
	>=
	if (("ns","total") in goalConstraints)
	   then - TargetWeightDistribution[i] + (sum{l in FeasibleLocations}s[l]*LocationWeightDistribution[l,i])/GoalBound["ns","total"]
	   else 0.0;

##
## total awd Goal Value
##
subject to awdGoal:
	if (("awd","total") in ActiveGMPair)
	   then GoalValue["awd","total"]
	   else 0.0
	>=
	if (("awd","total") in ActiveGMPair)
 	   then sum{i in 1..NumWeightCategories} AbsDevWeightDistribution[i]
	   else 0.0;

##
## CVaR Goal Value
##
subject to cvarGoals {(g,m) in ActiveGMPair : m == "cvar"}:
	GoalValue[g,m] = VaR[g] + (1.0/gamma) * 
			sum{a in Events[g]} (EventWeight[g,a] * y[g,a]);

##
## Ylimit - set y to the max of excess consumption above VaR or 0.
##
subject to yplus {g in ActiveGoals, a in Events[g] : (g,"cvar") in ActiveGMPair}:
	y[g,a] >= sum {L in EventSuperLocations[g,a]} 
    			GoalMeasurement[g,a,WorstSuperLocationMember[g,a,L]]*
			FirstSensorForEvent[g,a,L] - VaR[g];

##
## Constraint limits
##
subject to goalBounds {(g,m) in goalConstraints}:
	GoalValue[g,m] <= GoalBound[g,m];

# PICO SYMBOL: numNetworkLocations numEvents GoalMeasurement GoalBound
