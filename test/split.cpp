#include <mpi.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
using namespace std;


// hubs
// workers
// bounders

// pure hubs should not be assigned bounding processors.
//
// figure out how many processors for a full cluster and allocate that way.
//

// Example test cases:
//
// np = 25
// clusterSize = 5
// processorsPerGroup = 5
// hubsDontWorkSize = 10
// 

// In the case where there is a remainder that does not fit, do we:
//  1) Fill out the PEBBL processors and short their bounding groups
//  2) Short the PEBBL processors but fill out the bounding groups
//
//  Better to give options, but shorting PEBBL processors is a reasonable
//  default. That will minimize the number of underpowered workers.

// some coupling with the hub allocation code: assume that the hub is the
// first index in the group, and that groups are made contiguously wrt. rank

// USAGE: ./split <clusterSize> <boundingGroupSize> <hubsDontWorkSize>
int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	if (argc != 4)
	{
		std::cout << "Bad arguments" << std::endl;
		return 1;
	}
	
	MPI_Comm comm = MPI_COMM_WORLD;
	int clusterSize = strtol(argv[1], NULL, 10);
	int prefBoundSize = strtol(argv[2], NULL, 10);
	int hubsDontWorkSize = strtol(argv[3], NULL, 10);

	int worldRank = -1;
	int worldSize = -1;
	int headRank = -1;
	int headSize = -1;
	int boundRank = -1;
	int boundSize = -1;

	MPI_Comm headComm;
	MPI_Comm boundComm;

	MPI_Comm_rank(comm, &worldRank);
	MPI_Comm_size(comm, &worldSize);

	bool hubsWork = hubsDontWorkSize > clusterSize;
	int fullClusterSize = clusterSize * prefBoundSize - (1 - hubsWork) * (prefBoundSize - 1);
	if (worldRank == 0) 
	{
		std::stringstream s1;
		s1 << "hubsWork: " << hubsWork << " fullClusterSize: " << fullClusterSize << std::endl;
		std::cerr << s1.str();
	}
	int boundGroup = -1;	

	if (hubsWork)
	{
		int isHead = worldRank % prefBoundSize == 0;	
		MPI_Comm_split(comm, isHead, worldRank, &headComm);
		if (isHead)
		{
			MPI_Comm_rank(headComm, &headRank);
			MPI_Comm_size(headComm, &headSize);
		}
		else
		{
			MPI_Comm_free(&headComm);
		}
		boundGroup = worldRank / prefBoundSize;
		MPI_Comm_split(comm, boundGroup, worldRank, &boundComm);

		MPI_Comm_rank(boundComm, &boundRank);
		MPI_Comm_size(boundComm, &boundSize);
	}
	else
	{
		// this is tightly coupled with the decision making strategy
		// pebbl uses to decide how ranks are associated to workers/hubs.
		// Maybe this code shold be moved there, or scrapped to make
		// something that uses Jonathan's forthcoming MPI code
		int isHub = worldRank % fullClusterSize == 0;
		int isHead = ((worldRank % fullClusterSize)) % prefBoundSize == 1;
		int visibleToPebbl = isHub || isHead;
		MPI_Comm_split(comm, visibleToPebbl, worldRank, &headComm);
		if (visibleToPebbl)
		{	
			MPI_Comm_rank(headComm, &headRank);
			MPI_Comm_size(headComm, &headSize);
		}
		else
		{
			MPI_Comm_free(&headComm);	
		}
		
		boundGroup = (worldRank - (worldRank / fullClusterSize + 1)) / prefBoundSize;
		if (worldRank % fullClusterSize == 0 && (worldSize - worldRank > fullClusterSize || worldSize % fullClusterSize <= prefBoundSize * (hubsDontWorkSize - 1)))
		{ // we are a pure hub
			boundGroup = -1;
		}
		
		MPI_Comm_split(comm, boundGroup, worldRank, &boundComm);
		if (boundGroup >= 0)
		{
			MPI_Comm_rank(boundComm, &boundRank);
			MPI_Comm_size(boundComm, &boundSize);
		}
		else
		{
			MPI_Comm_free(&boundComm);
		}
	}

	std::stringstream s;
	s << worldRank << " " << headRank << " " << boundRank << " " << boundGroup << std::endl;
	std::cout << s.str();

	MPI_Finalize();
	return 0;
}
