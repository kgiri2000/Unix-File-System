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
    char finode[64];
    for(int i = 0; i<64; i++)finode[i] = '#';
    finode[64] = '\0';

    DirectoryInode *rootDir = reinterpret_cast<DirectoryInode* >(rootDirBlock);
    rootDir->nextDirBlock = -1;
    for(int i = 0; i<10;i++){

        rootDir->entries[i].entryName = 'a';
        rootDir->entries[i].entryType = 'd';
        rootDir->entries[i].blockPointer = -1;

    }

    char fileInodeBlock[64];
    for(int i = 0; i<64;i++)fileInodeBlock[i] = '#';
    fileInodeBlock[64] = '\0';
    FileInode *fileDir = reinterpret_cast<FileInode*>(fileInodeBlock);

    fileDir->name = 'v'; //1
    fileDir->type = 'f'; //2
    fileDir->size = 2; //6
    fileDir->direct[0] = 1;//10
    fileDir->direct[1] = 1;//14
    fileDir->direct[2] = 2;//18
    fileDir->indirect = 0; //22
    fileDir->time = 't'; //23
    fileDir->direct1[0] = 1;//10
    fileDir->direct1[1] = 1;//14
    fileDir->direct1[2] = 2;//18
    fileDir->direct1[3] = 1;//10
    fileDir->direct1[4] = 1;//14
    fileDir->direct1[5] = 2;//18
    fileDir->direct1[6] = 1;//10
    fileDir->direct1[7] = 1;//14
    fileDir->direct1[8] = 2;//18
    fileDir->direct1[9] = 5;
    fileDir->direct1[10] = 90;
    fileDir->last = 'e';



    myPM->writeDiskBlock(2,fileInodeBlock);
    myPM->writeDiskBlock(1, rootDirBlock);
    myPM->readDiskBlock(2, finode);
    
    cout<<"Block 2 Data File inode: ";
    for (int i = 0; i < 64; ++i) {
        cout << finode[i];
    }
    cout<<endl;



    char temp1[64];
    for(int i = 0; i< 64;i++)temp1[i]='#';
    temp1[64] = '\0';

    myPM->readDiskBlock(1, temp1);
    cout << "Block 1 Data: ";
    for (int i = 0; i < 64; ++i) {
        cout << temp1[i];
    }
    cout<<endl;


    cout<<"Sizes"<<endl;
    cout << "Size of DirectoryEntry: " << sizeof(DirectoryEntry) << endl;
    cout << "Size of FileInode: " << sizeof(FileInode) << endl;
    cout << "Size of DirectoryInode: " << sizeof(DirectoryInode) << endl;
    cout << "Offset of entryName: " << offsetof(DirectoryEntry, entryName) << endl;
    cout << "Offset of entryType: " << offsetof(DirectoryEntry, entryType) << endl;
    cout << "Offset of blockPointer: " << offsetof(DirectoryEntry, blockPointer) << endl;

    //Reading data and verifing
    DirectoryInode *rootDir1 =  reinterpret_cast<DirectoryInode*>(temp1);

    for(int i = 0; i< 10; i++){
        cout << ((rootDir1->entries[i].entryName == 'a') ? "True" : "False") << endl;
        cout << ((rootDir1->entries[i].entryType == 'd') ? "True" : "False") << endl;
        cout << ((rootDir1->entries[i].blockPointer == -1) ? "True" : "False") << endl;


    }
    cout<< ((rootDir1->nextDirBlock == -1) ? "True": "False") <<endl;

    //Reading and  verifying file inode
    cout<<"Reading from file-inode"<<endl;
    FileInode *fnode = reinterpret_cast<FileInode*>(finode);
    cout << ((fnode->name == 'v') ? "True" : "False" ) <<endl;
    cout << ((fnode->type == 'f') ? "True" : "False" ) <<endl;
    cout << ((fnode->size == 2) ? "True" : "False" ) <<endl;


    //Readin what is initially stored
    cout<<"Reeading only initialized root directory"<<endl;
    char newtemp[64];
    for(int i = 0; i< 64; i++)newtemp[i] = '0';
    DirectoryInode *newDir =  reinterpret_cast<DirectoryInode*>(newtemp);
    myPM->writeDiskBlock(3, newtemp);

    //Reading before writing in the block
    cout<<"Reading before writing in the block"<<endl;
    for(int i = 0; i< 10; i++){
        cout << newDir->entries[i].entryName  << endl;
        cout << newDir->entries[i].entryType << endl;
        cout << newDir->entries[i].blockPointer  << endl;
    }
    cout<<"Reading after writing in the block"<<endl;
    char newtemp1[64];
    for(int i = 0; i< 64; i++)newtemp1[i] = '0';
    myPM->readDiskBlock(3, newtemp1);
    DirectoryInode *newDir1 =  reinterpret_cast<DirectoryInode*>(newtemp1);
    for(int i = 0; i< 10; i++){
        cout << newDir1->entries[i].entryName  << endl;
        cout << newDir1->entries[i].entryType << endl;
        cout << newDir1->entries[i].blockPointer  << endl;
    }

}