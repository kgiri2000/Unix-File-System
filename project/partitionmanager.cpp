#include "disk.h"
#include "diskmanager.h"
#include "partitionmanager.h"
#include <iostream>
using namespace std;


PartitionManager::PartitionManager(DiskManager *dm, char partitionname, int partitionsize)
{
  myDM = dm;
  myPartitionName = partitionname;
  myPartitionSize = myDM->getPartitionSize(myPartitionName);
  /* If needed, initialize bit vector to keep track of free and allocted
     blocks in this partition */
  myBitVector = new BitVector(myPartitionSize);
  char buffer[64];
  myDM->readDiskBlock(myPartitionName, 0, buffer);
  myBitVector->setBitVector((unsigned int * ) buffer);
}

PartitionManager::~PartitionManager()
{
  delete myBitVector;
}

/*
 * return blocknum, -1 otherwise
 */
int PartitionManager::getFreeDiskBlock()
{
  /* write the code for allocating a partition block */
  for (int i = 2; i < myPartitionSize; i++) {
    if (myBitVector->testBit(i) == 0) {
      myBitVector->setBit(i);
      char buffer[64];

      myBitVector->getBitVector((unsigned int *) buffer);
      myDM->writeDiskBlock(myPartitionName, 0, buffer);
      return i;
    }
  }
  return -1; //place holder so there are no compiler warnings
}

/*
 * return 0 for sucess, -1 otherwise
 */
int PartitionManager::returnDiskBlock(int blknum)
{
  /* write the code for deallocating a partition block */
  if (blknum <=1 || blknum >= myPartitionSize) {
    return -1;
  }
  
  char buffer[64];
  for (int i = 0; i <= 63; i++) {
    buffer[i] = "#"
  }
  int result = myDM->writeDiskBlock(myPartitionName, blknum, buffer);
  if (result == 0) {
    myBitVector->resetBit(blknum);
    myBitVector->getBitVector((unsigned int *) buffer);
    myDM->writeDiskBlock(myPartitionName, 0, buffer);
    return 0;
  }
  return -1; //place holder so there are no compiler warnings
}


int PartitionManager::readDiskBlock(int blknum, char *blkdata)
{
  return myDM->readDiskBlock(myPartitionName, blknum, blkdata);
}

int PartitionManager::writeDiskBlock(int blknum, char *blkdata)
{
  return myDM->writeDiskBlock(myPartitionName, blknum, blkdata);
}

int PartitionManager::getBlockSize() 
{
  return myDM->getBlockSize();
}
