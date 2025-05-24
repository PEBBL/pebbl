# Python script for minimal confidence testing of a PEBBL build


from __future__ import print_function   # Fix for Python 2.7

import subprocess
import sys
import os



def patternMatch(inputString, pattern, outputList) :
	tokens = inputString.split()
	output = []
	for matchToken in pattern :
		if len(tokens) == 0 :
			return False
		token = tokens.pop(0)
		if matchToken == None :
			try :
				val = float(token)
				outputList.append(val)
			except ValueError :
				return False
		elif token != matchToken :
			return False
	return True


def genericTest(command, objective) :
	objPattern = ["Best", "Solution:",  "Value", "=", None]
	commandResultFile = open("temp.log","w")
	subprocess.call(command, stdout=commandResultFile, stderr=commandResultFile)
	commandResultFile.close()
	readBackIn = open("temp.log","r")
	for line in readBackIn :
		objList = []
		if patternMatch(line, objPattern, objList) :
			objFound = objList[0]
			denom = max(abs(objFound), abs(objective))
			if denom == 0 :
				denom == 1
			diff = abs(objective - objective)/denom
			readBackIn.close()
			if diff <= 1e-7 :
				os.remove("temp.log")
				return True
			else :
				print("\nError: found objective " + str(objFound) +
					  ", which should be " + str(objFound), end="", file = sys.stderr)
				break
	print("\nError -- tests not passed, output in temp.log", file = sys.stderr)
	exit(1)


def runTests(execName, datDirName, suffix, testList, 
	         minProcs, maxProcs, addFac, multFac, mpiCommand) :
	execPath = "./src/pebbl/example/" + execName
	dataPath = "../pebbl/data/" + datDirName + "/"
	for test in testList :
		instanceName = test[0]
		instancePath = dataPath + instanceName + suffix
		minPforInstance = test[1]
		maxPforInstance = test[2]
		objective = test[3]
		p = minProcs
		anyRunsYet = False
		while p <= maxProcs :
			if p >= minPforInstance and p <= maxPforInstance :
				if not anyRunsYet :
					print(instanceName, end=":")
					anyRunsYet = True
				print(" " + str(p), end="")
				sys.stdout.flush()
				if p == 1 :
					command  = [execPath, instancePath]
				else :
					command = [mpiCommand,"-np",str(p),execPath,instancePath]
				genericTest(command, objective)
			p = multFac*p + addFac
		if anyRunsYet :
			print()


# Start of main program

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-p", "--minprocs", dest="minprocs", type="int", default=1,
                  help="Minimum number of MPI processes", metavar="PROCS")
parser.add_option("-P", "--maxprocs", dest="maxprocs", type="int", default=32,
                  help="Maximum number of MPI processes", metavar="PROCS")
parser.add_option("-a", "--add", dest="add", type="int", default=0,
                  help="Additive processor count step", metavar="A")
parser.add_option("-m", "--multiply", dest="multiply", type="int", default=2,
                  help="Multiplicative processor count step", metavar="M")
parser.add_option("-c", "--mpicommand", dest="command", type="str", default="mpirun",
	              help="Command used to launch MPI", metavar="CMD")
(options, args) = parser.parse_args()

minProcs   = int(options.minprocs)
maxProcs   = int(options.maxprocs)
addFac     = int(options.add)
multFac    = int(options.multiply)
mpiCommand = options.command

garbage    = []
mpiPattern = ["#define", "ACRO_HAVE_MPI", "1"]

foundParallel = False
try :
	configFile = open("src/pebbl_config.h","r")
except :
	print("No configuration file found -- make sure that the working directory",
		  file = sys.stderr)
	print("is the root of a PEBBL build directory, and that you have already",
		  file = sys.stderr)
	print("configured with cmake-gui, ccmake, or cmake, and performed a",
		  file = sys.stderr)
	print("successful make.", file = sys.stderr)
	exit(1)

for line in configFile :
	if patternMatch(line, mpiPattern, garbage) :
		foundParallel = True

configFile.close()

if (not foundParallel) and (maxProcs > 1) :
	print("Only a serial configuration found -- maxprocs reset to 1.")
	maxProcs = 1

if (minProcs > maxProcs) :
	print("Error: miniumum number of processors (" + str(minProcs) +
		  ") exceeds maximum number of processors (" + 
		  str(maxProcs) + ")", file = sys.stderr)
	exit(1)

if multFac < 1 :
	print("Error: multiply must be at least 1", file = sys.stderr)
	exit(1)

if addFac < 0 :
	print("Error: add cannot be negative", file = sys.stderr)
	exit(1)

if multFac + addFac < 2 :
	print("Error: multiply + add must be at least 2", file = sys.stderr)
	exit(1)

knapsackTests = [ ["animals.1",        1,  8,    157.0],
                  ["animals.2",        1,  8,    152.0],
                  ["hard24",           4, 64,     24.0],
                  ["scor1k.3",         1, 32,   1441.0],
                  ["test-data.1000.1", 1,  8, 273709.0],
                  ["v24",              4, 64,   24.009],
                  ["v24b",             4, 64,    26.22] ]

print("Knapsack tests --")
runTests("knapsack", "knapsack", "", knapsackTests, 
	     minProcs, maxProcs, addFac, multFac, mpiCommand)

monomTests = [ ["testdata",                         1,  2,  0.55555555555],
               ["cmc.data.ss.bin",                 16, 32,  0.26205023761],
               ["pima-indians-diabetes.ss.33.bin",  1,  8,  0.39713541667] ]

print("Monomial tests --")
runTests("monomial", "monomial", ".txt", monomTests,  
	     minProcs, maxProcs, addFac, multFac, mpiCommand)

print("Tests passed")

exit(0)
