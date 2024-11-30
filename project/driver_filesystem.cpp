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
  int res;
  res = fs1->createFile(const_cast<char *>("/a"), 2);
  if(res==0){
    cout<<"File Created from Driver"<<endl;
  }

  res = fs1->createDirectory(const_cast<char *>("/b"), 2);
  if(res==0){
    cout<<"Directory Created from Driver"<<endl;
  }
  res = fs1->createDirectory(const_cast<char*>("/c"), 2);
  if(res==0){
    cout<<"Directory Created from Driver"<<endl;
  }
  res = fs1->createFile(const_cast<char*>("/b/d"), 4);
  if(res==0){
    cout<<"Directory Created from Driver"<<endl;
  }


  delete []dp;
  delete d;
  delete dm;
  delete fs1;


}