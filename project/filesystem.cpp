#include "disk.h"
#include "diskmanager.h"
#include "partitionmanager.h"
#include "filesystem.h"
#include <time.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
using namespace std;


FileSystem::FileSystem(DiskManager *dm, char fileSystemName) {
    myDM = dm;
    myfileSystemName = fileSystemName;
    myfileSystemSize = dm->getPartitionSize(fileSystemName);
    myPM = new PartitionManager(myDM, myfileSystemName, myfileSystemSize);

    // Initialize the root directory block (block 1)
    char rootDirBlock[64] = {0}; // Clear the buffer to avoid garbage data
    int result = myPM->readDiskBlock(1, rootDirBlock);
    cout<<"Result is :"<<result<<endl;
    cout<<"Root Dir Block: "<<rootDirBlock<<endl;
    if (result != 0) {
        // The block was uninitialized or an error occurred, so initialize it
        DirectoryInode *rootDir = reinterpret_cast<DirectoryInode*>(rootDirBlock);
        rootDir->nextDirBlock = -1; // No overflow block initially
        // Optionally, you can initialize the directory entries with empty values
        std::fill(std::begin(rootDir->entries), std::end(rootDir->entries), DirectoryEntry{0, -1, 0});

        // Write the initialized block back to disk
        result = myPM->writeDiskBlock(1, rootDirBlock);
        if (result != 0) {
            std::cerr << "Error: Failed to write root directory block to disk!" << std::endl;
        } else {
            std::cout << "Root directory block initialized successfully!" << std::endl;
        }
    } else {
        std::cout << "Root directory block already initialized." << std::endl;
    }
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
int FileSystem::findFile(int fblock, const std::string &fname){
  int currentBlock = fblock;
  cout<<"Initial Current Block"<<currentBlock<<endl;
  while(currentBlock != -1){
    char parentBlockdata[64];
    myPM->readDiskBlock(currentBlock, parentBlockdata);
    cout<<"Parent Data Block"<<parentBlockdata<<endl;
    DirectoryInode *parentDir = reinterpret_cast<DirectoryInode*>(parentBlockdata);
    for(int i = 0; i< 10; i++){
      if(parentDir->entries[i].entryType == 0) continue;
      if(parentDir->entries[i].entryName == fname[0] && parentDir->entries[i].entryType == 1){
        return parentDir->entries[i].blockPointer; //File already exist
      }
    }
    
    currentBlock = parentDir->nextDirBlock;
    cout<<"Current Block"<<currentBlock<<endl;
    cout<<"Loop"<<endl;
  }
  return -1; // No file found

  
}


int FileSystem::findDirectory(int dirBlock, const std::string &dirName){
  //Until the next directory pointer is not -1,we loop
  while (dirBlock != -1) {
        char dirBlockData[64];
        myPM->readDiskBlock(dirBlock, dirBlockData);

        DirectoryInode *dir = reinterpret_cast<DirectoryInode*>(dirBlockData);

        for (int i = 0; i < 10; ++i) {
            if(dir->entries[i].entryName==0)continue;
            if (dir->entries[i].entryName == dirName[0] && dir->entries[i].entryType == 0 ) {
              
                return dir->entries[i].blockPointer; // File found
            }
        }

        dirBlock = dir->nextDirBlock; // Move to the next directory block
    }
    return -1; // File not found

}


//Function to add entry to Directory
int FileSystem::addEntryToDirectory(int dirBlock, const std::string &name, int blockPointer, int type) {
    while (true) {
        char dirBlockData[64];
        myPM->readDiskBlock(dirBlock, dirBlockData);

        DirectoryInode *dir = reinterpret_cast<DirectoryInode*>(dirBlockData);

        // Find an empty slot in the current block
        for (int i = 0; i < 10; ++i) {
            if (dir->entries[i].entryName == 0) { // Empty entry
                dir->entries[i].entryName = name[0];
                dir->entries[i].blockPointer = blockPointer;
                dir->entries[i].entryType = type;
                myPM->writeDiskBlock(dirBlock, dirBlockData);
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
            char newBlockData[64] = {0};
            DirectoryInode *newDir = reinterpret_cast<DirectoryInode*>(newBlockData);
            // newDir->name = dir->name; // Copy the parent directory name
            // newDir->type = 0; // Directory
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
  string path(filename,fnameLen);
  std::vector<std::string> components;
  size_t start = 1;
  size_t end = 0;
  while((end = path.find('/', start)) != string::npos){
    components.push_back(path.substr(start, end - start));
    start = end +1;
  }
  components.push_back(path.substr(start)); //  Add the last component
  for(const string &dir: components){
    cout<<dir<<endl;
  }

  string newFileName =  components.back();
  cout<<newFileName<<endl;
  components.pop_back(); //Remaining are the directory

  //Step 2: Traverse the directory hierarchy to locate the parent directory

  int currentBlock  = 1;
  for(const string &dir: components){
    currentBlock = findDirectory(currentBlock, dir);
    if(currentBlock == -1){
      return -1; // Directory does not exit
    }
  }
  cout<<"Current Director: "<<currentBlock<<endl;

  //Check if file already exists

  if(findFile(currentBlock, newFileName)== -1){
    return -1; // File already exists
  }
  //Allocate a block for the file i-node
  int fileInodeBlock = allocateBlock();
  if(fileInodeBlock == -1){
    return -2; // Space not available
  }
  cout<<"File i-node"<<fileInodeBlock<<endl;
  //Initialize the file i-node
  char inodeBlock[64] = {0};
  FileInode *fileInode = reinterpret_cast<FileInode*>(inodeBlock);
  fileInode->name = newFileName[0];
  fileInode->type = 1; // File
  fileInode->size = 0; // Empty file
  std::fill(std::begin(fileInode->direct), std::end(fileInode->direct), -1);
  fileInode->indirect = -1;
  cout<<"Writing the i-node to file i-node block"<<endl;

  myPM->writeDiskBlock(fileInodeBlock, inodeBlock);
  cout<<"After Writing"<<endl;
  //Add an entry in the parent directory
  cout<<"Adding to the directory"<<endl;
  int result =  addEntryToDirectory(currentBlock, newFileName, fileInodeBlock, 1);
  cout<<"The result is"<<result<<endl;
  if(result != 0){
    freeBlock(fileInodeBlock);
    return -2; //Directory full

  }
  return 0; //File Created


}

int FileSystem::createDirectory(char *dirname, int dnameLen)
{
  if(!isValidFileName(dirname, dnameLen)){
    return -3; //Invalid directory name
  }

  //Parse the file path to separate the directory path and file name
  string path(dirname,dnameLen);
  std::vector<std::string> components;
  size_t start = 1;
  size_t end = 0;
  while((end = path.find('/', start)) != string::npos){
    components.push_back(path.substr(start, end - start));
    start = end +1;
  }
  components.push_back(path.substr(start)); //  Add the last component

  string newDirName =  components.back();
  components.pop_back(); //Remaining are the directory

  int currentBlock  = 1;
  for(const string &dir: components){
    currentBlock = findDirectory(currentBlock, dir);
    if(currentBlock == -1){
      return -4; // Directory does not exit
    }
  }
  //Find if directory already exists
  if(findDirectory(currentBlock, newDirName)){
    return -1; // Directory already exists;
  }

  //Add entry to Directory
  while(true){
    char dirBlockData[64];
    myPM->readDiskBlock(currentBlock, dirBlockData);
    DirectoryInode *dir = reinterpret_cast<DirectoryInode*>(dirBlockData);

    //Find the empty slot in current block
    for (int i = 0; i < 10; ++i) {
      if (dir->entries[i].entryName == 0) { // Empty entry
          dir->entries[i].entryName = newDirName[0];
          dir->entries[i].blockPointer = -1; // No sub directory for now
          dir->entries[i].entryType = 0; //Type Directories
          myPM->writeDiskBlock(currentBlock, dirBlockData);
          return 0; // Success
      }
    }
    if(dir->nextDirBlock == -1){
      //No space
      int newBlock = allocateBlock();
      if(newBlock == -1){
        return -2; // No free blocks
      }
      char newBlockData[64] = {0};
      DirectoryInode *newDir = reinterpret_cast<DirectoryInode*>(newBlockData);
      newDir ->nextDirBlock = -1;
      myPM->writeDiskBlock(newBlock, newBlockData);
      //Linking the dir
      dir->nextDirBlock = newBlock;
      myPM->writeDiskBlock(currentBlock, dirBlockData);
      currentBlock = newBlock;
    }else{
      currentBlock = dir->nextDirBlock;
    }



  }
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
