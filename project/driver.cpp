
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
  Client *c5 = new Client(fs2);
  Client *c6 = new Client(fs3);
  Client *c7 = new Client(fs4);

  int i, r, l1, l2, f1, f2, f3, f4, f5;
  char buf1[37], buf2[64], buf3[100], buf4[600], buf5[1200];
  char rbuf1[37], rbuf2[64], rbuf3[100], rbuf4[600], rbuf5[1200];
  
  c2->myFS->createFile(const_cast<char *>("/a"), 2);
  c4->myFS->createFile(const_cast<char *>("/a"), 2);
  int fd = c2->myFS->openFile(const_cast<char *>("/b"), 2, 'w', -1);
  c2->myFS->writeFile(fd, const_cast<char *>("aaaabbbbcccc"), 12);
  c2->myFS->closeFile(fd);

  return 0;
}
