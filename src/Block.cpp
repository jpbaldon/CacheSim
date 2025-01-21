#include "block.h"

Block::Block() {
	tag = 0;
	valid = 0;
	opCodeNum = 0;
	lruNum = 0;
	optimalNum = 0;
	fifoNum = 0;
	dirty = false;
}

int Block::getTag() {
	return tag;
}

void Block::setTag(int num) {
	tag = num;
}

int Block::getValid() {
	return valid;
}

void Block::setValid(int num) {
	valid = num;
}

bool Block::getDirty() {
	return dirty;
}
void Block::setDirty(bool setVal) {
	dirty = setVal;
}

int Block::getOpCodeNum() {
	return opCodeNum;
}

void Block::setOpCodeNum(int num) {
	opCodeNum = num;
}

int Block::getLruNum() {
	return lruNum;
}
void Block::setLruNum(int num) {
	lruNum = num;
}

int Block::getOptimalNum() {
	return optimalNum;
}

void Block::setOptimalNum(int num) {
	optimalNum = num;
}

int Block::getFifoNum() {
	return fifoNum;
}

void Block::setFifoNum(int num) {
	fifoNum = num;
}