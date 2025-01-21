#include <bitset>
#include <vector>
#include <cmath>
#include <climits>
#include <iostream>

#include "Block.h"
#include "Cache.h"

int Cache::memTraffic = 0;
int Cache::lruCounter = 0;  //used to update lru priority for blocks; given FIFO's similar nature to lru, we will recycle this for FIFO but use it only for reads and writes after misses
int Cache::numValidCaches = 2;
int Cache::incPolicy = 0;
int Cache::repPolicy = 0;
std::vector<Cache> Cache::cacheSet;

Cache::Cache(int i, int j, int depth, int blockSize) {
	//initialize everything 

	reads = 0;
	readMisses = 0;
	//keep track of the number of writes and write misses
	writes = 0;
	writeMisses = 0;
	//keep track of the number of writebacks
	writeBacks = 0;

	missRate = 0.0f;
	level = depth;
	size = i;
	assoc = j;
	numSets = size / (assoc * blockSize);

	numIndexBits = (int)log2(numSets);
	numOffsetBits = (int)log2(blockSize);
	numTagBits = ADDRESS_SIZE - numIndexBits - numOffsetBits;

	tagSelector = (pow(2, numTagBits) - 1) * pow(2, numIndexBits + numOffsetBits);
	indexSelector = (pow(2, numIndexBits) - 1) * pow(2, numOffsetBits);

	mem = new Block * [numSets];

	for (int i = 0; i < numSets; i++) {
		mem[i] = new Block[assoc];
	}

	processedTags = NULL;
	processedIndices = NULL;
	lookAheadValues = NULL;
}
//this function parses out the index and tag out of each address and stores them in arrays
void Cache::processAddresses(int numOpCodes, std::vector<int> addresses) {

	processedIndices = new int[numOpCodes];
	processedTags = new int[numOpCodes];

	for (int i = 0; i < numOpCodes; i++) {  //for each address, convert to an int from 0 to whatever is appropriate

		processedIndices[i] = (addresses[i] & indexSelector) / pow(2, numOffsetBits);
		processedTags[i] = (addresses[i] & tagSelector) / pow(2, numOffsetBits + numIndexBits);
	}

}
//this function cretes an array that stores values which determine optimal replacement priority for each block
void Cache::optimalLookahead(int numOpCodes) {

	if (processedIndices == NULL)
		return;

	lookAheadValues = new int[numOpCodes];  //store numbers in this array that will determine replacement priority for each address

	for (int i = 0; i < numOpCodes - 1; i++) {  //for each address...

		bool found = false; //keeps track of whether we've verified the block will be used later

		for (int j = i + 1; j < numOpCodes; j++) {  //search through all later addresses

			if (processedTags[i] == processedTags[j] && processedIndices[i] == processedIndices[j]) {  //if there is a matching address (index AND tag)

				lookAheadValues[i] = j;
				found = true;
				j = numOpCodes; //set j to exit for loop
			}

		}

		if (!found)  //if the block will not be used later, this has the greatest need to be replaced
			lookAheadValues[i] = numOpCodes;
	}

	lookAheadValues[numOpCodes - 1] = numOpCodes;  //obviously, the last address will not be used again
}

