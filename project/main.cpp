// Main function tosimulate disk
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include "disk.h"
#include "diskmanager.h"
using namespace std;

int main() {
    // Create a disk with 10 blocks, each of size 512 bytes, stored in "virtualdisk"
    Disk myDisk(10, 64, (char*)"virtualdisk");

    // Initialize the disk
    int initResult = myDisk.initDisk();
    if (initResult == 1) {
        cout << "Disk initialized." << endl;
    } else if (initResult == 0) {
        cout << "Disk already exists." << endl;
    } else {
        cerr << "Failed to initialize disk." << endl;
        return 1;
    }

    // Write a block
    char writeData[64] = "Testing";
    if (myDisk.writeDiskBlock(3, writeData) == 0) {
        cout << "Block written successfully." << endl;
    } else {
        cerr << "Failed to write block." << endl;
    }

    // Read the block back
    char readData[64] = {0}; // Initialize buffer
    if (myDisk.readDiskBlock(3, readData) == 0) {
        cout << "Block read successfully: " << readData << endl;
    } else {
        cerr << "Failed to read block." << endl;
    }

    return 0;
}
