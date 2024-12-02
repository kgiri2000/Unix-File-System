#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include "disk.h"
#include "diskmanager.h"
#include "partitionmanager.h"
#include "filesystem.h"
#include "client.h"

using namespace std;

/*
  This driver will test the getAttributes() and setAttributes()
  functions. You need to complete this driver according to the
  attributes you have implemented in your file system, before
  testing your program.
  
  
  Required tests:
  get and set on the fs1 on a file
    and on a file that doesn't exist
    and on a file in a directory in fs1
    and on a file that doesn't exist in a directory in fs1

 fs2, fs3, fs4
  on a file both get and set on both fs2, fs3, and fs4

  samples are provided below.  Use them and/or make up your own.


*/

int main()
{
    Disk *d = new Disk(350, 64, const_cast<char *>("DISK1"));
    DiskPartition *dp = new DiskPartition[4];

    dp[0].partitionName = 'A';
    dp[0].partitionSize = 100;
    dp[1].partitionName = 'B';
    dp[1].partitionSize = 75;
    dp[2].partitionName = 'C';
    dp[2].partitionSize = 105;
    dp[3].partitionName = 'D';
    dp[3].partitionSize = 25;

    DiskManager *dm = new DiskManager(d, 4, dp);
    FileSystem *fs1 = new FileSystem(dm, 'A');
    FileSystem *fs2 = new FileSystem(dm, 'B');
    FileSystem *fs3 = new FileSystem(dm, 'C');
    FileSystem *fs4 = new FileSystem(dm, 'D');

    Client *c1 = new Client(fs1);
    Client *c2 = new Client(fs2);
    Client *c3 = new Client(fs3);
    Client *c4 = new Client(fs1);
    Client *c5 = new Client(fs1);
    Client *c6 = new Client(fs2);
    Client *c7 = new Client(fs3);
    Client *c8 = new Client(fs4);

    int r;
    time_t creationTime;
    int openCount;

    // Set and Get Attributes for fs1
    cout << "Testing set and get attributes on fs1..." << endl;

    // Create file /e/f and set attributes
    r = c1->myFS->createFile(const_cast<char *>("/e/f"), 4);
    cout << "Create /e/f: " << (r == 0 ? "Success" : "Fail") << endl;

    time_t currentTime = time(nullptr);
    r = c1->myFS->setAttribute(const_cast<char *>("/e/f"), 4, &currentTime, new int(5));  // Example: new open count
    cout << "Set attributes for /e/f: " << (r == 0 ? "Success" : "Fail") << endl;

    // Get attributes for /e/f
    r = c1->myFS->getAttribute(const_cast<char *>("/e/f"), 4, &creationTime, &openCount);
    cout << "Get attributes for /e/f: ";
    if (r == 0) {
        cout << "Creation Time: " << ctime(&creationTime) << "Open Count: " << openCount << endl;
    } else {
        cout << "Error, code: " << r << endl;
    }

    // Test invalid file /p (file doesn't exist)
    r = c1->myFS->getAttribute(const_cast<char *>("/p"), 2, &creationTime, &openCount);
    cout << "Get attributes for /p (should fail): " << (r == -2 ? "Success" : "Fail") << endl;

    r = c4->myFS->setAttribute(const_cast<char *>("/p"), 2, &currentTime, new int(5));
    cout << "Set attributes for /p (should fail): " << (r == -2 ? "Success" : "Fail") << endl;

    // Set and Get Attributes for fs2
    cout << "\nTesting set and get attributes on fs2..." << endl;

    // Create file /f and set attributes
    r = c2->myFS->createFile(const_cast<char *>("/f"), 2);
    cout << "Create /f: " << (r == 0 ? "Success" : "Fail") << endl;

    r = c2->myFS->setAttribute(const_cast<char *>("/f"), 2, &currentTime, new int(3));  // New open count
    cout << "Set attributes for /f: " << (r == 0 ? "Success" : "Fail") << endl;

    // Get attributes for /f
    r = c2->myFS->getAttribute(const_cast<char *>("/f"), 2, &creationTime, &openCount);
    cout << "Get attributes for /f: ";
    if (r == 0) {
        cout << "Creation Time: " << ctime(&creationTime) << "Open Count: " << openCount << endl;
    } else {
        cout << "Error, code: " << r << endl;
    }

    // Set and Get Attributes for fs3
    cout << "\nTesting set and get attributes on fs3..." << endl;

    // Create file /o/o/o/a/l
    r = c3->myFS->createFile(const_cast<char *>("/o/o/o/a/l"), 9);
    cout << "Create /o/o/o/a/l: " << (r == 0 ? "Success" : "Fail") << endl;

    r = c3->myFS->setAttribute(const_cast<char *>("/o/o/o/a/l"), 9, &currentTime, new int(4));  // New open count
    cout << "Set attributes for /o/o/o/a/l: " << (r == 0 ? "Success" : "Fail") << endl;

    r = c3->myFS->getAttribute(const_cast<char *>("/o/o/o/a/l"), 9, &creationTime, &openCount);
    cout << "Get attributes for /o/o/o/a/l: ";
    if (r == 0) {
        cout << "Creation Time: " << ctime(&creationTime) << "Open Count: " << openCount << endl;
    } else {
        cout << "Error, code: " << r << endl;
    }

    // Set and Get Attributes for fs4
    cout << "\nTesting set and get attributes on fs4..." << endl;

    // Create file /o/o/o/a/d
    r = c8->myFS->createFile(const_cast<char *>("/o/o/o/a/d"), 9);
    cout << "Create /o/o/o/a/d: " << (r == 0 ? "Success" : "Fail") << endl;

    r = c8->myFS->setAttribute(const_cast<char *>("/o/o/o/a/d"), 9, &currentTime, new int(6));  // New open count
    cout << "Set attributes for /o/o/o/a/d: " << (r == 0 ? "Success" : "Fail") << endl;

    r = c8->myFS->getAttribute(const_cast<char *>("/o/o/o/a/d"), 9, &creationTime, &openCount);
    cout << "Get attributes for /o/o/o/a/d: ";
    if (r == 0) {
        cout << "Creation Time: " << ctime(&creationTime) << "Open Count: " << openCount << endl;
    } else {
        cout << "Error, code: " << r << endl;
    }

    return 0;
}