void Cache::findTag(int codeNum, char type, std::vector<Cache> cacheSet, int rwCodeNum) {

	int tag = processedTags[rwCodeNum];
	int set = processedIndices[rwCodeNum];

	if (type == 'r')
		updateReads(level);
	else
		updateWrites(level);

	for (int i = 0; i < assoc; i++) {

		if (mem[set][i].getTag() == tag && mem[set][i].getValid()) {

			mem[set][i].setOpCodeNum(rwCodeNum);
			mem[set][i].setLruNum(Cache::lruCounter); //show that the block has been accessed more recently now

			if (repPolicy == 2)  //if using Optimal replacement, update this block's priority
				mem[set][i].setOptimalNum(lookAheadValues[rwCodeNum]);

			if (type == 'w') {  //if this is a write request, set the block's dirty bit
				mem[set][i].setDirty(true);
			}

			return;  //we found the requested tag, so exit the function
		}
	}
	//if we've made it this far in the function, we've missed, and need to update a miss value
	if (type == 'r')
		updateReadMisses(level);

	else
		updateWriteMisses(level);

	return replaceInvalidBlock(codeNum, type, tag, set, cacheSet, rwCodeNum);
}
//this function should only be called after a miss
//  this function will put an address into an invalid block, if an invalid block exists in the appropriate set
void Cache::replaceInvalidBlock(int codeNum, char type, int tag, int set, std::vector<Cache> cacheSet, int rwCodeNum) {

	for (int i = 0; i < assoc; i++) {  //for each block in the set...

		if (!mem[set][i].getValid()) {  //if we find an invalid block...

			mem[set][i].setTag(tag);  //put the new tag in the block
			mem[set][i].setOpCodeNum(rwCodeNum); //show that the block has been accessed more recently now
			mem[set][i].setLruNum(lruCounter);
			mem[set][i].setFifoNum(lruCounter); //we update the FIFO num since we're placing a new tag into this block
			mem[set][i].setDirty(false);

			if (repPolicy == 2)  //if we're using Optimal replacement, we update the block's priority
				mem[set][i].setOptimalNum(lookAheadValues[rwCodeNum]);

			mem[set][i].setValid(1);

			if (type == 'w') {  //if we're writing, we set the dirty bit
				mem[set][i].setDirty(true);
			}

			if (level + 1 < numValidCaches) {  //since we missed in cache, we send a read request to the next cache, if there is one

				cacheSet[level + 1].findTag(codeNum, 'r', cacheSet, rwCodeNum); //Reminder: next cache will figure out the correct index and tag from rwCodeNum
				lruCounter++;
			}
			else {  //next level of memory is main
				//ISSUE A READ REQUEST TO MAIN MEMORY: OUT OF SCOPE
			}

			return;  //we were able to place the tag in an invalid block, so we exit the function
		}
	}
	//we were not able to find an invalid block to use, so we'll allocate one with a replacement policy
	return allocateBlock(codeNum, type, tag, set, cacheSet, rwCodeNum);
}
//this function should only be called after replaceInvalidBlock has been called
//   this function initiates one of the three replacement policies and issues write (if needed) and read requests to the next level of memory
void Cache::allocateBlock(int codeNum, char type, int tag, int set, std::vector<Cache> cacheSet, int rwCodeNum) {

	int oldCodeNum = 0;
	int oldWriteBacks = writeBacks;

	if (repPolicy == 0) {  // LRU
		oldCodeNum = lruReplace(rwCodeNum, type, tag, set);
	}

	else if (repPolicy == 1) {  //FIFO
		oldCodeNum = fifoReplace(rwCodeNum, type, tag, set);
	}

	else {  //Optimal
		oldCodeNum = optimalReplace(codeNum, type, tag, set, rwCodeNum);
	}

	if (oldWriteBacks == writeBacks - 1) { //if the victim block was dirty

		if (level + 1 < numValidCaches) {  //if there's another level of cache, write the block to the next level of cache
			cacheSet[level + 1].findTag(codeNum, 'w', cacheSet, oldCodeNum);
			lruCounter++;
		}
		else {  //next level is main memory

		}
	}

	//if we replaced a block in a cache that is not L1 AND we're using the inclusive property
	if (level > 0 && incPolicy == 1)
		return cacheSet[0].invalidateBlock(oldCodeNum);

	if (level + 1 < numValidCaches) {
		cacheSet[level + 1].findTag(codeNum, 'r', cacheSet, rwCodeNum);
		lruCounter++;
	}

	return;
}
//this function uses the lru policy to determine a victim block; it returns the operation number associated with the victim block so that this can be sent to the next level of memory if the victim block is dirty
int Cache::lruReplace(int codeNum, char type, int tag, int set) {

	int lru = INT_MAX;
	int lruIndex = 0;
	int returnVal = 0;

	for (int i = 0; i < assoc; i++) {

		if (mem[set][i].getLruNum() < lru) {
			lru = mem[set][i].getLruNum();
			lruIndex = i;
		}
	}

	if (mem[set][lruIndex].getDirty()) {
		updateWriteBacks(level);
		mem[set][lruIndex].setDirty(false);
	}
	returnVal = mem[set][lruIndex].getOpCodeNum();  //return the op num associated with the victim block

	mem[set][lruIndex].setTag(tag);
	mem[set][lruIndex].setOpCodeNum(codeNum);
	mem[set][lruIndex].setLruNum(lruCounter);

	if (type == 'w') {
		mem[set][lruIndex].setDirty(true);
	}

	return returnVal;
}
//this function uses the fifo policy to determine a victim block; it returns the operation number associated with the victim block so that this can be sent to the next level of memory if the victim block is dirty
int Cache::fifoReplace(int codeNum, char type, int tag, int set) {

	int fifo = INT_MAX;
	int fifoIndex = 0;
	int returnVal = 0;

	for (int i = 0; i < assoc; i++) {

		if (mem[set][i].getFifoNum() < fifo) {
			fifo = mem[set][i].getFifoNum();
			fifoIndex = i;
		}
	}

	if (mem[set][fifoIndex].getDirty()) {
		updateWriteBacks(level);
		mem[set][fifoIndex].setDirty(false);
	}
	returnVal = mem[set][fifoIndex].getOpCodeNum();  //return the op num associated with the victim block

	mem[set][fifoIndex].setTag(tag); //insert the new tag into the block
	mem[set][fifoIndex].setOpCodeNum(codeNum);
	mem[set][fifoIndex].setLruNum(lruCounter);  //show that the block has been updated more recently
	mem[set][fifoIndex].setFifoNum(lruCounter);  //since we're putting a new tag into the block, we update the fifo priority

	if (type == 'w') {
		mem[set][fifoIndex].setDirty(true);
	}

	return returnVal;
}
//this function uses the optimal policy to determine a victim block; it returns the operation number associated with the victim block so that this can be sent to the next level of memory if the victim block is dirty
int Cache::optimalReplace(int codeNum, char type, int tag, int set, int rwCodeNum) {

	int opt = 0;
	int optIndex = 0;
	int returnVal = 0;

	for (int i = 0; i < assoc; i++) {

		if (mem[set][i].getOptimalNum() > opt) {
			opt = mem[set][i].getOptimalNum();
			optIndex = i;
		}
	}
	if (mem[set][optIndex].getDirty()) {
		updateWriteBacks(level);
		mem[set][optIndex].setDirty(false);
	}

	returnVal = mem[set][optIndex].getOpCodeNum();
	mem[set][optIndex].setTag(tag);
	mem[set][optIndex].setOpCodeNum(rwCodeNum);
	mem[set][optIndex].setOptimalNum(lookAheadValues[rwCodeNum]);

	if (type == 'w') {
		mem[set][optIndex].setDirty(true);
	}

	return returnVal;
}

