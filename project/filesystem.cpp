#include "disk.h"
#include "diskmanager.h"
#include "partitionmanager.h"
#include "filesystem.h"
#include <time.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <mutex>
#include <atomic>
using namespace std;

map <string,int> fLocks;     //For locking and unlocking the files, we will need to map the file name tot he lock number
mutex fLocksMut;        //For protecting the fLocks map
atomic<int> lockNumGen{1};   //Atomic way of getting unique lock numbers

FileSystem::FileSystem(DiskManager *dm, char fileSystemName) {
    myDM = dm;
    myfileSystemName = fileSystemName;
    myfileSystemSize = dm->getPartitionSize(fileSystemName);
    myPM = new PartitionManager(myDM, myfileSystemName, myfileSystemSize);

    // Initialize the root directory block (block 1)
    char rootDirBlock[64];
    for(int i = 0; i< 64; i++) rootDirBlock[i] = '0';
    rootDirBlock[64] = '\0';
    myPM->readDiskBlock(1, rootDirBlock);
    //Setting root directory pointer to -1 for now
    DirectoryInode *dirInode = reinterpret_cast<DirectoryInode*>(rootDirBlock);
    dirInode->nextDirBlock = -1; // No overflow block
    for (int i = 0; i < 10; i++) {
        dirInode->entries[i].entryName = '0'; // Empty slot
        dirInode->entries[i].entryType = '0'; // Undefined
        dirInode->entries[i].blockPointer = -1;
    }
    //Everything is 0 except the block pointer
    myPM->writeDiskBlock(1, rootDirBlock);

}


//Check the valid fileName
//Need to be worked for alternating promot like /a/b/c and /a/b////c
bool FileSystem::isValidFileName(const char *filename, int fnameLen) {
    if (fnameLen <= 0 || filename[0] != '/') return false;
    bool lastWasSlash = false;
    for (int i = 1; i < fnameLen; ++i) {
        if (filename[i] == '/') {
            if (lastWasSlash) return false; // Reject consecutive slashes
            lastWasSlash = true;
        } else if (!((filename[i] >= 'A' && filename[i] <= 'Z') ||
                     (filename[i] >= 'a' && filename[i] <= 'z'))) {
            return false; // Invalid character
        } else {
            lastWasSlash = false;
        }
    }
    return !lastWasSlash; // Reject trailing slash
}

//Allocate the block
int FileSystem::allocateBlock(){
  return myPM->getFreeDiskBlock();
}
//Deallocate the Block
void FileSystem::freeBlock(int blknum){
  myPM->returnDiskBlock(blknum);
}

//To find the file
int FileSystem::findFile(int fblock, const char &fname){
  int currentBlock = fblock;
  while(currentBlock != -1){
    char parentBlockdata[64];
    for(int i = 0; i< 64; i++) parentBlockdata[i] = '#';
    parentBlockdata[64] = '\0';
    myPM->readDiskBlock(currentBlock, parentBlockdata);
    DirectoryInode *parentDir = reinterpret_cast<DirectoryInode*>(parentBlockdata);
    for(int i = 0; i< 10; i++){
      if(parentDir->entries[i].entryName == '0') continue;
      if(parentDir->entries[i].entryName == fname && parentDir->entries[i].entryType == 'f'){
        return parentDir->entries[i].blockPointer; //File already exist
      }
    }
    
    currentBlock = parentDir->nextDirBlock;
  }
  return -1; // No file found

  
}


int FileSystem::findDirectory(int dirBlock, const char &dirName){
  //Until the next directory pointer is not -1,we loop
  while (dirBlock != -1) {
    char dirBlockData[64];
    for(int i = 0; i< 64; i++)dirBlockData[i] = '#';
    dirBlockData[64] = '\0';
    myPM->readDiskBlock(dirBlock, dirBlockData);
    DirectoryInode *dir1 = reinterpret_cast<DirectoryInode*>(dirBlockData);
    for (int i = 0; i < 10; i++) {
      if(dir1->entries[i].entryName== '0')continue;
      if (dir1->entries[i].entryName == dirName && dir1->entries[i].entryType == 'd' ) {
        if(dir1->entries[i].blockPointer != -1){
          return dir1->entries[i].blockPointer; // File found and directories already created
        }
        
      }
    }


    dirBlock = dir1->nextDirBlock; // Move to the next directory block
  }
  return -1; // Dir not found

}


