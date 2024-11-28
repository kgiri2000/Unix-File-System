#include "disk.h"
#include "diskmanager.h"
#include "partitionmanager.h"
#include "filesystem.h"
#include <time.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
using namespace std;


FileSystem::FileSystem(DiskManager *dm, char fileSystemName)
{
  //this will create the new Partition and put the return value in myPM
  myDM = dm;
  myfileSystemName = fileSystemName;
  myfileSystemSize = dm->getPartitionSize(fileSystemName);
  myPM = new PartitionManager(myDM,myfileSystemName, myfileSystemSize);


  //Initializing root directory
  char rootDirBlock[64];
  if(myPM->readDiskBlock(1,rootDirBlock) != 0){
    std::memset(rootDirBlock, 0, 64);
    myPM->writeDiskBlock(1, rootDirBlock);
  }
}

//Check the valid fileName
//Need to be worked for alternating promot like /a/b/c and /a/b////c
bool FileSystem::isValidFileName(const char *filename, int fnameLen){
  if(fnameLen<= 0 || filename[0] != '/')return false;
  for(int i = 1; i<fnameLen; ++i){
    if(!((filename[i] >= 'A' && filename[i] <= 'Z') ||
      (filename[i] >= 'a' && filename[i] <= 'z') ||
      (filename[i] == '/'))){
        return false;
      }
  }
  return true;
}

//Checking if file already exists
int FileSystem::findFile(const char *filename){
  char rootDirBlock[64];
  myPM->readDiskBlock(1,rootDirBlock);
  for(int i = 0; i <10 ; ++i){
    //File name can be found in every 6th byte in block 1
    if(rootDirBlock[i*6] == filename[0]){
      return i; //File found
    }
  }
  return -1; //File not found
}
//Allocate the block
int FileSystem::allocateBlock(){
  return myPM->getFreeDiskBlock();
}
//Deallocate the Block
void FileSystem::freeBlock(int blknum){
  myPM->returnDiskBlock(blknum);
}

//TODO:
//We need to allocate the memory through indirect i-nodes as well
int FileSystem::allocateFileBlocks(int fileSize, std::vector<int> &allocatedBlocks){
  int blocksNeeded = (fileSize+ 63)/64;
  for(int i = 0; i< blocksNeeded; ++i){
    int block =  allocateBlock();
    if(block == -1){
      //Not enough memory so we deallocate the previous memory
      for(std::vector<int>::size_type j= 0; j<allocatedBlocks.size(); ++j){
        freeBlock(allocatedBlocks[j]);
      }
      return -2; //Not enough memory
    }
    //Store the allocated block in the vector
    allocatedBlocks.push_back(block);
  }
  return 0; //Success
}
int FileSystem::createFile(char *filename, int fnameLen)
{
  if(!isValidFileName(filename, fnameLen)){
    return -3; //Invalid filename
  }
  if(findFile(filename) >= 0){
    return -1; //File already exists
  }
  //Allocate block for file i-node
  int fileInodeBlock = allocateBlock();
  if(fileInodeBlock == -1){
    return -2; // Not enough memory
  }
  //Allocate data blocks based on the fnameSize;
  std::vector<int> dataBlocks;
  if(allocateFileBlocks(fnameLen, dataBlocks) != 0){
    //Deallocate the file-inode
    freeBlock(fileInodeBlock);
    return -2; //Not enough data blocks
  }

  //Initialize and write file i-node
  char inodeBlock[64] = {0};
  //Casting inodeBlock to FileNode* to directly manipulate the memory
  FileInode *fileInode = reinterpret_cast<FileInode*>(inodeBlock);
  fileInode->name = filename[0];
  fileInode->type = 1; //File
  fileInode->size = fnameLen;
  for(size_t i = 0; i<3 && i< dataBlocks.size() ; ++i){
    fileInode->direct[i] = dataBlocks[i];
  }
  if(dataBlocks.size() > 3){
    int indirectBlock = allocateBlock();
    if(indirectBlock == -1){
      freeBlock(fileInodeBlock);
      for(int block: dataBlocks)freeBlock(block);
      return -2; //Not enough space
    }
    //If available we, allocate new indirect block which has 16 more pointers to data blocks
    fileInode->indirect = indirectBlock;
    char indirectDataBlock[64] = {0};
    IndirectInode *indirect = reinterpret_cast<IndirectInode*>(indirectDataBlock);
    //Allocating indirect data blocks
    for(size_t i = 3; i < dataBlocks.size(); ++i){
      indirect->blockPointers[i-3] = dataBlocks[i];
    }
    //Writing indirect Data Block in the new indirect Block
    myPM->writeDiskBlock(indirectBlock, indirectDataBlock);
  }
  //Writing file i-node data to inode Block
  myPM->writeDiskBlock(fileInodeBlock, inodeBlock);

  //Add file entry to root directory
  char rootDirBlock[64];
  myPM->readDiskBlock(1,rootDirBlock);

  DirectoryInode *rootDir = reinterpret_cast<DirectoryInode*>(rootDirBlock);
  //Check all ten positions
  //TODO: Need to allocate another block for directory if full
  for(int i = 0; i < 10; ++i){
    if(rootDir->entries[i].enntryName == 0){
      rootDir->entries[i].enntryName = filename[0];
      rootDir->entries[i].blockPointer = fileInodeBlock; //storing file i-node
      rootDir->entries[i].entryType = 1; //File
      myPM->writeDiskBlock(1, rootDirBlock);
      return 0;
    }
  }

  freeBlock(fileInodeBlock);
  for(int block: dataBlocks) freeBlock(block);
  return -2; //Directory Full

}

