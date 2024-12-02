#include "disk.h"
#include "diskmanager.h"
#include <fstream>
#include  <cstring>
#include <iostream>
using namespace std;

DiskManager::DiskManager(Disk *d, int partcount, DiskPartition *dp)
{
  myDisk = d;
  partCount= partcount;
  int r = myDisk->initDisk();
  char buffer[64];
  for (int j = 0; j < 64; j++) buffer[j] = '#';
  buffer[64] = '\0';

  /* If needed, initialize the disk to keep partition information */
  diskP = new DiskPartition[partCount];
  /* else  read back the partition information from the DISK1 */
  if(r ==1 ){
    memcpy(diskP, dp, sizeof(DiskPartition)*partCount);

    //Storing it it on the buffer
    memcpy(buffer, diskP, sizeof(DiskPartition)* partCount);
    //Prepare to store the partation information(block 0 in the disk)
    myDisk->writeDiskBlock(0, buffer);
  }else{
    //Disk already exists, read the partation table from the super block
    myDisk->readDiskBlock(0,buffer);
    //Restore the partition info into memory
    memcpy(diskP, buffer, sizeof(DiskPartition) * partCount);
  }



}
DiskManager::~DiskManager(){
  delete [] diskP;
}

/*
 *   returns: 
 *   0, if the block is successfully read;
 *  -1, if disk can't be opened; (same as disk)
 *  -2, if blknum is out of bounds; (same as disk)
 *  -3 if partition doesn't exist
 */
int DiskManager::readDiskBlock(char partitionname, int blknum, char *blkdata)
{
  /* write the code for reading a disk block from a partition */
  int partationStart = 0;
  ifstream f(myDisk->diskFilename, ios::in);
  if(!f) return(-1);

  //Loop through all partations to find the specified partation
  for(int i = 0; i < partCount; ++i){
    //If the partation supplied matches the disk partation name
    if(diskP[i].partitionName == partitionname){
      //If block number is within the bounds of the partation
      if(blknum >= 0 && blknum < diskP[i].partitionSize){
        //skip the superblock
        int diskBlockNum = partationStart + blknum +  1; 
        return myDisk->readDiskBlock(diskBlockNum, blkdata);
      }else{
        //Block number out of bound
        return(-2);
      }
    }
    //Move to next partition's start block
    partationStart += diskP[i].partitionSize;
  }
  //Partition doesn't exit
  return(-3);

}


/*
 *   returns: 
 *   0, if the block is successfully written;
 *  -1, if disk can't be opened; (same as disk)
 *  -2, if blknum is out of bounds;  (same as disk)
 *  -3 if partition doesn't exist
 */
int DiskManager::writeDiskBlock(char partitionname, int blknum, char *blkdata)
{
  /* write the code for writing a disk block to a partition */
  int partationStart = 0;
  fstream f(myDisk->diskFilename, ios::in|ios::out);
  if (!f) return(-1);

  //Loop through all the partations to find the specified partation
  for(int i = 0 ; i < partCount; ++i){
    if(diskP[i].partitionName == partitionname){
      if(blknum>= 0 && blknum < diskP[i].partitionSize){
        int diskBlockNum = partationStart + blknum +1;
        return myDisk->writeDiskBlock(diskBlockNum,blkdata);
      }else{
        return(-2);
      }
    }
    partationStart+=diskP[i].partitionSize;

  }
  return(-3);

}

/*
 * return size of partition
 * -1 if partition doesn't exist.
 */
int DiskManager::getPartitionSize(char partitionname)
{
  /* write the code for returning partition size */
  for(int i = 0; i< partCount; ++i){
    if(diskP[i].partitionName == partitionname){
      return diskP[i].partitionSize;
    }
  }
  return(-1);

}