//Function to add entry to Directory
int FileSystem::addEntryToDirectory(int dirBlock, const char &name, int blockPointer, char type) {
    while (true) {
        char dirBlockData[64];
        for(int i = 0; i< 64; i++)dirBlockData[i]= '#';
        myPM->readDiskBlock(dirBlock, dirBlockData);
        DirectoryInode *dir = reinterpret_cast<DirectoryInode*>(dirBlockData);
        // Find an empty slot in the current block
        for (int i = 0; i < 10; ++i) {
            if (dir->entries[i].entryName == '0') { // Empty entry
                dir->entries[i].entryName = name;
                dir->entries[i].blockPointer = blockPointer;
                dir->entries[i].entryType = type;
                cout<<"Writing in the directory"<<endl;
                myPM->writeDiskBlock(dirBlock, dirBlockData);
                cout<<"Writing success"<<endl;
                return 0; // Success
            }
        }

        // If no space in the current block, check for an overflow block
        if (dir->nextDirBlock == -1) {
            // Allocate a new block for overflow
            int newBlock = allocateBlock();
            if (newBlock == -1) {
                return -2; // No free blocks
            }

            // Initialize the new block as an empty directory
            char newBlockData[64];
            for(int i = 0; i< 64; i++)newBlockData[i] = '0';
            newBlockData[64] = '\0';
            DirectoryInode *newDir = reinterpret_cast<DirectoryInode*>(newBlockData);
            for (int i = 0; i < 10; i++) {
              newDir->entries[i].entryName = '0'; // Empty slot
              newDir->entries[i].entryType = '0'; // Undefined
              newDir->entries[i].blockPointer = -1;
            }
            newDir->nextDirBlock = -1; // No further overflow yet
            myPM->writeDiskBlock(newBlock, newBlockData);

            // Link the current block to the new block
            dir->nextDirBlock = newBlock;
            myPM->writeDiskBlock(dirBlock, dirBlockData);

            // Continue with the newly allocated block
            dirBlock = newBlock;
        } else {
            // Move to the next block in the chain
            dirBlock = dir->nextDirBlock;
        }
    }
}



int FileSystem::createFile(char *filename, int fnameLen)
{
  if(!isValidFileName(filename, fnameLen)){
    return -3; //Invalid filename
  }

  //Parse the file path to separate the directory path and file name
  char components[64];
  int componentCount = 0;
  int start= 1;
  for (int i = 1; i < fnameLen; ++i) {
    if (filename[i] == '/') {
        components[componentCount++] = filename[start]; // Add directory component
        start = i + 1;
    }
  }
  char newFileName = filename[start];
  for(int i = 0; i< componentCount; i++){
    cout<<components[i];
  }

  cout<<"New File Name: "<<newFileName<<endl; 
  //Step 2: Traverse the directory hierarchy to locate the parent directory

  int currentBlock  = 1;
  for(int i = 0; i < componentCount; ++i){
    cout<<components[i]<<endl;
    currentBlock = findDirectory(currentBlock, components[i]);
    if(currentBlock == -1){
      return -1; // Directory does not exit
    }
  }

  //Check if file already exists

  if(findFile(currentBlock, newFileName) != -1){
    return -1; // File already exists
  }
  //Allocate a block for the file i-node
  int fileInodeBlock = allocateBlock();
  if(fileInodeBlock == -1){
    return -2; // Space not available
  }
  cout<<"\nFile i-node: "<<fileInodeBlock<<endl;
  //Initialize the file i-node
  char inodeBlock[64];
  for(int i = 0;  i<64; i++)inodeBlock[i]= '#';
  inodeBlock[64] = '\0';
  FileInode *fileInode = reinterpret_cast<FileInode*>(inodeBlock);
  fileInode->name = newFileName;
  fileInode->type = 'f'; // File
  fileInode->size = 0; // Only the size of 
  std::fill(std::begin(fileInode->direct), std::end(fileInode->direct), -1);
  fileInode->indirect = -1;

  myPM->writeDiskBlock(fileInodeBlock, inodeBlock);
  cout<<"After Writing"<<endl;
  //Add an entry in the parent directory
  cout<<"Adding to the directory"<<endl;
  int result = addEntryToDirectory(currentBlock, newFileName, fileInodeBlock, 'f');

  if(result != 0){
    freeBlock(fileInodeBlock);
    return -2; //Directory full

  }
  cout<<"File Created"<<endl;
  return 0; //File Created


}

