#pragma once
#include <bitset>
#include <vector>

#include "Block.h"

class Cache {

public:

	static const int ADDRESS_SIZE = 32;

	static int numValidCaches;
	static int memTraffic;
	static int lruCounter;  //used to update lru priority for blocks; given FIFO's similar nature to lru, we will recycle this for FIFO but use it only for reads and writes after misses
	static std::vector<Cache> cacheSet;
	static int incPolicy; // 0 for non-inclusive, 1 for inclusive
	static int repPolicy; // 0 for LRU, 1 for Pseudo-LRU, 2 for Optimal

private:

	//these three arrays keep track of processed data to make life easier
	int* processedTags;
	int* processedIndices;
	int* lookAheadValues;
	//used to separate the contents of the address
	int numIndexBits;
	int numOffsetBits;
	int numTagBits;
	//used to perform bitwise operations on the address
	unsigned int tagSelector;
	unsigned int indexSelector;

	int level; //keeps track of which level of Cache this is (0 for L1, 1 for L2,...)
	int size;
	int assoc;
	int numSets;  //guaranteed to be power of 2; not guaranteed to be the same across caches

	//keep track of the number of reads and read misses
	int reads;
	int readMisses;
	//keep track of the number of writes and write misses
	int writes;
	int writeMisses;
	//keep track of the number of writebacks
	int writeBacks;

	float missRate;

public:

	//a 2d array of our cache contents
	Block** mem;

	Cache(int i, int j, int depth, int blockSize);

	//this function parses out the index and tag out of each address and stores them in arrays
	void processAddresses(int numOpCodes, std::vector<int> addresses);

	//this function cretes an array that stores values which determine optimal replacement priority for each block
	void optimalLookahead(int numOpCodes);

	void findTag(int codeNum, char type, std::vector<Cache> cacheSet, int rwCodeNum);

	//this function should only be called after a miss
	//  this function will put an address into an invalid block, if an invalid block exists in the appropriate set
	void replaceInvalidBlock(int codeNum, char type, int tag, int set, std::vector<Cache> cacheSet, int rwCodeNum);

	//this function should only be called after replaceInvalidBlock has been called
	//   this function initiates one of the three replacement policies and issues write (if needed) and read requests to the next level of memory
	void allocateBlock(int codeNum, char type, int tag, int set, std::vector<Cache> cacheSet, int rwCodeNum);

	//this function uses the lru policy to determine a victim block; it returns the operation number associated with the victim block so that this can be sent to the next level of memory if the victim block is dirty
	int lruReplace(int codeNum, char type, int tag, int set);

	//this function uses the fifo policy to determine a victim block; it returns the operation number associated with the victim block so that this can be sent to the next level of memory if the victim block is dirty
	int fifoReplace(int codeNum, char type, int tag, int set);

	//this function uses the optimal policy to determine a victim block; it returns the operation number associated with the victim block so that this can be sent to the next level of memory if the victim block is dirty
	int optimalReplace(int codeNum, char type, int tag, int set, int rwCodeNum);

	//this function invalidates a block if the corresponding block in the next level of cache was evicted
	void invalidateBlock(int codeNum);

	//functions to update members of the Cache Class
	void updateReadMisses(int level);

	void updateWriteMisses(int level);

	void updateReads(int level);

	void updateWrites(int level);

	void updateWriteBacks(int level);

	int getAssoc();
	void setAssoc(int num);
	int getNumSets();
	void setNumSets(int num);

	int getReads();
	int getReadMisses();
	//keep track of the number of writes and write misses
	int getWrites();
	int getWriteMisses();
	//keep track of the number of writebacks
	int getWriteBacks();

	float getMissRate();
	void setMissRate(float rate);

};