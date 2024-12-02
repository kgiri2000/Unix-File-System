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

FileSystem::FileSystem(DiskManager *dm, char fileSystemName)
{
  myDM = dm;
  myfileSystemName = fileSystemName;
  myfileSystemSize = dm->getPartitionSize(fileSystemName);
  myPM = new PartitionManager(myDM, myfileSystemName, myfileSystemSize);

  fileDescCounter = 1;
  // Initialize the root directory block (block 1)
  char rootDirBlock[64];
  for (int i = 0; i < 64; i++)
    rootDirBlock[i] = '0';
  rootDirBlock[64] = '\0';
  myPM->readDiskBlock(1, rootDirBlock);
  // Setting root directory pointer to -1 for now
  DirectoryInode *dirInode = reinterpret_cast<DirectoryInode *>(rootDirBlock);
  bool isInitialized = false;
  for (int i = 0; i < 10; i++)
  {
    if (dirInode->entries[i].entryName != '0')
    {
      isInitialized = true;
      break;
    }
  }
  if (!isInitialized)
  {
    dirInode->nextDirBlock = -1; // No overflow block
    for (int i = 0; i < 10; i++)
    {
      dirInode->entries[i].entryName = '0'; // Empty slot
      dirInode->entries[i].entryType = '0'; // Undefined
      dirInode->entries[i].blockPointer = -1;
    }
    // Everything is 0 except the block pointer
    myPM->writeDiskBlock(1, rootDirBlock);
  }
}

// Check the valid fileName
// Need to be worked for alternating promot like /a/b/c and /a/b////c
bool FileSystem::isValidFileName(const char *filename, int fnameLen)
{
  if (fnameLen <= 0 || filename[0] != '/')
    return false;
  bool lastWasSlash = false;
  for (int i = 1; i < fnameLen; ++i)
  {
    if (filename[i] == '/')
    {
      if (lastWasSlash)
        return false; // Reject consecutive slashes
      lastWasSlash = true;
    }
    else if (!((filename[i] >= 'A' && filename[i] <= 'Z') ||
               (filename[i] >= 'a' && filename[i] <= 'z')))
    {
      return false; // Invalid character
    }
    else
    {
      lastWasSlash = false;
    }
  }
  return !lastWasSlash; // Reject trailing slash
}

// Allocate the block
int FileSystem::allocateBlock()
{
  return myPM->getFreeDiskBlock();
}
// Deallocate the Block
void FileSystem::freeBlock(int blknum)
{
  myPM->returnDiskBlock(blknum);
}

// To find the file
int FileSystem::findFile(char *filename, int fnameLen, int *parentBlock)
{
  int currentBlock = 1;
  // Parse the file path to separate the directory path and file name
  char newFileName = filename[fnameLen - 1];
  *parentBlock = -1;
  // Traverse directories in the path
  for (int i = 1; i < fnameLen - 1; i++)
  {
    if (filename[i] == '/')
    {
      char dirname = filename[i - 1];
      currentBlock = findDirectory(currentBlock, dirname);
      if (currentBlock == -1)
      {
        return -4; // Directory doesnot exist
      }
    }
  }
  *parentBlock = currentBlock;
  while (currentBlock != -1)
  {
    char parentBlockdata[64];
    for (int i = 0; i < 64; i++)
      parentBlockdata[i] = '#';
    parentBlockdata[64] = '\0';
    myPM->readDiskBlock(currentBlock, parentBlockdata);
    DirectoryInode *parentDir = reinterpret_cast<DirectoryInode *>(parentBlockdata);
    for (int i = 0; i < 10; i++)
    {
      if (parentDir->entries[i].entryName == '0')
        continue;
      if (parentDir->entries[i].entryName == newFileName && parentDir->entries[i].entryType == 'f')
      {
        return parentDir->entries[i].blockPointer; // File already exist, returning the pointer
      }
    }

    currentBlock = parentDir->nextDirBlock;
  }

  return -2; // No file found
}

int FileSystem::findDirectory(int dirBlock, char dirName)
{
  // Until the next directory pointer is not -1,we loop
  while (dirBlock != -1)
  {
    char dirBlockData[64];
    for (int i = 0; i < 64; i++)
      dirBlockData[i] = '#';
    dirBlockData[64] = '\0';
    myPM->readDiskBlock(dirBlock, dirBlockData);
    DirectoryInode *dir1 = reinterpret_cast<DirectoryInode *>(dirBlockData);
    for (int i = 0; i < 10; i++)
    {
      if (dir1->entries[i].entryName == '0')
        continue;
      if (dir1->entries[i].entryName == dirName && dir1->entries[i].entryType == 'd')
      {
        if (dir1->entries[i].blockPointer != -1)
        {
          return dir1->entries[i].blockPointer; // File found and directories already created
        }
      }
    }

    dirBlock = dir1->nextDirBlock; // Move to the next directory block
  }
  return -1; // Dir not found
}

