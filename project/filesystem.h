#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <vector>
using namespace std;

//File i-node
struct FileInode{
  char name; //1 byte
  char type; //1 byte
  int size; //4 bytes
  int direct[3]; //12 bytes
  int indirect; // 4 bytes
  char attributes[44];

};

//Directory entry structure
struct DirectoryEntry{
  char enntryName;
  int blockPointer;
  char entryType;
};

//Directory i-node Structure
struct DirectoryInode{
  char name;
  char type;
  DirectoryEntry entries[10];
  int nextDirBlock;
};

//Indirect i-node structure
struct IndirectInode{
  int blockPointers[16];
};

class FileSystem {
  DiskManager *myDM;
  PartitionManager *myPM;
  char myfileSystemName;
  int myfileSystemSize;
  /* declare other private members here */

  //Open File Entry to store file description at run time
  struct OpenFileEntry{
    int fileDesc;
    int rwPointer;
    char mode;
    bool isLocked;
    int lockId;
  };

  //Vector to store open file table with open file entry object
  std::vector<OpenFileEntry> openFileTable;

  //Helper Functions
  bool isValidFileName(const char *filename, int fileLen);
  int findFile(const char *filename);
  int allocateBlock();
  void freeBlock(int blknum);
  int allocateFileBlocks(int fileSize, std::vector<int> &allocatedBlocks);

  public:
    FileSystem(DiskManager *dm, char fileSystemName);
    int createFile(char *filename, int fnameLen);
    int createDirectory(char *dirname, int dnameLen);
    int lockFile(char *filename, int fnameLen);
    int unlockFile(char *filename, int fnameLen, int lockId);
    int deleteFile(char *filename, int fnameLen);
    int deleteDirectory(char *dirname, int dnameLen);
    int openFile(char *filename, int fnameLen, char mode, int lockId);
    int closeFile(int fileDesc);
    int readFile(int fileDesc, char *data, int len);
    int writeFile(int fileDesc, char *data, int len);
    int appendFile(int fileDesc, char *data, int len);
    int seekFile(int fileDesc, int offset, int flag);
    int truncFile(int fileDesc, int offset, int flag);
    int renameFile(char *filename1, int fnameLen1, char *filename2, int fnameLen2);
    int renameDirectory(char *dirname1, int dnameLen1, char *dirname2, int dnameLen2);
    int getAttribute(char *filename, int fnameLen /* ... and other parameters as needed */);
    int setAttribute(char *filename, int fnameLen /* ... and other parameters as needed */);

    /* declare other public members here */

};
#endif
