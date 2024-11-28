#include "disk.h"
#include "diskmanager.h"
#include "partitionmanager.h"
#include "bitvector.h"
#include <iostream>
using namespace std;


PartitionManager::PartitionManager(DiskManager *dm, char partitionname, int partitionsize)
{
  myDM = dm;
  myPartitionName = partitionname;
  myPartitionSize = myDM->getPartitionSize(myPartitionName);
  /* If needed, initialize bit vector to keep track of free and allocted
     blocks in this partition */
  BitVector *myBV = new BitVector(myPartitionSize);
  char buffer[64];
  //setup buffer with default value, so no junk in the bitvector
  for (int j = 0; j < 64; j++) buffer[j] = '#';
  
  //Code from technical_issue
  myBV->getBitVector((unsigned int *)buffer);
  myDM->writeDiskBlock(myPartitionName, 0, buffer);
  //Debugging Only
  //print out the bit vector
  cout<<"Original Bit Vector: ";
  for (int i=0; i<100; i++) {
    if (myBV->testBit(i) ==0) {
       cout <<"0";
    } else {
       cout <<"1";
    }
  }
  cout<<endl;


}

PartitionManager::~PartitionManager()
{
  //we can try saving bitVector back to block zero before deleting
  delete myBV;
 
}

/*
 * return blocknum, -1 otherwise
 */
int PartitionManager::getFreeDiskBlock()
{
  /* write the code for allocating a partition block */
  for (int i = 2; i < myPartitionSize; i++) {
    if (myBV->testBit(i) == 0) {

      //After getting the free block, we set it to 1
      myBV->setBit(i);
      //Save the updated BitVecctor back to the block 0
      char buffer[64];
      myBV->getBitVector((unsigned int *) buffer);
      myDM->writeDiskBlock(myPartitionName, 0, buffer);
        //Debugging Only
      //print out the bit vector

      cout<<"After Allocating at block"<<i<<": ";
      for (int i=0; i<10; i++) {
        if (myBV->testBit(i) ==0) {
          cout <<"0";
        } else {
          cout <<"1";
        }
      }


      return i;
    }
  }
  return -1; // No free blocks
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
    buffer[i] = '#';
  }
  int result = myDM->writeDiskBlock(myPartitionName, blknum, buffer);
  if (result == 0) {
    //reset the bit vector
    myBV->resetBit(blknum);
    //Get the bit vector in the buffer
    myBV->getBitVector((unsigned int *) buffer);
    //Store the new buffer in the first block of the partation
    myDM->writeDiskBlock(myPartitionName, 0, buffer);
    //print out the bit vector

    //Debugging Only
    cout<<"After Deallocating in block"<<blknum<<": ";
    for (int i=0; i<10; i++) {
      if (myBV->testBit(i) ==0) {
        cout <<"0";
      } else {
        cout <<"1";
      }
    }
    return 0;
  }
  return -1; //Error
}


int PartitionManager::readDiskBlock(int blknum, char *blkdata)
{
  //We can check the bound of the blk number
  return myDM->readDiskBlock(myPartitionName, blknum, blkdata);
}

int PartitionManager::writeDiskBlock(int blknum, char *blkdata)
{
  //We can check the bound for the block number
  return myDM->writeDiskBlock(myPartitionName, blknum, blkdata);
}

int PartitionManager::getBlockSize() 
{
  return myDM->getBlockSize();
}