// Function to add entry to Directory
int FileSystem::addEntryToDirectory(int dirBlock, const char &name, int blockPointer, char type)
{
  while (true)
  {

    char dirBlockData[64];
    for (int i = 0; i < 64; i++)
      dirBlockData[i] = '#';
    myPM->readDiskBlock(dirBlock, dirBlockData);
    DirectoryInode *dir = reinterpret_cast<DirectoryInode *>(dirBlockData);
    // Find an empty slot in the current block
    for (int i = 0; i < 10; ++i)
    {
      if (dir->entries[i].entryName == '0')
      { // Empty entry
        dir->entries[i].entryName = name;
        dir->entries[i].blockPointer = blockPointer;
        dir->entries[i].entryType = type;
        myPM->writeDiskBlock(dirBlock, dirBlockData);
        return 0; // Success
      }
    }

    // If no space in the current block, check for an overflow block
    if (dir->nextDirBlock == -1)
    {
      // Allocate a new block for overflow
      int newBlock = allocateBlock();
      if (newBlock == -1)
      {
        return -2; // No free blocks
      }

      // Initialize the new block as an empty directory
      char newBlockData[64];
      for (int i = 0; i < 64; i++)
        newBlockData[i] = '0';
      newBlockData[64] = '\0';
      DirectoryInode *newDir = reinterpret_cast<DirectoryInode *>(newBlockData);
      for (int i = 0; i < 10; i++)
      {
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
    }
    else
    {
      // Move to the next block in the chain
      dirBlock = dir->nextDirBlock;
    }
  }
}

int FileSystem::createFile(char *filename, int fnameLen)
{
  if (!isValidFileName(filename, fnameLen))
  {
    return -3; // Invalid filename
  }

  int parentBlock = -1;
  // Check if file already exists
  int result = findFile(filename, fnameLen, &parentBlock);
  if (result == -4)
  {
    return -4; // Directory doesn't exist
  }
  if (result != -2)
  {
    return -1; // File does exist
  }

  char newFileName = filename[fnameLen - 1];
  int currentBlock = parentBlock;

  // Allocate a block for the file i-node
  int fileInodeBlock = allocateBlock();
  if (fileInodeBlock == -1)
  {
    return -2; // Space not available
  }
  // Initialize the file i-node
  char inodeBlock[64];
  for (int i = 0; i < 64; i++)
    inodeBlock[i] = '#';
  inodeBlock[64] = '\0';
  FileInode *fileInode = reinterpret_cast<FileInode *>(inodeBlock);
  fileInode->name = newFileName;
  fileInode->type = 'f'; // File
  fileInode->size = 0;   // Only the size of
  std::fill(std::begin(fileInode->direct), std::end(fileInode->direct), -1);
  fileInode->indirect = -1;
  fileInode->lockId = -1; // Initializing the lock id = -1 (unlocked)
  fileInode->creationTime = time(nullptr);
  fileInode->openCount = 0;

  myPM->writeDiskBlock(fileInodeBlock, inodeBlock);
  int result1 = addEntryToDirectory(currentBlock, newFileName, fileInodeBlock, 'f');

  if (result1 != 0)
  {
    freeBlock(fileInodeBlock);
    return -2; // Directory full
  }
  return 0; // File Created
}

int FileSystem::createDirectory(char *dirname, int dnameLen)
{
  if (!isValidFileName(dirname, dnameLen))
  {
    return -3; // Invalid directory name
  }
  char newDirName = dirname[dnameLen - 1];
  int parentBlock = 1;

  for (int i = 1; i < dnameLen - 1; ++i)
  {
    if (dirname[i] == '/')
    {
      char dirName = dirname[i - 1];
      parentBlock = findDirectory(parentBlock, dirName);
      if (parentBlock == -1)
      {
        return -4; // Directory does not exist
      }
    }
  }

  if (findDirectory(parentBlock, newDirName) != -1)
  {
    return -1; // Directory already exists
  }
  // Allocate new sub directory
  int newBlock = allocateBlock();
  if (newBlock == -1)
  {
    return -2; // Space not available
  }

  // Initialize the new block as an empty directory
  char newDirBlock[64];
  for (int i = 0; i < 64; i++)
    newDirBlock[i] = '0';
  newDirBlock[64] = '\0';
  DirectoryInode *newDir1 = reinterpret_cast<DirectoryInode *>(newDirBlock);
  for (int i = 0; i < 10; i++)
  {
    newDir1->entries[i].entryName = '0'; // Empty slot
    newDir1->entries[i].entryType = '0'; // Undefined
    newDir1->entries[i].blockPointer = -1;
  }
  newDir1->nextDirBlock = -1; // No further overflow yet
  myPM->writeDiskBlock(newBlock, newDirBlock);
  int result = addEntryToDirectory(parentBlock, newDirName, newBlock, 'd');

  if (result != 0)
  {
    freeBlock(newBlock);
    return -2; // Directory Full
  }
  return (0);
}

int FileSystem::lockFile(char *filename, int fnameLen)
{
  if (!isValidFileName(filename, fnameLen))
  {

    return -4; // Invalid filename
  }
  int parentBlock = -1;
  int fileInodeBlock = findFile(filename, fnameLen, &parentBlock);
  if (fileInodeBlock == -1)
  {
    return -2; // Directory doesn't exist
  }
  if (fileInodeBlock == -2)
  {
    return -2; // File doesn't exist
  }
  char fileInodeDataBlock[64];
  for (int i = 0; i < 64; i++)
    fileInodeDataBlock[i] = '#';
  fileInodeDataBlock[64] = '\0';
  if (myPM->readDiskBlock(fileInodeBlock, fileInodeDataBlock) != 0)
  {

    return -4; // Error reading parent block
  }
  FileInode *fileInode = reinterpret_cast<FileInode *>(fileInodeDataBlock);
  // Check if file is already locked
  if (fileInode->lockId != -1)
  {
    return -1; // File already locked
  }
  // Check if it is currently open
  for (const auto &entry : openFileTable)
  {
    if (entry.fileInodeBlock == fileInodeBlock)
    {
      return -3; // File is currently open
    }
  }

  // Lock the file
  int lockId = rand() % 10000 + 1;
  fileInode->lockId = lockId;

  // Write back the updated inode back
  myPM->writeDiskBlock(fileInodeBlock, fileInodeDataBlock);
  return lockId;
}

int FileSystem::unlockFile(char *filename, int fnameLen, int lockId)
{

  int parentBlock = -1;
  int fileInodeBlock = findFile(filename, fnameLen, &parentBlock);
  if (fileInodeBlock == -1)
  {
    return -2; // File doesn't exist
  }
  char fileInodeDataBlock[64];
  for (int i = 0; i < 64; i++)
    fileInodeDataBlock[i] = '#';
  fileInodeDataBlock[64] = '\0';
  if (myPM->readDiskBlock(fileInodeBlock, fileInodeDataBlock) != 0)
  {
    return -2; // Error reading parent block
  }
  FileInode *fileInode = reinterpret_cast<FileInode *>(fileInodeDataBlock);
  if (lockId > 0 && fileInode->lockId == -1)
  {
    return -2; // File already locked
  }

  if (lockId != fileInode->lockId)
  {
    return -1; // Invalid lock id
  }

  // Unlock the file
  fileInode->lockId = -1;
  // Writing back to the inode block
  myPM->writeDiskBlock(fileInodeBlock, fileInodeDataBlock);

  return 0; // Success
}

int FileSystem::deleteFile(char *filename, int fnameLen)
{
  if (!isValidFileName(filename, fnameLen))
  {
    return -3; // Invalid filename
  }

  int parentBlock = -1;
  int fileInodeBlock = findFile(filename, fnameLen, &parentBlock);
  if (fileInodeBlock == -1)
  {
    return -1; // File does not exist
  }

  // Read the file inode block
  char fileInodeDataBlock[64];
  if (myPM->readDiskBlock(fileInodeBlock, fileInodeDataBlock) != 0)
  {
    return -3; // Error reading file inode block
  }

  FileInode *fileInode = reinterpret_cast<FileInode *>(fileInodeDataBlock);

  // Check if the file is locked
  if (fileInode->lockId != -1)
  {
    return -2; // File is locked
  }

  // Check if the file is currently open
  for (const auto &entry : openFileTable)
  {
    if (entry.fileInodeBlock == fileInodeBlock)
    {
      return -2; // File is currently open
    }
  }

  // Deallocate all allocated blocks
  for (int i = 0; i < 3; ++i)
  {
    if (fileInode->direct[i] != -1)
    {
      freeBlock(fileInode->direct[i]);
    }
  }

  if (fileInode->indirect != -1)
  {
    char indirectBlockData[64];
    if (myPM->readDiskBlock(fileInode->indirect, indirectBlockData) != 0)
    {
      return -3; // Error reading indirect block
    }

    int *indirectPointers = reinterpret_cast<int *>(indirectBlockData);
    for (int i = 0; i < 16; ++i)
    {
      if (indirectPointers[i] != -1)
      {
        freeBlock(indirectPointers[i]);
      }
    }

    freeBlock(fileInode->indirect); // Free the indirect block itself
  }

  // Free the file inode block
  freeBlock(fileInodeBlock);

  // Remove the file entry from the parent directory
  char parentBlockData[64];
  if (myPM->readDiskBlock(parentBlock, parentBlockData) != 0)
  {
    return -3; // Error reading parent directory block
  }

  DirectoryInode *parentDir = reinterpret_cast<DirectoryInode *>(parentBlockData);
  for (int i = 0; i < 10; ++i)
  {
    if (parentDir->entries[i].blockPointer == fileInodeBlock)
    {
      parentDir->entries[i].entryName = '0'; // Clear the entry
      parentDir->entries[i].blockPointer = -1;
      parentDir->entries[i].entryType = '0';
      myPM->writeDiskBlock(parentBlock, parentBlockData);
      return 0; // File deleted successfully
    }
  }

  return -3; // File could not be deleted
}

int FileSystem::deleteDirectory(char *dirname, int dnameLen)
{
  if (!isValidFileName(dirname, dnameLen))
  {
    return -3; // Invalid directory name
  }

  int parentBlock = 1;
  char dirToDelete = dirname[dnameLen - 1];
  for (int i = 1; i < dnameLen - 1; ++i)
  {
    if (dirname[i] == '/')
    {
      char dirName = dirname[i - 1];
      parentBlock = findDirectory(parentBlock, dirName);
      if (parentBlock == -1)
      {
        return -4; // Directory  doesn't exist
      }
    }
  }

  int dirBlock = findDirectory(parentBlock, dirToDelete); // Find the target directory block

  if (dirBlock == -1)
  {
    return -1; // Dir  doesn't exist
  }

  // Check if the directory is empty
  char dirBlockData[64];
  if (myPM->readDiskBlock(dirBlock, dirBlockData) != 0)
  {
    return -3; // Error reading directory block
  }

  DirectoryInode *dir = reinterpret_cast<DirectoryInode *>(dirBlockData);

  // Check if the directory has any entries
  for (int i = 0; i < 10; ++i)
  {
    if (dir->entries[i].entryName != '0')
    {
      return -2; // Directory is not empty
    }
  }

  // Handle overflow blocks
  int currentBlock = dir->nextDirBlock;
  vector<int> overflowBlocks;
  while (currentBlock != -1)
  {
    int nextBlock;
    char overflowBlockData[64];
    if (myPM->readDiskBlock(currentBlock, overflowBlockData) != 0)
    {
      return -3; // Error reading overflow block
    }

    DirectoryInode *overflowDir = reinterpret_cast<DirectoryInode *>(overflowBlockData);
    for (int i = 0; i < 10; ++i)
    {
      if (overflowDir->entries->entryName != '0')
      {
        return -2; // Directory not empty
      }
    }
    nextBlock = overflowDir->nextDirBlock;
    overflowBlocks.push_back(nextBlock);
    currentBlock = nextBlock;
  }
  // Deallocate all blocks
  for (int block : overflowBlocks)
  {
    freeBlock(block);
  }

  // Free the main directory block itself
  freeBlock(dirBlock);

  // Remove the directory entry from the parent directory
  char parentBlockData[64];
  if (myPM->readDiskBlock(parentBlock, parentBlockData) != 0)
  {
    return -3; // Error reading parent directory block
  }

  DirectoryInode *parentDir = reinterpret_cast<DirectoryInode *>(parentBlockData);
  for (int i = 0; i < 10; ++i)
  {
    if (parentDir->entries[i].blockPointer == dirBlock)
    {
      parentDir->entries[i].entryName = '0'; // Clear the entry
      parentDir->entries[i].blockPointer = -1;
      parentDir->entries[i].entryType = '0'; // Mark as empty
      myPM->writeDiskBlock(parentBlock, parentBlockData);
      return 0; // Directory deleted successfully
    }
  }

  return -3; // Directory could not be deleted
}

int FileSystem::openFile(char *filename, int fnameLen, char mode, int lockId)
{
  if (!isValidFileName(filename, fnameLen))
  {
    return -4; // Invalid file name or doesn't exist
  }
  if (mode != 'r' && mode != 'w' && mode != 'm')
  {
    return -2; // Invalid mode
  }
  int parentBlock = 1;
  int fileInodeBlock = findFile(filename, fnameLen, &parentBlock);
  if (fileInodeBlock == -2)
  {
    return -1; // File doesn't exist
  }

  char fileInodeDataBlock[64];
  for (int i = 0; i > 64; i++)
    fileInodeDataBlock[i] = '#';
  fileInodeDataBlock[64] = '\0';
  if (myPM->readDiskBlock(fileInodeBlock, fileInodeDataBlock) != 0)
  {
    return -4; // Error reading parent block
  }
  FileInode *fileInode = reinterpret_cast<FileInode *>(fileInodeDataBlock);

  if (fileInode->lockId != -1)
  {
    if (fileInode->lockId != lockId)
    {
      return -3; // File s locked, lock ID mismatch
    }
  }
  else
  {
    if (lockId != -1)
    {
      return -3; // File is not locked, invalid lock ID
    }
  }
  // Create a new file descriptor
  int fileDesc = fileDescCounter++; // Based on the table size
  // Everytime I open the file
  fileInode->openCount++;
  if (myPM->writeDiskBlock(fileInodeBlock, fileInodeDataBlock) != 0)
  {
    return -4; // Error writing inode block
  }
    OpenFileEntry newEntry;
    newEntry.fileDesc = fileDesc;
    newEntry.mode = mode;
    newEntry.rwPointer = 0; // Initially
    newEntry.fileInodeBlock = fileInodeBlock;

    // Add entry to open file table
    openFileTable.push_back(newEntry);
    return fileDesc;
}

int FileSystem::closeFile(int fileDesc)
{
  // Search the fileDesc in the open file
  for (size_t i = 0; i < openFileTable.size(); ++i)
  {
    if (openFileTable[i].fileDesc == fileDesc)
    {
      // Remove it from the open file Table
      openFileTable.erase(openFileTable.begin() + i);
      return 0; // File successfully closed
    }
  }

  return -1; // Invalid file description
  // TODO:
  // Find the use case of return -2
}

int FileSystem::readFile(int fileDesc, char *data, int len)
{
  if (len < 0)
  {
    return -2; // Invalid Length
  }
  // Locate the file in open file table
  for (auto &entry : openFileTable)
  {
    if (entry.fileDesc == fileDesc)
    {

      // Make sure it is readable
      if (entry.mode != 'r' && entry.mode != 'm')
      {
        return -3; // Operation not permitted
      }
      int fileInodeBlock = entry.fileInodeBlock;
      char fileInodeDataBlock[64];
      myPM->readDiskBlock(fileInodeBlock, fileInodeDataBlock);
      FileInode *fileInode = reinterpret_cast<FileInode *>(fileInodeDataBlock);

      /// Perform read operations using the rw Pointer
      int bytesRead = 0;
      char fileDataBlock[64];
      for (int i = 0; i < 64; i++)
        fileDataBlock[i] = '#';
      fileDataBlock[64] = '\0';
      while (bytesRead < len && entry.rwPointer < fileInode->size)
      {
        int blockOffset = entry.rwPointer % 64; // where to start in the block
        int blockNumber = entry.rwPointer / 64; // which block

        // If within the 3 direct block
        if (blockNumber < 3)
        {
          if (myPM->readDiskBlock(fileInode->direct[blockNumber], fileDataBlock) != 0)
          {
            return -4; // Error reading
          }
        }
        else if (blockNumber >= 3)
        {
          int indirectBlock = fileInode->indirect;
          if (indirectBlock == -1)
          {
            return -4; // No indirect block allocated yet
          }
          // Read the indirect block which holds the pointer to further data blocks
          char indirectBlockData[64];
          for (int i = 0; i < 64; i++)
            indirectBlockData[i] = '#';
          indirectBlockData[64] = '\0';

          if (myPM->readDiskBlock(fileInode->indirect, indirectBlockData) != 0)
          {
            return -4; // Error reading indirect block
          }
          // Find the correct pointer from the inirect block
          int indirectPointerIndex = blockNumber - 3;
          int datablockNumber = *reinterpret_cast<int *>(indirectBlockData + indirectPointerIndex * sizeof(int));
          // Read tthe data pointed by the indirect block
          if (myPM->readDiskBlock(datablockNumber, fileDataBlock) != 0)
          {
            return -4; // Error reading the data block
          }
        }
        // Copy data from the block into the buffer
        for (; blockOffset < 64 && bytesRead < len && entry.rwPointer < fileInode->size; ++blockOffset, ++bytesRead)
        {
          data[bytesRead] = fileDataBlock[blockOffset];
          entry.rwPointer++; // Move the read pointer
        }
      }
      return bytesRead;
    }
  }
  return -1; // Invalid file descriptor
}

int FileSystem::writeFile(int fileDesc, char *data, int len)
{
  if (len < 0)
  {
    return -2; // Invalid length
  }

  for (auto &entry : openFileTable)
  {
    if (entry.fileDesc == fileDesc)
    {
      int fileInodeBlock = entry.fileInodeBlock;
      // Read the file's inode block

      if (entry.mode != 'w' && entry.mode != 'm')
      {
        return -3; // Operation not permitted
      }
      char fileInodeData[64];
      myPM->readDiskBlock(fileInodeBlock, fileInodeData);
      FileInode *fileInode = reinterpret_cast<FileInode *>(fileInodeData);

      // Perform the write operation
      int bytesWritten = 0;
      char fileDataBlock[64]; // temporary buffer to store the file data

      while (bytesWritten < len)
      {
        int blockOffset = entry.rwPointer % 64;
        int blockNumber = entry.rwPointer / 64;

        // Write to the first 3 direct block decided by rwPointer
        if (blockNumber < 3)
        {
          if (fileInode->direct[blockNumber] == -1)
          {
            fileInode->direct[blockNumber] = allocateBlock(); // Allocate block if needed
            if (fileInode->direct[blockNumber] == -1)
            {
              return -4; // No space left
            }
          }

          if (myPM->readDiskBlock(fileInode->direct[blockNumber], fileDataBlock) != 0)
          {
            return -4; // Error reading block
          }
        }
        else if (blockNumber >= 3)
        {
          if (fileInode->indirect == -1)
          {
            fileInode->indirect = allocateBlock(); // Allocate indirect block if needed
            if (fileInode->indirect == -1)
            {
              return -4; // No space left
            }
            // Initialize indirect block
            char indirectBlockData[64];
            for (int i = 0; i < 64; i++)
              indirectBlockData[i] = '#';
            indirectBlockData[64] = '\0';
            std::fill_n(reinterpret_cast<int *>(indirectBlockData), 16, -1);
            if (myPM->writeDiskBlock(fileInode->indirect, indirectBlockData) != 0)
            {
              return -4; // Error writing indirect block
            }
          }
          // Read the indirect block (which holds pointers to further data blocks)
          char indirectBlockData[64];
          for (int i = 0; i < 64; i++)
            indirectBlockData[i] = '#';
          indirectBlockData[64] = '\0';
          if (myPM->readDiskBlock(fileInode->indirect, indirectBlockData) != 0)
          {
            return -4; // Error reading indirect block
          }
          // Find the correct pointer from the indirect block
          int indirectPointerIndex = blockNumber - 3;
          int *dataBlockNumber = reinterpret_cast<int *>(indirectBlockData) + indirectPointerIndex;

          // Allocate the data block if not already allocated
          if (*dataBlockNumber == -1)
          {
            *dataBlockNumber = allocateBlock();
            if (*dataBlockNumber == -1)
            {
              return -4; // No space left
            }

            if (myPM->writeDiskBlock(fileInode->indirect, indirectBlockData) != 0)
            {
              return -4; // Error writing indirect block
            }
          }
          // Read the data block pointed to by the indirect block
          if (myPM->readDiskBlock(*dataBlockNumber, fileDataBlock) != 0)
          {
            return -4; // Error reading data block
          }
        }
        // Write data into the block
        for (; blockOffset < 64 && bytesWritten < len; ++blockOffset, ++bytesWritten)
        {
          fileDataBlock[blockOffset] = data[bytesWritten];
          entry.rwPointer++; // Move the write pointer
        }
        // Write the block back to disk
        if (blockNumber < 3)
        {
          if (myPM->writeDiskBlock(fileInode->direct[blockNumber], fileDataBlock) != 0)
          {
            return -4; // Error writing direct block
          }
        }
        else
        {
          char indirectBlockData[64];
          if (myPM->readDiskBlock(fileInode->indirect, indirectBlockData) != 0)
          {
            return -4; // Error reading indirect block
          }
          int indirectPointerIndex = blockNumber - 3;
          int *dataBlockPointer = reinterpret_cast<int *>(indirectBlockData) + indirectPointerIndex;
          if (myPM->writeDiskBlock(*dataBlockPointer, fileDataBlock) != 0)
          {
            return -4; // Error writing indirect block
          }
        }
        // Update the inode block back to the disk
        fileInode->size = std::max(fileInode->size, entry.rwPointer);
      }
      // Write updated inode value
      if (myPM->writeDiskBlock(fileInodeBlock, fileInodeData) != 0)
      {
        return -4;
      }
      return bytesWritten;
    }
  }
  return -1; // Invalid file descriptor
}

int FileSystem::appendFile(int filedesc, char *data, int length)
{
  if (length < 0)
  {
    return -2; // Invalid length
  }

  // Find the open file entry in the OFT
  for (auto &entry : openFileTable)
  {
    if (entry.fileDesc == filedesc)
    {
      int fileInodeBlock = entry.fileInodeBlock;
      if (entry.mode != 'w' && entry.mode != 'm')
      {
        return -3; // Operation not permitted
      }

      // Read the file's inode block
      char fileInodeDataBlock[64];
      if (myPM->readDiskBlock(fileInodeBlock, fileInodeDataBlock) != 0)
      {
        return -3; // Error reading file inode block
      }

      FileInode *fileInode = reinterpret_cast<FileInode *>(fileInodeDataBlock);

      // Start appending from the end of the file
      entry.rwPointer = fileInode->size;

      // Perform the append operation
      int bytesAppended = 0;
      char fileDataBlock[64]; // Temporary buffer to store file data

      while (bytesAppended < length)
      {
        int blockOffset = entry.rwPointer % 64; // Offset within the block
        int blockNumber = entry.rwPointer / 64; // Determine which block

        // Allocate and write to the first 3 direct blocks
        if (blockNumber < 3)
        {
          if (fileInode->direct[blockNumber] == -1)
          {
            fileInode->direct[blockNumber] = allocateBlock(); // Allocate a new block
            if (fileInode->direct[blockNumber] == -1)
            {
              return -3; // No space left
            }
          }

          // Read the block to avoid overwriting existing data
          if (myPM->readDiskBlock(fileInode->direct[blockNumber], fileDataBlock) != 0)
          {
            return -3; // Error reading block
          }
        }
        // Handle indirect block allocation and writing
        else if (blockNumber >= 3)
        {
          if (fileInode->indirect == -1)
          {
            fileInode->indirect = allocateBlock(); // Allocate indirect block if needed
            if (fileInode->indirect == -1)
            {
              return -3; // No space left
            }

            // Initialize the indirect block
            char indirectBlockData[64] = {0};
            std::fill_n(reinterpret_cast<int *>(indirectBlockData), 16, -1); // Initialize pointers
            if (myPM->writeDiskBlock(fileInode->indirect, indirectBlockData) != 0)
            {
              return -3; // Error writing indirect block
            }
          }

          // Read the indirect block
          char indirectBlockData[64];
          if (myPM->readDiskBlock(fileInode->indirect, indirectBlockData) != 0)
          {
            return -3; // Error reading indirect block
          }

          // Find the correct pointer within the indirect block
          int indirectPointerIndex = blockNumber - 3;
          int *dataBlockPointer = reinterpret_cast<int *>(indirectBlockData) + indirectPointerIndex;

          // Allocate the data block if not already allocated
          if (*dataBlockPointer == -1)
          {
            *dataBlockPointer = allocateBlock();
            if (*dataBlockPointer == -1)
            {
              return -3; // No space left
            }

            // Write the updated indirect block back to disk
            if (myPM->writeDiskBlock(fileInode->indirect, indirectBlockData) != 0)
            {
              return -3; // Error writing indirect block
            }
          }

          // Read the data block pointed to by the indirect block
          if (myPM->readDiskBlock(*dataBlockPointer, fileDataBlock) != 0)
          {
            return -3; // Error reading data block
          }
        }

        // Append data into the block
        for (; blockOffset < 64 && bytesAppended < length; ++blockOffset, ++bytesAppended)
        {
          fileDataBlock[blockOffset] = data[bytesAppended];
          entry.rwPointer++; // Move the pointer
        }

        // Write the block back to disk
        if (blockNumber < 3)
        {
          if (myPM->writeDiskBlock(fileInode->direct[blockNumber], fileDataBlock) != 0)
          {
            return -3; // Error writing direct block
          }
        }
        else
        {
          char indirectBlockData[64];
          if (myPM->readDiskBlock(fileInode->indirect, indirectBlockData) != 0)
          {
            return -3; // Error reading indirect block
          }
          int indirectPointerIndex = blockNumber - 3;
          int *dataBlockPointer = reinterpret_cast<int *>(indirectBlockData) + indirectPointerIndex;
          if (myPM->writeDiskBlock(*dataBlockPointer, fileDataBlock) != 0)
          {
            return -3; // Error writing indirect block
          }
        }

        // Update the file size if necessary
        fileInode->size = std::max(fileInode->size, entry.rwPointer);
      }

      // Write the updated inode block back to disk
      if (myPM->writeDiskBlock(fileInodeBlock, fileInodeDataBlock) != 0)
      {
        return -3; // Error writing file inode block
      }

      return bytesAppended; // Return number of bytes appended
    }
  }

  return -1; // Invalid file descriptor
}

int FileSystem::truncFile(int fileDesc, int offset, int flag)
{
  for (auto &entry : openFileTable)
  {
    if (entry.fileDesc == fileDesc)
    {
      // Ensure the mode is not read-only
      if (entry.mode == 'r')
      {
        return -3; // Cannot truncate in read-only mode
      }

      char fileInodeData[64];
      if (myPM->readDiskBlock(entry.fileInodeBlock, fileInodeData) != 0)
      {
        return -1; // Error reading file inode block
      }

      FileInode *fileInode = reinterpret_cast<FileInode *>(fileInodeData);
      int rwPointer = entry.rwPointer;
      int fileSize = fileInode->size;

      // Calculate truncation pointer
      int truncPointer = (flag == 0) ? rwPointer + offset : offset;
      if (flag != 0 && flag != 1)
      {
        return -1; // Invalid flag
      }

      // Validate truncation pointer
      if (truncPointer < 0 || truncPointer > fileSize)
      {
        return -2; // Pointer out of bounds
      }

      // Calculate bytes to delete
      int bytesDeleted = fileSize - truncPointer;

      // Handle truncation in direct blocks
      for (int i = 0; i < 3; ++i)
      {
        int blockNumber = fileInode->direct[i];
        if (blockNumber == -1)
        {
          continue; // Skip unallocated blocks
        }

        if (truncPointer <= (i + 1) * 64)
        {
          // Truncate within this block
          int start = truncPointer - (i * 64);
          char buffer[64];
          myPM->readDiskBlock(blockNumber, buffer);

          // Clear bytes after the truncation point
          for (int j = start; j < 64; ++j)
          {
            buffer[j] = '#';
          }
          myPM->writeDiskBlock(blockNumber, buffer);

          // Free remaining direct blocks
          for (int j = i + 1; j < 3; ++j)
          {
            int toFree = fileInode->direct[j];
            if (toFree != -1)
            {
              myPM->returnDiskBlock(toFree);
              fileInode->direct[j] = -1; // Reset block reference
            }
          }
          break; // Direct truncation complete
        }
      }

      // Handle truncation in indirect blocks if necessary
      if (truncPointer > 3 * 64)
      {
        int indirectBlock = fileInode->indirect;
        if (indirectBlock != -1)
        {
          char indirectBlockData[64];
          myPM->readDiskBlock(indirectBlock, indirectBlockData);
          int *indirectPointers = reinterpret_cast<int *>(indirectBlockData);

          for (size_t i = 0; i < 64 / sizeof(int); ++i)
          {
            int blockNumber = indirectPointers[i];
            if (blockNumber == -1)
            {
              continue; // Skip unallocated blocks
            }

            if (truncPointer <= static_cast<int>((3 + i + 1) * 64))
            {
              // Truncate within this block
              int start = truncPointer - (3 + i) * 64;
              char buffer[64];
              myPM->readDiskBlock(blockNumber, buffer);

              // Clear bytes after the truncation point
              for (int j = start; j < 64; ++j)
              {
                buffer[j] = '#';
              }
              myPM->writeDiskBlock(blockNumber, buffer);

              // Free remaining blocks in the indirect block
              for (size_t j = i + 1; j < 64 / sizeof(int); ++j)
              {
                int toFree = indirectPointers[j];
                if (toFree != -1)
                {
                  myPM->returnDiskBlock(toFree);
                  indirectPointers[j] = -1; // Reset block reference
                }
              }

              // Write updated indirect block back to disk
              myPM->writeDiskBlock(indirectBlock, indirectBlockData);
              break;
            }
          }
        }
      }

      // Update file size and rwPointer
      fileInode->size = truncPointer;
      entry.rwPointer = truncPointer;

      // Write updated inode back to disk
      myPM->writeDiskBlock(entry.fileInodeBlock, fileInodeData);

      return bytesDeleted; // Return number of bytes truncated
    }
  }
  return -1; // File descriptor not found
}

int FileSystem::seekFile(int fileDesc, int offset, int flag)
{
  // Loop through the open file table to find the file descriptor
  for (auto &entry : openFileTable)
  {
    if (entry.fileDesc == fileDesc)
    {
      int fileInodeBlock = entry.fileInodeBlock;

      // Read the file's inode block
      char fileInodeDataBlock[64];
      if (myPM->readDiskBlock(fileInodeBlock, fileInodeDataBlock) != 0)
      {
        return -1; // Error reading file inode block
      }

      // Interpret the inode data
      FileInode *fileInode = reinterpret_cast<FileInode *>(fileInodeDataBlock);
      int rwPointer = entry.rwPointer;
      int fileSize = fileInode->size;

      // Handle the seek operation
      if (flag != 0) // Absolute positioning
      {
        if (offset < 0 || offset > fileSize)
        {
          if (offset < 0)
          {
            return -1; // Incorrect offset value.
          }
          return -2; // Out of bounds
        }
        entry.rwPointer = offset;
      }
      else // Relative positioning
      {
        int newPointer = rwPointer + offset;
        if (newPointer < 0 || newPointer > fileSize)
        {
          return -2; // Out of bounds
        }
        entry.rwPointer = newPointer;
      }

      // Success
      return 0;
    }
  }

  // File descriptor not found
  return -1;
}

int FileSystem::renameFile(char *filename1, int fnameLen1, char *filename2, int fnameLen2)
{
  if (!isValidFileName(filename1, fnameLen1) || !isValidFileName(filename2, fnameLen2))
  {
    return -1; // Invalid filename
  }
  int parentBlock1 = -1;
  int fileInode1 = findFile(filename1, fnameLen1, &parentBlock1);
  if (fileInode1 == -1 || fileInode1 == -2)
  {
    return -2; // File does not exist
  }

  // Check if file 2 already exists
  int parentBlock2 = -1;
  int fileInode2 = findFile(filename2, fnameLen2, &parentBlock2);
  if (fileInode2 != -2)
  {
    return -3; // File with new name already exists
  }
  if (fileInode1 == -4 || fileInode2 == -4)
  {
    return -5; // Directory does not exist
  }
  // Check if file is opened or locked
  for (const auto &entry : openFileTable)
  {
    if (entry.fileInodeBlock == fileInode1)
    {
      return -4; // File is open
    }
  }
  char fileInodeData[64];
  myPM->readDiskBlock(fileInode1, fileInodeData);
  FileInode *fileInode = reinterpret_cast<FileInode *>(fileInodeData);
  if (fileInode->lockId != -1)
  {
    return -4; // File is locked
  }

  // Update the parent director
  int currentBlock = parentBlock1;
  while (currentBlock != -1)
  {
    char parentBlockData[64];
    myPM->readDiskBlock(currentBlock, parentBlockData);
    DirectoryInode *parentDir = reinterpret_cast<DirectoryInode *>(parentBlockData);
    for (int i = 0; i < 10; ++i)
    {
      if (parentDir->entries[i].blockPointer == fileInode1)
      {
        parentDir->entries[i].entryName = filename2[fnameLen2 - 1];
        myPM->writeDiskBlock(parentBlock1, parentBlockData);
        return 0;
      }
    }
    currentBlock = parentDir->nextDirBlock;
  }
  return -5; // Others reasons
}

int FileSystem::renameDirectory(char *dirname1, int dnameLen1, char *dirname2, int dnameLen2)
{
  if (!isValidFileName(dirname1, dnameLen1) || !isValidFileName(dirname2, dnameLen2))
  {
    return -1; // Invalid directory name
  }
  char oldDir = dirname1[dnameLen1 - 1];
  char newDir = dirname2[dnameLen2];
  int parentBlock = 1;
  for (int i = 1; i < dnameLen1 - 1; ++i)
  {
    if (dirname1[i] == '/')
    {
      char dirName = dirname1[i - 1];
      parentBlock = findDirectory(parentBlock, dirName);
      if (parentBlock == -1)
      {
        return -4; // Directory does not exist
      }
    }
  }
  int dirBlock = findDirectory(parentBlock, oldDir);
  if (dirBlock == -1)
  {
    return -1;
  }
  if (findDirectory(parentBlock, newDir) != -1)
  {
    return -2;
  }
  int currentBlock = 1;
  while (currentBlock != -1)
  {
    char dirBlockData[64];
    if (myPM->readDiskBlock(currentBlock, dirBlockData) != 0)
    {
      return -4; // Error reading directory block
    }

    DirectoryInode *dir = reinterpret_cast<DirectoryInode *>(dirBlockData);
    for (int i = 0; i < 10; ++i)
    {
      if (dir->entries[i].blockPointer == dirBlock)
      {
        dir->entries[i].entryName = newDir; // Update directory name
        myPM->writeDiskBlock(currentBlock, dirBlockData);
        return 0; // Success
      }
    }

    currentBlock = dir->nextDirBlock; // Move to overflow block, if any
  }
  return -4;
}

int FileSystem::getAttribute(char *filename, int fnameLen, time_t *creationTime, int *openCount)
{
  if (!isValidFileName(filename, fnameLen))
  {
    return -1; // Invalid filename
  }
  int parentBlock = -1;
  int fileInodeBlock = findFile(filename, fnameLen, &parentBlock);
  if (fileInodeBlock == -1)
  {
    return -2;
  }
  char fileInodeData[64];
  if (myPM->readDiskBlock(fileInodeBlock, fileInodeData) != 0)
  {
    return -3;
  }
  FileInode *fileInode = reinterpret_cast<FileInode *>(fileInodeData);
  if (creationTime){
    *creationTime = fileInode->creationTime;

  }

  if (openCount){
    *openCount = fileInode->openCount;

  }
  return 0;

}

int FileSystem::setAttribute(char *filename, int fnameLen, time_t *newCreationTime, int *newOpenCount)
{
  if (!isValidFileName(filename, fnameLen))
  {
    return -1; // Invalid filename
  }

  int parentBlock = -1;
  int fileInodeBlock = findFile(filename, fnameLen, &parentBlock);
  if (fileInodeBlock == -1)
  {
    return -2; // File does not exist
  }

  char fileInodeData[64];
  if (myPM->readDiskBlock(fileInodeBlock, fileInodeData) != 0)
  {
    return -3; // Error reading inode block
  }

  FileInode *fileInode = reinterpret_cast<FileInode *>(fileInodeData);

  // Update attributes
  fileInode->openCount = *newOpenCount;
  fileInode->creationTime = *newCreationTime;

  // Write updated inode back
  if (myPM->writeDiskBlock(fileInodeBlock, fileInodeData) != 0)
  {
    return -4; // Error writing inode block
  }

  return 0; // Success
}
