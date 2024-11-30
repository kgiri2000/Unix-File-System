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
    char rootDirBlock[64]; // Clear the buffer to avoid garbage data
    for(int i = 0; i<64; i++)rootDirBlock[i] = '#';
    rootDirBlock[64] = '\0';
    char temp[64];
    for(int i = 0; i<64; i++)temp[i] = '#';
    cout<<"Initialize Temp: "<<temp<<endl;

    DirectoryInode *rootDir = reinterpret_cast<DirectoryInode* >(rootDirBlock);
    rootDir->nextDirBlock = -1;
    for(int i = 0; i<10;i++){

        rootDir->entries[i].entryName = 'a';
        rootDir->entries[i].entryType = 'd';
        rootDir->entries[i].blockPointer = -1;

    }

    char fileInodeBlock[64];
    char art[44];
    for(int i = 0; i<44;i++)art[i] = 'j';
    FileInode *fileDir = reinterpret_cast<FileInode*>(fileInodeBlock);
    fileDir->name = 'v';
    fileDir->type = 'f';
    fileDir->size = 2;
    fileDir->direct[0] = 1;
    fileDir->direct[1] = 1;
    fileDir->direct[2] = 2;
    fileDir->indirect = 0;
    fileDir->time = 't';
    fileDir->modified = 'y';

    myPM->writeDiskBlock(2,fileInodeBlock);


    myPM->writeDiskBlock(1, rootDirBlock);
    myPM->readDiskBlock(1, temp);
    cout<<"Block 1 Data in Temp: "<<temp<<endl;

    char temp1[64] = {0};
    myPM->readDiskBlock(1, temp1);
    cout << "Block 1 Data: ";
    for (int i = 0; i < 64; ++i) {
        cout << temp1[i];
    }



}