//this function invalidates a block if the corresponding block in the next level of cache was evicted
void Cache::invalidateBlock(int codeNum) {

	int tag = processedTags[codeNum];
	int set = processedIndices[codeNum];

	for (int i = 0; i < assoc; i++) {

		if (mem[set][i].getTag() == tag && mem[set][i].getValid()) {

			mem[set][i].setValid(0);  //invalidate the block
			mem[set][i].setLruNum(0);

			if (mem[set][i].getDirty()) {
				//updateWriteBacks(0);
				mem[set][i].setDirty(false);
				memTraffic++;
			}

			return;
		}
	}

	return;
}

//functions to update members of the Cache Class
void Cache::updateReadMisses(int level) {
	if (level == numValidCaches - 1) {
		Cache::memTraffic++;
	}
	cacheSet[level].readMisses++;
}

void Cache::updateWriteMisses(int level) {
	if (level == numValidCaches - 1) {
		Cache::memTraffic++;
	}
	cacheSet[level].writeMisses++;
}

void Cache::updateReads(int level) {
	cacheSet[level].reads++;
}

void Cache::updateWrites(int level) {
	cacheSet[level].writes++;
}

void Cache::updateWriteBacks(int level) {
	if (level == numValidCaches - 1) {
		Cache::memTraffic++;
	}
	cacheSet[level].writeBacks++;
}

int Cache::getAssoc() {
	return assoc;
}

void Cache::setAssoc(int num) {
	assoc = num;
}

int Cache::getNumSets() {
	return numSets;
}

void Cache::setNumSets(int num) {
	numSets = num;
}

int Cache::getReads() {
	return reads;
}

int Cache::getReadMisses() {
	return readMisses;
}
//keep track of the number of writes and write misses
int Cache::getWrites() {
	return writes;
}

int Cache::getWriteMisses() {
	return writeMisses;
}

//keep track of the number of writebacks
int Cache::getWriteBacks() {
	return writeBacks;
}

float Cache::getMissRate() {
	return missRate;
}

void Cache::setMissRate(float rate) {
	missRate = rate;
}