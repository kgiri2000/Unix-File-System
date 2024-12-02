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

  myBV = new BitVector(myPartitionSize);  

  // First, check if the partition's superblock has been initialized
  char buffer[64];
  for(int i = 0; i< 64; i++)buffer[i] = '0';
  buffer[64] = '\0';
  myDM->writeDiskBlock(myPartitionName, 1, buffer);


  // int readResult = myDM->readDiskBlock(myPartitionName, 0, buffer);

  // if(buffer[0] != '#'){
  //   myBV->setBitVector(reinterpret_cast<unsigned int*> (buffer));
  // }else{
  //   myBV->getBitVector(reinterpret_cast<unsigned int*>(buffer));
  //   myDM->writeDiskBlock(partitionname,1,buffer);
  // }
  

}


PartitionManager::~PartitionManager()
{
  //we can try saving bitVector back to block zero before deleting
  delete []myBV;
 
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
      for (int j = 0; j < 64; j++) buffer[j] = '#';
      myBV->getBitVector((unsigned int *) buffer);
      myDM->writeDiskBlock(myPartitionName, 0, buffer);
        //Debugging Only
      //print out the bit vector

      // cout<<"After Allocating at block"<<i<<": ";
      // for (int i=0; i<10; i++) {
      //   if (myBV->testBit(i) ==0) {
      //     cout <<"0";
      //   } else {
      //     cout <<"1";
      //   }
      // }


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
  if(myBV->testBit(blknum) == 0){
    return -1; //Block is alread free
  }
  
  char buffer[64];
  for (int j = 0; j < 64; j++) buffer[j] = '#';
  int result = myDM->writeDiskBlock(myPartitionName, blknum, buffer);
  if (result == 0) {
    //reset the bit vector
    myBV->resetBit(blknum);
    //Get the bit vector in the buffer
    myBV->getBitVector((unsigned int *) buffer);
    //Store the new buffer in the first block of the partation
    myDM->writeDiskBlock(myPartitionName, 0, buffer);
    //print out the bit vector

    // //Debugging Only
    // cout<<"After Deallocating in block"<<blknum<<": ";
    // for (int i=0; i<10; i++) {
    //   if (myBV->testBit(i) ==0) {
    //     cout <<"0";
    //   } else {
    //     cout <<"1";
    //   }
    // }
    return 0;
  }
  return -1; //Error
}


int PartitionManager::readDiskBlock(int blknum, char *blkdata)
{
  //We can check the bound of the blk number
  return myDM->readDiskBlock(myPartitionName, blknum, blkdata);
}

int PartitionManager::writeDiskBlock(int blknum, char *blkdata) {
    // Check if the block number is valid
    if (blknum < 0 || blknum >= myPartitionSize) {
        return -1; // Invalid block number
    }

    // Ensure that the write operation was successful
    int result = myDM->writeDiskBlock(myPartitionName, blknum, blkdata);
    if (result != 0) {
        std::cerr << "Error: Failed to write to disk block " << blknum << std::endl;
        return -1; // Write operation failed
    }
    return 0; // Write operation successful
}

int PartitionManager::getBlockSize() 
{
  return myDM->getBlockSize();
}
