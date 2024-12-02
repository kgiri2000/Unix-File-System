/* This is an example of a driver to test the filesystem */

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "disk.h"
#include "diskmanager.h"
#include "partitionmanager.h"
#include "filesystem.h"
#include "client.h"
using namespace std;

int main()
{
    Disk *d = new Disk(100, 64, const_cast<char *>("DISK1"));
    DiskPartition *dp = new DiskPartition[4];

    dp[0].partitionName = 'A';
    dp[0].partitionSize = 50;
    dp[1].partitionName = 'B';
    dp[1].partitionSize = 50;
    // dp[2].partitionName = 'C';
    // dp[2].partitionSize = 105;
    // dp[3].partitionName = 'D';
    // dp[3].partitionSize = 25;

    DiskManager *dm = new DiskManager(d, 4, dp);
    FileSystem *fs1 = new FileSystem(dm, 'A');
    FileSystem *fs2 = new FileSystem(dm, 'B');
    int r;
    Client *c1 = new Client(fs1);
    Client *c2 = new Client(fs2);
    cout << "begin driver 2\n";
    cout << "\nDid the data survive on the disk?\n";
    r = c1->myFS->createFile(const_cast<char *>("/A"), 2);
    cout << "rv from createFile /A fs1 is " << r << (r == 0 ? " correct " : " fail") << endl;
    r = c1->myFS->createFile(const_cast<char *>("/a"), 2);
    cout << "rv from createFile /B fs1 is " << r << (r == 0 ? " correct " : " fail") << endl;

    r = c1->myFS->deleteFile(const_cast<char *>("/A"), 2);
    cout << r << endl;
}