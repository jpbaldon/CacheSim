#pragma once

class Block {

private:
	int tag;
	int valid;  //whether the block is actually storing data
	bool dirty;  //whether the block has been written to and needs to be written to the next cache upon eviction
	int opCodeNum;  //allows us to easily access the full address again without actually storing it in the block (due to requests made to L2 cache, this number may be less than lruNum)
	int lruNum;
	int optimalNum; //op num corresponding to when this block will be needed again
	int fifoNum;   //a number associated with when this block was first put into the cache

public:

	Block();

	int getTag();
	void setTag(int num);
	int getValid();
	void setValid(int num);
	bool getDirty();
	void setDirty(bool setVal);
	int getOpCodeNum();
	void setOpCodeNum(int num);
	int getLruNum();
	void setLruNum(int num);
	int getOptimalNum();
	void setOptimalNum(int num);
	int getFifoNum();
	void setFifoNum(int num);

};