int FileSystem::createDirectory(char *dirname, int dnameLen)
{
 return -1; //place holder so there is no warnings when compiling.
}

int FileSystem::lockFile(char *filename, int fnameLen)
{
 return -1; //place holder so there is no warnings when compiling.
}

int FileSystem::unlockFile(char *filename, int fnameLen, int lockId)
{
 return -1; //place holder so there is no warnings when compiling.
}

int FileSystem::deleteFile(char *filename, int fnameLen)
{
 return -1; //place holder so there is no warnings when compiling.
}

int FileSystem::deleteDirectory(char *dirname, int dnameLen)
{
 return -1; //place holder so there is no warnings when compiling.
}

int FileSystem::openFile(char *filename, int fnameLen, char mode, int lockId)
{
 return -1; //place holder so there is no warnings when compiling.
}

int FileSystem::closeFile(int fileDesc)
{
 return -1; //place holder so there is no warnings when compiling.
}

int FileSystem::readFile(int fileDesc, char *data, int len)
{
  return 0; //place holder so there is no warnings when compiling.
}

int FileSystem::writeFile(int fileDesc, char *data, int len)
{
  return 0; //place holder so there is no warnings when compiling.
}

int FileSystem::appendFile(int fileDesc, char *data, int len)
{
 return -1; //place holder so there is no warnings when compiling.
}

int FileSystem::truncFile(int fileDesc, int offset, int flag)
{
 return -1; //place holder so there is no warnings when compiling.
}

int FileSystem::seekFile(int fileDesc, int offset, int flag)
{
 return -1; //place holder so there is no warnings when compiling.
}

int FileSystem::renameFile(char *filename1, int fnameLen1, char *filename2, int fnameLen2)
{
 return -1; //place holder so there is no warnings when compiling.
}

int FileSystem::renameDirectory(char *dirname1, int dnameLen1, char *dirname2, int dnameLen2)
{
 return -1; //place holder so there is no warnings when compiling.
}

int FileSystem::getAttribute(char *filename, int fnameLen /* ... and other parameters as needed */)
{
 return -1; //place holder so there is no warnings when compiling.
}

int setAttribute(char *filename, int fnameLen /* ... and other parameters as needed */)
{
 return -1; //place holder so there is no warnings when compiling.
}
