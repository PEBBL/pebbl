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
// pbOutput.cpp
//
// Parallel branching class code for PEBBL -- code dealing with output to
// stdout, log files, and so forth.
//
// Jonathan Eckstein
//


#include <pebbl_config.h>
#include <pebbl/utilib/CharString.h>
#include <pebbl/utilib/mpiUtil.h>
#include <pebbl/utilib/nicePrint.h>
#include <pebbl/pbb/parBranching.h>

#ifdef ACRO_HAVE_MPI

using namespace std;
using namespace utilib;

namespace pebbl {


void parallelBranching::printSolValue(std::ostream& stream)
{
  if (iDoIO())
    {
      int taggingWasActive = CommonIO::tagging_active();
      if (taggingWasActive)
	CommonIO::end_tagging();
      stream << endl;
      serialPrintSolValue(stream);
      if (taggingWasActive)
	CommonIO::begin_tagging();
    }
}


solution* parallelBranching::getSolution(int whichProcessor)
{
  // If no solution exists, just return NULL

  if (incumbentSource == MPI_ANY_SOURCE)
    return NULL;

  if (myRank() == incumbentSource)
    {
      if (whichProcessor == myRank())
         return incumbent->incrementRefs();

      PackBuffer solBuf;
      incumbent->pack(solBuf);
      DEBUGPR(120,ucout << "Packed solution size: " << solBuf.size() << endl);
      if (whichProcessor == allProcessors)
	{
	  int bytes = solBuf.size();
	  broadcast(&bytes,1,MPI_INT,myRank());
	  broadcast((void *) solBuf.buf(),
		    bytes,
		    MPI_PACKED,
		myRank());
          return incumbent->incrementRefs();
	}
      else
        {
	  send((void *) solBuf.buf(),
	       solBuf.size(),
	       MPI_PACKED,
	       whichProcessor,
	       printSolutionTag);
          return NULL;
        }
    }
  else
    {
      UnPackBuffer solBuf(solBufSize);
      if (whichProcessor == allProcessors)
	{
	  int bytes;
	  broadcast(&bytes,1,MPI_INT,incumbentSource);
	  broadcast((void *) solBuf.buf(),
			  solBuf.size(),
			  MPI_PACKED,
			  incumbentSource);
	  solBuf.reset(bytes);
	}
      else if (whichProcessor != myRank())
	return NULL;
      else
	{
	  MPI_Status status;
	  recv((void *) solBuf.buf(),
	       solBuf.size(),
	       MPI_PACKED,
	       incumbentSource,
	       printSolutionTag,
	       &status);
	  solBuf.reset(&status);
	}
      solOutputMessages++;
      return unpackSolution(solBuf);
    }
}


void parallelBranching::printSolution(const char* header,
				      const char* footer,
				      ostream& outStream)
{
  DEBUGPR(120,ucout << "parallelBranching::printSolution(): "
	  << "rank = " << myRank() 
	  << ", incumbentValue=" << incumbentValue 
	  << ", incumbentSource = " << incumbentSource << "\n");

  // If there's nothing to print, bail out...

  int whichProc = myIoProc();

  if (!printSolutionSynch && (incumbentSource != MPI_ANY_SOURCE))
    whichProc = incumbentSource;

  solution* solPtr = getSolution(whichProc);

  // Now print the solution 

  if (myRank() == whichProc)
    {
      int taggingWasActive = CommonIO::tagging_active();
      if (taggingWasActive)
	CommonIO::end_tagging();
      if (solPtr)
	{
	  int oldPrecision = outStream.precision();
	  outStream.precision(statusLinePrecision);
	  outStream << header;
	  solPtr->print(outStream);
	  solPtr->dispose();
	  outStream << footer;
	  outStream.precision(oldPrecision);
	}
      else 
	outStream << "No solution identified.\n";
      outStream.flush();
      if (taggingWasActive)
	CommonIO::begin_tagging();
    }

  DEBUGPR(1,ucout << '\n');

}


void parallelBranching::solutionToFile()
{
  double startTime = CPUSeconds();
  double startWCTime = WallClockSeconds();

  ostream* outStreamP = NULL;

  if (enumerating)
    {
      if (iDoIO())
	outStreamP = openSolutionFile();
      printRepository(outStreamP);         // Special routine for enumeration
    }
  else                                     // Normal execution
    {
      // If we already put the solution out, there's nothing to do

      int flag = false;
      if (iAmFirstHub() &&
	  (earlyOutputMinutes > 0) && 
	  (lastSolValOutput == incumbentValue))
      flag = true;
      broadcast((void*) &flag,1,MPI_INT,firstHub());
      solOutputMessages += (myRank() > 0);

      if (!flag)
	{
	  // Otherwise, set up a stream and print to it.

	  if ((printSolutionSynch && iDoIO()) ||
	      (!printSolutionSynch && myRank() == incumbentSource))
	    outStreamP = openSolutionFile();
	  printSolution("","",*outStreamP);
	}
    }

  if (outStreamP)
    closeSolutionFile(outStreamP);

  solOutputTime += CPUSeconds() - startTime;
  solOutputWCTime += WallClockSeconds() - startWCTime;
}


void parallelBranching::statusLine(loadObject& l, const char* tag) 
{
  LOG_EVENT(3,start,statusPrintLogState);
  branching::statusLine(l,tag);
  ucout << Flush;
  LOG_EVENT(3,end,statusPrintLogState);
}


//  Validation log stuff

ostream* parallelBranching::valLogFile()
{
  if (validateLog)
    {
      char name[32];
      sprintf(name,"val%05d.log",myRank());
      return new ofstream(name,restarted ? ios::app : ios::out);
    }
  else
    return 0;
}


void parallelBranchSub::valLogPackChildPrint()
{
  *vout << "packchild " << pGlobal()->myRank() << ' ' << bGlobal()->probCounter
    << ' ' << bound << ' ';
  valLogWriteID();
  valLogPackChildExtra();
  *vout << endl;
}


void parallelBranchSub::valLogPackPrint()
{
  *vout << "pack ";
  valLogWriteID();
  valLogPackExtra();
  *vout << endl;
}


void parallelBranchSub::valLogUnpackPrint()
{
  *vout << "unpack ";
  valLogWriteID(' ');
  *vout << childrenLeft;
  valLogUnpackExtra();
  *vout << endl;
}


// Load log stuff


parLoadLogRecord::parLoadLogRecord(int sense) :
  loadLogRecord(sense),
  hubPool(0),
  serverPool(0),
  gLoad(0),
  cLoad(0),
  releases(0),
  dispatch(0),
  reception(0),
  rebal(0),
  lbRounds(0),
  globalBound(-sense*MAXDOUBLE),
  clusterBound(-sense*MAXDOUBLE),
  hubBound(-sense*MAXDOUBLE),
  serverBound(-sense*MAXDOUBLE)
{ }


size_type parLoadLogRecord::packSize()
{
  parLoadLogRecord dummyRecord(1);
  PackBuffer dummyBuffer;
  dummyRecord.pack(dummyBuffer);
  return dummyBuffer.size();
}


void parLoadLogRecord::pack(PackBuffer& pb)
{
  loadLogRecord::pack(pb);
  pb << hubPool
     << serverPool
     << gLoad
     << cLoad
     << releases
     << dispatch
     << reception
     << rebal
     << lbRounds
     << globalBound
     << clusterBound
     << hubBound
     << serverBound;
}


void parLoadLogRecord::unpack(UnPackBuffer& upb)
{
  loadLogRecord::unpack(upb);
  upb >> hubPool
      >> serverPool
      >> gLoad
      >> cLoad
      >> releases
      >> dispatch
      >> reception
      >> rebal
      >> lbRounds
      >> globalBound
      >> clusterBound
      >> hubBound
      >> serverBound;
}


void parallelBranching::startLoadLogIfNeeded()
{
  pLastLog = new parLoadLogRecord(sense);
  lastLog  = pLastLog;
  haveLLToken = true;
  needLLAppend = (myRank() > 0);
  beginLoadLog();
}


void parallelBranching::recordLoadLogData(double time)
{
  bool recordingBounds = !(depthFirst || breadthFirst);

  size_type poolSize = 0;
  double    wBound = -sense*MAXDOUBLE;

  if (iAmWorker())
    {
      poolSize = workerPool->size();
      wBound = updatedPLoad().aggregateBound;
    }

  parLoadLogRecord* record = new parLoadLogRecord(sense);

  if (iAmHub())
    {
      record->hubPool = hubPool->size();

      record->dispatch   = messages.hubDispatch.sent - pLastLog->dispatch;
      pLastLog->dispatch = messages.hubDispatch.sent;

      record->lbRounds   = loadBalancer->numRounds() - pLastLog->lbRounds;
      pLastLog->lbRounds = loadBalancer->numRounds();
    }

  if (iAmWorker())
    {
      record->serverPool = serverPool.size();

      record->reception   = messages.hubDispatch.received - pLastLog->reception;
      pLastLog->reception = messages.hubDispatch.received;

      record->rebal   = rebalanceCount - pLastLog->rebal;
      pLastLog->rebal = rebalanceCount;
    }

  record->gLoad = globalLoad.count();

  record->cLoad = clusterLoad.count();

  record->releases   = spReleaseCount - pLastLog->releases;
  pLastLog->releases = spReleaseCount;

  if (recordingBounds)
    {
      record->globalBound  = globalLoad.aggregateBound;
      record->clusterBound = clusterLoad.aggregateBound;
    }

  if (iAmHub() && recordingBounds)
    record->hubBound = hubPool->updatedLoad().aggregateBound;

  if (iAmWorker() && recordingBounds)
    record->serverBound = serverPool.updatedLoad().aggregateBound;

  recordSerialLoadData(record,time,poolSize,wBound);

  // If this is processor 0 and it's time to write a chunk of the load
  // log, write, and then start passing the token around the ring so
  // everybody else writes in sequence.

  if ((myRank() == 0) && needToWriteLoadLog(time) && haveLLToken)
    {
      lastLLWriteTime = time;
      writeLoadLogPassToken();
    }
}


// Generic code chunk for writing directly to the load log from any processor

void parallelBranching::loadLogSMPWrite()
{
  if (loadLogEntries.empty())
    return;
  ofstream loadLogFile(loadLogFileName().c_str(),
		       needLLAppend ? ios::app : ios::out);
  if (loadLogFile.bad())
    ucout << "*** Warning *** could not open load log file.\n";
  else
    branching::writeLoadLog(loadLogFile,myRank());
}


//  Write to load log and pass token down the ring to tell next
//  processor in the chain to do likewise

void parallelBranching::writeLoadLogPassToken()
{
  loadLogSMPWrite();
  if (mySize() == 1)
    return;
  isend((void*) &haveLLToken,           // Dummy buffer
	0,                              // No data except tag
        MPI_BYTE,                       // Datatype really doesn't matter
        (myRank() + 1) % mySize(),  // Next processor in ring
        llTokenTag,
        &llSendRequest);
  haveLLToken = false;          // Don't write again until the token comes back
  messages.general.sent++;
}


//  Gets executed when the load log token message is received

void parallelBranching::receiveLLToken()
{
  // If we sent a token at some time in the past, make sure that send is done.
  if (!haveLLToken)
    {
      int flag;
      uMPI::test(&llSendRequest,&flag,MPI_STATUS_IGNORE);
      if (!flag)
	ucout << "*** Warning *** incomplete LL Token send.\n";
    }
  haveLLToken = true;
  if (myRank() > 0)               // Pass around rings stops at processor 0
    writeLoadLogPassToken();        // Otherwise, write log and pass token
  recordMessageReceived(llChainer);
}


//  Write load log -- this version is called at the end of the run

void parallelBranching::writeLoadLog()
{
  // See if we can get away with something simple -- either loadLogSMP
  // or writing the file in chunks, which implies each processor has
  // direct file system access.
  if (loadLogSMP || (loadLogWriteSeconds > 0))
    for (int p=0; p<mySize(); p++)          // Simple serial output to
      {				              // same file
	barrier();
	if (myRank() == p)
	  loadLogSMPWrite();
      }
  else           // Either no common file system or no common clock
    {		 // (or both)

      // Max size for buffers etc.
      int maxLength = maxReduce((int) loadLogEntries.size(), myIoProc());

      MPI_Status status;

      //  This is what the main processor does

      if (iDoIO())
	{
	  // Write own stuff, but don't let file get closed.
	  // This cannot be called if load log is written in chunks,
          // so just open in overwrite mode.
	  ofstream loadLogFile(loadLogFileName().c_str(), ios::out);
	  if (loadLogFile.bad())
	    ucout << "*** Warning *** could not open load log file.\n";
	  else
	    branching::writeLoadLog(loadLogFile,myRank());

	  UnPackBuffer llBuf(maxLength*parLoadLogRecord::packSize());

	  // Loop over all other processors

	  for (int p=0; p<mySize(); p++)
	    if (p != myRank())
	      {
		// Determine clock offset

		double clockOffset  = 0;
		double shortestTurn = MAXDOUBLE;

		for (int i=0; i<loadLogClockSyncs; i++)
		  {
		    double theirTime = 0;
		    double startTime = WallClockSeconds();
		    send(&theirTime,0,MPI_DOUBLE,p,llSyncOutTag);
		    recv(&theirTime,1,MPI_DOUBLE,p,llSyncBackTag,&status);
		    double roundTrip = WallClockSeconds() - startTime;
		    if (roundTrip < shortestTurn)
		      {
			shortestTurn = roundTrip;
			clockOffset = theirTime - (startTime + roundTrip/2);
		      }
		  }

		// ucout << "Clock skew for processor " << p << " is "
		// 	<< clockOffset << " seconds.\n";

		// Suck in the data from the other processor; 

		recv((void *) llBuf.buf(),
		     llBuf.size(),
		     MPI_PACKED,
		     p,
		     llDataTag,
		     &status);
		llBuf.reset(&status);

		parLoadLogRecord record(sense);
		while(llBuf.data_remaining())
		  {
		    record.unpack(llBuf);
		    record.writeToStream(loadLogFile,
					 sense,
					 loadLogBaseTime - clockOffset,
					 p);
		  }
	      }
	}
      else     // This is what other processors do...
	{
	  // Respond to clock skew calculation

	  double localTime = 0;

	  for (int i=0; i<loadLogClockSyncs; i++)
	    {
	      recv(&localTime,0,MPI_DOUBLE,myIoProc(),llSyncOutTag,&status);
	      localTime = WallClockSeconds();
	      send(&localTime,1,MPI_DOUBLE,myIoProc(),llSyncBackTag);
	    }

	  // Send data

	  PackBuffer llBuf;
	  while(!loadLogEntries.empty())
	    {
	      loadLogRecord* record = NULL;
	      loadLogEntries.remove(record);
	      record->pack(llBuf);
	      delete record;
	    }

	  send((void *) llBuf.buf(),
	       llBuf.size(),
	       MPI_PACKED,
	       myIoProc(),
	       llDataTag);
	}
    }
}


// Guts of writing a chunk of the load log file.

void parLoadLogRecord::writeToStream(ostream& os,
				     int      sense,
				     double   baseTime,
				     int      proc)
{
  os << proc << '\t'              // Processor
     << time - baseTime << '\t'   // Time
     << boundCalls << '\t'        // Bound calls
     << pool << '\t'              // (Worker) pool size
     << hubPool << '\t'           // Hub pool size
     << pool + hubPool << '\t'    // Total pool size
     << serverPool << '\t'        // Server pool size
     << gLoad << '\t'             // Global load est
     << cLoad << '\t'             // Cluster load est
     << releases << '\t'          // SP releases
     << bound << '\t'             // Bound
     << incVal << '\t'            // Incumbent value
     << globalBound << '\t'       // Global bound
     << clusterBound << '\t'      // Cluster bound
     << hubBound << '\t'          // Bound of hub pool
     << serverBound << '\t'       // Bound or server pool
     << offers << '\t'            // Sols offered to repos
     << admits << '\t'            // Sols admitted
     << dispatch << '\t'          // Problems dispatched from hubs
     << reception << '\t'         // Received at worker
     << rebal << '\t'             // Rebalanced from worker to hub
     << lbRounds << '\n';         // Number of load balancing rounds
}


//  Printout of processor configuration.

void parallelBranching::printConfiguration(ostream& stream)
{
  if (!iDoIO())
    return;
  CommonIO::end_tagging();
  stream << "\nPEBBL Configuration:\n";
  hyphens(stream,19) << '\n';
  int pWidth = digitsNeededFor(mySize());
  stream.width(pWidth);
  stream << numHubs() << " cluster" << plural(numHubs());
  int mod  = mySize() % cluster.typicalSize;
  if (mod)
    stream << ": " << numHubs() - 1;
  stream << " of size " << cluster.typicalSize;
  if (mod)
    stream << ", 1 of size " << mod;
  stream << '\n';
  stream.width(pWidth);
  stream << mySize() << " processor" << plural(mySize()) << '\n';
  int pureHubs    = mySize() - totalWorkers();
  int workerHubs  = numHubs() - pureHubs;
  int pureWorkers = totalWorkers() - workerHubs;
  configLine(stream,pWidth,pureWorkers,"pure worker");
  configLine(stream,pWidth,pureHubs,   "pure hub");
  configLine(stream,pWidth,workerHubs, "worker-hub");
  stream << "\nTarget timeslice: " << timeSlice << " second";
  if (timeSlice != 1)
    stream << 's';
  stream << ".\n\n";
  CommonIO::begin_tagging();
}


void parallelBranching::configLine(ostream& stream,
				   int      pWidth,
				   int      number,
				   const char*    kind)
{
  if (number == 0)
    return;
  int padding = 13 - strlen(kind);
  stream.width(pWidth);
  stream << number << ' ' << kind;
  stream.width(1);
  stream << plural(number);
  stream.width(padding);
  stream << "(";
  printPercent(stream,number,mySize()) << ")\n";
}


void parallelBranching::printSPStatistics(ostream& stream)
{
  CommonIO::end_tagging();
  int combinedTable[numStates];
  reduce(subCount,combinedTable,numStates,MPI_INT,MPI_SUM,myIoProc());
  spTotal = combinedTable[boundable];
  int totalUnderHubControl = sumReduce(hubHandledCount);
  int totalDelivered       = sumReduce(spDeliverCount);
  int totalReleased        = sumReduce(spReleaseCount);
  int totalRebalanced      = sumReduce(rebalanceSPCount);
  int totalLoadBalanced    = 0;
  if (numHubs() > 1)
    totalLoadBalanced = sumReduce(loadBalSPCount);

  if (iDoIO())
    {
      const char* hubControlString   = "Tokens at Hubs";
      const char* scatterString      = "Scattered to Hubs";
      const char* rebalanceString    = "Rebalanced to Local Hub";
      const char* nlScatterString    = "Scattered to Nonlocal Hubs";
      if (numHubs() == 1)
	{
	  hubControlString   = "Tokens at Hub";
	  scatterString      = "Scattered to Hub";
	  rebalanceString    = "Rebalanced to Hub";
	  nlScatterString    = "";
	}

      const char* rampBoundString = "Bounded during ramp up";
      const char* rampPoolString = "In pool at end of ramp up";

      if (restarted)
	{
	  rampBoundString = "";
	  rampPoolString  = "Loaded from checkpoint";
	}

      int fieldWidth = strlen(rebalanceString);
      fieldWidth = std::max(fieldWidth,(int) strlen(rampBoundString));
      fieldWidth = std::max(fieldWidth,(int) strlen(rampPoolString));
      fieldWidth = std::max(fieldWidth,(int) strlen(nlScatterString));

      int numWidth = digitsNeededFor(max(spTotal,rampUpPool));

      printSPTable(stream,combinedTable,fieldWidth);
      if (!restarted)
	printSPLine(stream,
		    rampUpBounds,
		    rampBoundString,
		    spTotal,
		    fieldWidth,
		    numWidth);
      printSPLine(stream,
		  rampUpPool,
		  rampPoolString,
		  spTotal,
		  fieldWidth,
		  numWidth);
      printSPLine(stream,
		  totalUnderHubControl,
		  hubControlString,
		  spTotal,
		  fieldWidth,
		  numWidth);
      printSPLine(stream,
		  totalReleased - totalRebalanced,
		  scatterString,
		  spTotal,
		  fieldWidth,
		  numWidth);
      printSPLine(stream,
		  totalRebalanced,
		  rebalanceString,
		  spTotal,
		  fieldWidth,
		  numWidth);
      printSPLine(stream,
		  globalLoad.messages.hubDispatch.sent,
		  "Dispatched from a Hub",
		  spTotal,
		  fieldWidth,
		  numWidth);
      printSPLine(stream,
		  totalDelivered,
		  "Moved between Workers",
		  spTotal,
		  fieldWidth,
		  numWidth);

      if (numHubs() > 1)
	{
	  printSPLine(stream,
		      globalLoad.messages.nonLocalScatter.sent,
		      "Scattered to Nonlocal Hubs",
		      spTotal,
		      fieldWidth,
		      numWidth);
	  printSPLine(stream,
		      totalLoadBalanced,
		      "Balanced between Hubs",
		      spTotal,
		      fieldWidth,
		      numWidth);
	}

      stream << '\n';
      CommonIO::begin_tagging();
    }
}



void parallelBranching::printTimings(ostream& stream)
{
  if (printSpTimes)
    { 
      for (int i=0; i<mySize(); i++)
	{
	  if (i == myRank())
	    {
	      printSpTimeStats(stream);
	      if (i == mySize() - 1)
		stream << endl;
	      stream << Flush;
	    }
	  barrier();
	}
    }

  CommonIO::end_tagging();

  int oldPrecision = stream.precision(1);
  stream.setf(ios::fixed,ios::floatfield);

  timingPrintNameWidth = strlen("Thread/Function");
  if (mySize() > 1) 
    {
      int tmp = strlen("Problem Broadcast");
      timingPrintNameWidth = max(timingPrintNameWidth,tmp);
    }
  {
    int tmp = strlen(preprocessPhaseName());
    timingPrintNameWidth = max(timingPrintNameWidth, tmp);
  }

  string rampUpString = "Ramp-up";
  if (restarted)
    rampUpString = "Loading checkpoint";
  timingPrintNameWidth = max(timingPrintNameWidth,(int) rampUpString.size());

  messagesReceivedThisProcessor =   broadcastMessageCount 
                                  + preprocessMessages 
                                  + rampUpMessages 
                                  + solOutputMessages;

  parBranchingThreadObj* thread;
  double overhead = 0;
  ListItem<parBranchingThreadObj*> *l1;

  for(l1=threadsList.head(); l1; l1=threadsList.next(l1) )
    {
      thread = l1->data();
      overhead += thread->overheadTime();
      if (sumReduce(thread->active) > 0)
	{
	  int l = strlen(thread->name);
	  if (l > timingPrintNameWidth)
	    timingPrintNameWidth = l;
	}
      messagesReceivedThisProcessor += thread->messageCount();      
    }

  double idleProcTime;
  double procThreadTime;
  sched.timing(searchTime,idleProcTime,procThreadTime);
  double schedProcTime = searchTime - procThreadTime - idleProcTime;
  if (schedProcTime < 0)
    schedProcTime = 0;
  searchTime += broadcastTime + preprocessTime + rampUpTime + solOutputTime;
  searchWCTime +=   broadcastWCTime + preprocessWCTime 
                  + rampUpWCTime + solOutputWCTime;

  totalCPU = sumReduce(searchTime);
  maxCPU   = maxReduce(searchTime);
  totalWC  = sumReduce(searchWCTime);
  maxWC    = maxReduce(searchWCTime);

  totalMessages = sumReduce(messagesReceivedThisProcessor);

  timingPrintTimeWidth    = max(7,digitsNeededFor(maxCPU) + 2);
  timingPrintWCTimeWidth  = max(7,digitsNeededFor(maxWC) + 2);
  timingPrintPNWidth      = max(6,digitsNeededFor(mySize()));
  timingPrintMessageWidth = max(8,digitsNeededFor(totalMessages));

  if (iDoIO())
    {
      // CAP: Some of these streamwidth settings appear to be redundant, but I'm
      // just following the model from before to be safe.
      stream << "Average search time (CPU)        ";
      stream.width(timingPrintTimeWidth);
      stream << totalCPU/mySize() << " seconds.\n";
      stream << "Maximum search time (CPU)        ";
      stream.width(timingPrintTimeWidth);
      stream << maxCPU << " seconds.\n";
      stream << "Average search time (Wall clock) ";
      stream.width(timingPrintWCTimeWidth);
      stream << totalWC/mySize() << " seconds.\n";
      stream << "Maximum search time (Wall clock) ";
      stream.width(timingPrintWCTimeWidth);
      stream << maxWC << " seconds.\n";

      stream << '\n';

      if (numHubs() > 1)
	{
	  int r = loadBalancer->numRounds();
	  int s = loadBalancer->numSurveyRestarts();
	  stream << r << " load balancing round" << plural(r) << ", "
	    << s << " survey restart" << plural(s) << ".\n";
	}

      {
	int qp = loadBalancer->numQuiescencePolls();
	int tc = loadBalancer->numTermChecks();
	stream << qp << " quiescence poll" << plural(qp) << ", "
	       << tc << " termination check" << plural(tc) << ".\n\n";
      }

      timingPrintText(stream,' ',' ',
		      "","","Average",
		      "% of","% where","",
		      "Messages","% of");
      timingPrintText(stream,' ',' ',
		      "Thread/Function","Number","Seconds",
		      "Total","Active","COV",
		      "Received","Msgs");
      timingPrintText(stream);
    }

  if (combineTimingsMax)
    totalCPU = mySize()*maxCPU;

  timingPrintData(stream,
		  "Problem Broadcast",
		  mySize() > 1,
		  broadcastTime,
		  broadcastMessageCount);

  timingPrintData(stream,
		  preprocessPhaseName(),
		  true,
		  preprocessTime,
		  preprocessMessages);
  
  timingPrintData(stream,
		  rampUpString.c_str(),
		  true,
		  rampUpTime,
		  rampUpMessages);
  
 for (l1=threadsList.head(); l1;  l1=threadsList.next(l1))
   {
      thread = l1->data();
      timingPrintData(stream,
		      thread->name,
		      thread->active,
		      thread->usefulTime(),
		      thread->messageCount());
       }

  timingPrintData(stream,
		  "Scheduler",
		  true,
		  schedProcTime,
		  0);

 timingPrintData(stream,
		 "Solution Output",
		 true,
		 solOutputTime,
		 solOutputMessages);

  timingPrintData(stream,
		  "Other Overhead",
		  overhead > 0,
		  overhead,
		  0);

  timingPrintData(stream,
		  "Idle",
		  true,
		  idleProcTime,
		  0);
  
  if (iDoIO())
    timingPrintText(stream,'-',' ');

  timingPrintData(stream,
		  "Total",
		  true,
		  searchTime,
		  messagesReceivedThisProcessor);

  if (iDoIO()) 
    {
      if (checkpointsEnabled)
	{
	  int cpsWritten = checkpointNumber - restartCPNum;
	  stream << endl << cpsWritten 
		 << " checkpoint" << plural(cpsWritten)
		 << " written, consuming " << checkpointTotalTime 
		 << " seconds.\n";
	}
      stream.setf(ios::fixed,ios::floatfield);
      stream.precision(2);
      stream << "\nMessages per subproblem: " 
	     << ((double) totalMessages)/((double) spTotal) 
	     << '\n';
      stream.precision(0);
      stream << "Hub loading factor: ";
      if (numHubs() > 1)
	stream << "typical " << 100*typicalHubBusyFraction << "%, last ";
      stream << 100*lastHubBusyFraction << "%\n\n";
    }

  stream.precision(oldPrecision);
  stream.unsetf(ios::floatfield);

  CommonIO::begin_tagging();
}


void parallelBranching::timingPrintText(ostream& stream,
					char  percentTotalFill,
					char  restFill,
					const char* text1,
					const char* text2,
					const char* text3,
					const char* text4,
					const char* text5,
					const char* text6,
					const char* text7,
					const char* text8)
{
  stream.fill(restFill);

  stream.setf(ios::left,ios::adjustfield);
  stream.width(timingPrintNameWidth);
  stream << text1 << ' ';
  stream.setf(ios::right,ios::adjustfield);

  stream.width(timingPrintPNWidth);
  stream << text2 << ' ';

  stream.width(timingPrintTimeWidth);
  stream << text3 << ' ';
  
  stream.fill(percentTotalFill);

  stream.width(6);
  stream << text4 << ' ';

  stream.fill(restFill);

  stream.width(7);
  stream << text5 << ' ';

  stream.width(5);
  stream << text6 << ' ';

  stream.fill(percentTotalFill);

  stream.width(timingPrintMessageWidth);
  stream << text7 << ' ';

  stream.width(6);
  stream << text8 << '\n';

  stream.fill(' ');
}


void parallelBranching::timingPrintData(ostream& stream,
					const char* name,
					int present,
					double time,
					double messageCount)
{
  int count = sumReduce(present);
  double timeSum = sumReduce(time);
  double timeSumSq = sumReduce(time*time);
  double baseLine;
  if (combineTimingsMax)
    baseLine = count*maxCPU;
  else
    baseLine = sumReduce(present*searchTime);

  double messageSum = sumReduce(messageCount);

  if (!iDoIO() || (count == 0))
    return;

  double mean     = timeSum/count;
  double variance = max((double) 0,timeSumSq/count - mean*mean);
  double stdev    = sqrt(variance);
  double cov;
  if (mean > 0)
    cov = stdev/mean;
  else
    cov = 0;

  stream.setf(ios::left,ios::adjustfield);
  stream.width(timingPrintNameWidth);
  stream << name << ' ';
  stream.setf(ios::right,ios::adjustfield);

  stream.width(timingPrintPNWidth);
  stream << count << ' ';

  stream.width(timingPrintTimeWidth);
  stream.setf(ios::fixed,ios::floatfield);
  stream.precision(1);
  stream << mean << " ";
  
  printPercent(stream,timeSum,totalCPU) << "  ";
  
  printPercent(stream,timeSum,baseLine) << ' ';

  stream.width(5);
  stream.setf(ios::fixed,ios::floatfield);
  stream.precision(2);
  stream << cov << ' ';

  stream.width(timingPrintMessageWidth);
  stream.setf(ios::fixed,ios::floatfield);
  stream.precision(0);
  stream << messageSum << ' ';

  printPercent(stream,messageSum,totalMessages) << '\n';
}


void parallelBranching::postRampUpAbort(double aggBound)
{
  // Figure out time and print some statistics

  double maxRampTime = maxReduce(rampUpTime);
  if (iDoIO())
    {
      CommonIO::end_tagging();
      ucout << "Aborting after ramp-up phase:\n\n";
      ucout.precision(2);
      ucout << "Ramp-up CPU time:             " << maxRampTime  << " seconds\n";
      ucout << "Subproblems bounded:      " << rampUpBounds << endl
	    << "Subproblems left in pool: " << rampUpPool   << "\n\n";
      ucout.precision(statusLinePrecision);
      ucout.width(12);
      if (abs(incumbentValue) > 1e10)
	ucout.setf(ios::scientific,ios::floatfield);
      else 
	ucout.setf(ios::fixed,ios::floatfield);
      ucout << "Incumbent value: " << incumbentValue << endl;
      if (abs(aggBound) > 1e10)
	ucout.setf(ios::scientific,ios::floatfield);
      else 
	ucout.setf(ios::fixed,ios::floatfield);
      ucout << "Aggregate bound: " << aggBound       << endl;
      ucout.width(6);
      ucout.precision(3);
      ucout << "Gap:             " << 100.0*relGap(aggBound) << " %\n\n";
      CommonIO::begin_tagging();
    }

  ucout.flush();
  cleanAbort();
  
}


void spToken::debugPrint(ostream& s) const
{
  coreSPInfo::debugPrint(s);
  s << '-';
  switch (whichChild)
    {
    case self:
      s << "self";
      break;
    case anyChild:
      s << "any-child";
      break;
    default:
      s << "child-" << whichChild;
    }
};


#ifdef EVENT_LOGGING_PRESENT

void parallelBranching::initEventLog()
{
  logEvent::init();
 
  preprocLogState.define("Preproc","yellow");
  workerLogState.define("Worker","green");
  boundLogState.define("Bounding","DarkGreen");
  foundIncLogState.define("New Incumb","magenta");
  pruneLogState.define("Pruning","orange");
  hubPruneLogState.define("Hub Pruning","brown");
  hubLogState.define("Hub","red");
  statusPrintLogState.define("Status Print","white");
  rebalLogState.define("Rebalance","LightBlue");
  pCreateLogState.define("SP Make","PaleGreen");
  pBoundedLogState.define("SP Bounded","black");
  pDeadLogState.define("SP Dead","purple");
  pSplitLogState.define("SP Split","SpringGreen");
  rampUpLogState.define("Ramp Up","gold");
  checkpointLogState.define("Checkpoint","chocolate");
}


#endif



// void parallelBranching::writeCachedOutput()
// {
//   ucout << "Writing cached debug output.\n";
//   *outputCache << '\0';
//   if (outputCache->fail())
//     {
//       ucout << "Debug output has been truncated.\n";
//       outputCacheBuf[outputCacheSize - 1] = '\0';
//     }
//   char name[32];
//   sprintf(name,"debug%05d.txt",myRank());
//   ofstream dump(name,ios::out);
//   dump << outputCacheBuf;
// }


} // namespace pebbl

#endif
