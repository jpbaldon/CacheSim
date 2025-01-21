#include <iostream>
#include <fstream>
#include <iomanip>

#include "Block.h"
#include "Cache.h"

const int NUM_CACHES = 2;

const std::string listRepPolicies[3] = { "LRU", "FIFO", "Optimal" };
const std::string listIncPolicies[2] = { "non-inclusive", "inclusive" };

int main() {

	int blockSize;  //power of 2

	int size[2];   // total bytes of storage for each cache

	int assoc[2];  // 1 is a direct-mapped cache

	std::string trace;  //file name

	char rw;  //used to read in each line of the input file
	int address;

	int numOpCodes = 0;  //the number of operations provided in the input file

	std::vector<char> readWrite; //stores whether we're reading or writing an address
	std::vector<int> addresses;   //stores addresses
	
	   //Read in the parameters from the console
	std::cout << "Example inputs: 16 1024 2 8192 4 0 0 gcc_trace.txt" << std::endl;
	std::cout << "Enter the blocksize (a power of 2): ";
	std::cin >> blockSize; 
	std::cout << "Enter the size of the L1 cache (a power of 2): ";
	std::cin >> size[0];
	std::cout << "Enter the associativity of the L1 cache (1 is direct-mapped): ";
	std::cin >> assoc[0];
	std::cout << "Enter the size of the L2 cache (a power of 2; 0 indicates no L2 cache): ";
	std::cin >> size[1];
	std::cout << "Enter the associativity of the L2 cache: ";
	std::cin >> assoc[1];
	std::cout << "Enter the replacement policy (0 = LRU, 1 = FIFO, 2 = Optimal): ";
	std::cin >> Cache::repPolicy;
	std::cout << "Enter the inclusion policy (0 = non-inclusive, 1 = inclusive): ";
	std::cin >> Cache::incPolicy;
	std::cout << "Enter the input file name (e.g., compress_trace.txt, gcc_trace.txt, go_trace.txt, perl_trace.txt, vortex_trace.txt): ";
	std::cin >> trace;

	  //L2 might not exist
	if (size[1] == 0)
		Cache::numValidCaches = 1;

	std::ifstream inputFile("input/" + trace);

	  //quit if file invalid
	if (!inputFile.is_open()) {
		std::cout << "Issue opening file " << trace << std::endl;
		return 0;
	}
	
	   //read in and store data from the file
	while (inputFile >> rw) {

		readWrite.push_back(rw);
		
		inputFile >> std::hex >> address;

		addresses.push_back(address);

		numOpCodes++;

	}

	for (int i = 0; i < NUM_CACHES; i++) {

		if (size[i] == 0) {
			Cache nextCache = Cache(1, 1, i, blockSize);
			Cache::cacheSet.push_back(nextCache);
		}
		else {
			Cache nextCache = Cache(size[i], assoc[i], i, blockSize);
			Cache::cacheSet.push_back(nextCache);
		}
	}

	for (int i = 0; i < Cache::numValidCaches; i++) {

		Cache::cacheSet[i].processAddresses(numOpCodes, addresses);

		if (Cache::repPolicy == 2) {
			Cache::cacheSet[i].optimalLookahead(numOpCodes);
		}
	}
	
	   //for each operation
	for (int i = 0; i < numOpCodes; i++) {

		Cache::cacheSet[0].findTag(i, readWrite[i], Cache::cacheSet, i);

		Cache::lruCounter++;

	}

	Cache::cacheSet[0].setMissRate((float)(Cache::cacheSet[0].getReadMisses() + Cache::cacheSet[0].getWriteMisses()) / (float)(Cache::cacheSet[0].getReads() + Cache::cacheSet[0].getWrites()));

	if (Cache::numValidCaches > 1)
		Cache::cacheSet[1].setMissRate(((float)Cache::cacheSet[1].getReadMisses() / (float)Cache::cacheSet[1].getReads()));

	  //Print the simulator parameters
	std::cout << "===== Simulator configuration =====" << std::endl;
	std::cout << "BLOCKSIZE:             " << blockSize << std::endl;
	std::cout << "L1_SIZE:               " << size[0] << std::endl;
	std::cout << "L1_ASSOC:              " << assoc[0] << std::endl;
	std::cout << "L2_SIZE:               " << size[1] << std::endl;
	std::cout << "L2_ASSOC:              " << assoc[1] << std::endl;
	std::cout << "REPLACEMENT POLICY:    " << listRepPolicies[Cache::repPolicy] << std::endl;
	std::cout << "INCLUSION PROPERTY:    " << listIncPolicies[Cache::incPolicy] << std::endl;
	std::cout << "trace_file:            " << trace << std::endl;
	    //Print the cache contents
	for (int k = 0; k < Cache::numValidCaches; k++) {

		std::cout << "===== L" << k + 1 << " contents =====" << std::endl;

		for (int i = 0; i < Cache::cacheSet[k].getNumSets(); i++) {

			std::cout << "Set " << std::dec << i << ":  ";

			for (int j = 0; j < Cache::cacheSet[k].getAssoc(); j++) {

				if (Cache::cacheSet[k].mem[i][j].getValid()) {
					std::cout << std::hex << Cache::cacheSet[k].mem[i][j].getTag();
					if (Cache::cacheSet[k].mem[i][j].getDirty())
						std::cout << " D     ";
					else
						std::cout << "       ";
				}

			}
			std::cout << std::endl;
		}
	}
	    //Print the results of the simulation
	std::cout << "===== Simulation results (raw) =====" << std::endl;
	std::cout << "a. number of L1 reads:          " << std::dec << Cache::cacheSet[0].getReads() << std::endl;
	std::cout << "b. number of L1 read misses:    " << Cache::cacheSet[0].getReadMisses() << std::endl;
	std::cout << "c. number of L1 writes:         " << Cache::cacheSet[0].getWrites() << std::endl;
	std::cout << "d. number of L1 write misses:   " << Cache::cacheSet[0].getWriteMisses() << std::endl;
	std::cout << "e. L1 miss rate:                " << std::fixed << std::setprecision(6) << Cache::cacheSet[0].getMissRate() << std::endl;
	std::cout << "f. number of L1 writebacks:     " << Cache::cacheSet[0].getWriteBacks() << std::endl;
	std::cout << "g. number of L2 reads:          " << Cache::cacheSet[1].getReads() << std::endl;
	std::cout << "h. number of L2 read misses:    " << Cache::cacheSet[1].getReadMisses() << std::endl;
	std::cout << "i. number of L2 writes:         " << Cache::cacheSet[1].getWrites() << std::endl;
	std::cout << "j. number of L2 write misses:   " << Cache::cacheSet[1].getWriteMisses() << std::endl;
	if(Cache::cacheSet[1].getMissRate() == 0)  //avoid showing digits after decimal
		std::cout << "k. L2 miss rate:                " << 0 << std::endl;
	else
		std::cout << "k. L2 miss rate:                " << Cache::cacheSet[1].getMissRate() << std::endl;
	std::cout << "l. number of L2 writebacks:     " << Cache::cacheSet[1].getWriteBacks() << std::endl;
	std::cout << "m. total memory traffic:        " << Cache::memTraffic << std::endl;

	size_t stuff = Cache::cacheSet.size();
	std::cout << stuff << std::endl;

	inputFile.close();

	return 0;
}