int FileSystem::createDirectory(char *dirname, int dnameLen)
{
  if(!isValidFileName(dirname, dnameLen)){
    return -3; //Invalid directory name
  }

  //Parse the file path to separate the directory path and file name
  char components[64];
  int componentCount = 0;
  int start= 1;
  for (int i = 1; i < dnameLen; ++i) {
    if (dirname[i] == '/') {
        components[componentCount++] = dirname[start]; // Add directory component
        start = i + 1;
    }
  }
  char newDirName = dirname[start];
  cout<<"All the directories: "<<components<<endl;

  cout<<"New Dir: "<<newDirName<<endl; 
  int currentBlock  = 1;
  for(int i = 0; i< componentCount;++i){
    currentBlock = findDirectory(currentBlock, components[i]);
    if(currentBlock == -1){
      return -4; // Directory does not exit
    }
  }
  cout<<"Current Directory: "<<currentBlock<<endl;
  //Find if directory already exists
  if(findDirectory(currentBlock, newDirName) != -1){
    return -1; // Directory already exists;
  }
  //Allocate new sub directory
  int newBlock = allocateBlock();
  if(newBlock == -1){
    return -2; // Space not available
  }

// Initialize the new block as an empty directory
  char newDirBlock[64];
  for(int i = 0; i< 64; i++)newDirBlock[i] = '0';
  newDirBlock[64] = '\0';
  DirectoryInode *newDir1 = reinterpret_cast<DirectoryInode*>(newDirBlock);
  for (int i = 0; i < 10; i++) {
    newDir1->entries[i].entryName = '0'; // Empty slot
    newDir1->entries[i].entryType = '0'; // Undefined
    newDir1->entries[i].blockPointer = -1;
  }
  newDir1->nextDirBlock = -1; // No further overflow yet
  myPM->writeDiskBlock(newBlock, newDirBlock);
  

  cout<<"Adding to the directory"<<endl;
  int result = addEntryToDirectory(currentBlock, newDirName, newBlock, 'd');

  if(result !=0 ){
    freeBlock(newBlock);
    return -2; // Directory Full
  }
  cout<<"Directory Created From File System"<<endl;
  return(0);

}

int FileSystem::lockFile(char *filename, int fnameLen)
{
 string fname(filename, fnameLen);
 lock_guard<mutex> lock(fLocksMut);
    if (fLocks.find(fname) != fLocks.end()) {
        return -1; //The file is already locked in this case
    }
    int lockNum = lockNumGen.fetch_add(1); //Random lock number
    fLocks[fname] = lockNum;
    return lockNum;
}

int FileSystem::unlockFile(char *filename, int fnameLen, int lockId)
{
 string fname(filename, fnameLen);
 lock_guard<mutex> lock(fLocksMut);
    if (fLocks.find(fname) == fLocks.end() || fLocks[fname] != lockId) {
        return -1; //No lock found
    }
    fLocks.erase(fname);
    return 0;
}

int FileSystem::deleteFile(char *filename, int fnameLen) {
  string fname(filename,fnameLen);
  //We need to find the file's parent directory block
  int parentBlock = findFile(1,  fname[0]);
  if (parentBlock == -1) {
    return -1; //Not found
  }

   //If the parent block is not -1, we need to free up the parent block
  freeBlock(parentBlock);
  return 0;
}

int FileSystem::deleteDirectory(char *dirname, int dnameLen) {
  string dirName(dirname, dnameLen);
  //We need to find the directory block that corresponds to given directory name
  int dirBlock = findDirectory(1, dirName[0]);
  if (dirBlock == -1) {
    return -1; //Not found
  }

    //Now let's read the data into the buffer because if there is a non-empty directory, we want to spit out an error
  char dirBlockData[64];
  myPM->readDiskBlock(dirBlock,dirBlockData);
  DirectoryInode *dir = reinterpret_cast<DirectoryInode*>(dirBlockData);

  //Checking if empty here
  for (int i = 0; i < 10; ++i) {
    if (dir->entries[i].entryName != 0) {
      return -2; // Non-empty directory
  }
}

  //Otherwise the directory is empty, so let's free its block
  freeBlock(dirBlock);
  return 0;
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
