#include "disk.h"
#include "diskmanager.h"
#include "partitionmanager.h"
#include <iostream>
#include <cstring>

using namespace std;

int main(){

    //Create a Disk and DiskManager
    Disk *d = new Disk(50, 64, const_cast<char *>("DISK1"));
    DiskPartition *dp = new DiskPartition[4];

    dp[0].partitionName = 'A';
    dp[0].partitionSize = 10;
    dp[1].partitionName = 'B';
    dp[1].partitionSize = 5;
    dp[2].partitionName = 'C';
    dp[2].partitionSize = 5;
    dp[3].partitionName = 'D';
    dp[3].partitionSize = 5;

    DiskManager *dm = new DiskManager(d, 4, dp);

    PartitionManager *pm = new PartitionManager(dm, 'B', 10);
    cout<<endl;

    cout<< "Testing Partition Manager for Partition B: \n";
    cout<<"Partation Size: "<<dm->getPartitionSize('B')<<endl;
    
    //Test 1: Allocate blocks
    //Get free blocks

    int block2 =  pm->getFreeDiskBlock();
    cout<<endl;
    int block3 =  pm->getFreeDiskBlock();
    cout<<endl;
    cout<<"Allocated Block 2: "<< block2 <<endl;
    cout<<"Allocated Block 3: "<< block3 <<endl;

    //Test2 : Wrting Data to Allocated Block
    char data1[64] = "Hello Block 2";
    char data2[64] = "Hello Block 3";
    pm->writeDiskBlock(block2,data1);
    pm->writeDiskBlock(block3, data2);
    cout<< "Data written to blocks"<<endl;
    //Test 3: Read Data from the Allocated Block
    char buffer[64];
    pm->readDiskBlock(block2,buffer);
    cout<<"Read from the block 2:" <<buffer <<endl;
    pm->readDiskBlock(block3,buffer);
    cout<<"Read from the block 3:" <<buffer <<endl;

    //Printing the bit vector
    // printBitVector(bvA,100);
    //Test 4: Free a Block
    pm->returnDiskBlock(block2);
    cout<<endl;
    cout<<"Freed Block 2 .\n";
    // printBitVector(bvA,100);

    //Test 5: REallocate the Freed Block
    int block4 = pm->getFreeDiskBlock();
    cout<<endl;
    cout<<"Realocated Block: " <<block4 <<endl;

    if(block4 == block2){
        cout<< " Block 2 was successfully reallocated"<<endl;
    }else{
        cout<<"Error! Block 2 is not allocated";
    }


    //Test 6 Write Data to reallocated data block
    char data3[64] = " Hello Reallocated Block";
    pm->writeDiskBlock(block4, data3);

    //Read back the data
    pm->readDiskBlock(block4, buffer);
    cout<< "Read from Reallocated Block: "<<buffer <<endl;


    return 0;
    




}