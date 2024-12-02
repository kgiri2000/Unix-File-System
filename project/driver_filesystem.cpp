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
  cout <<"begin driver 2\n";
  cout <<"\nDid the data survive on the disk?\n";
  r = c1->myFS->createFile(const_cast<char *>("/A"), 2);
  cout << "rv from createFile /A fs1 is " << r <<(r==0 ? " correct ": " fail") <<endl;
  r = c1->myFS->createFile(const_cast<char *>("/B"), 2);
  cout << "rv from createFile /B fs1 is " << r <<(r==0 ? " correct ": " fail") <<endl;
  r = c1->myFS->renameFile(const_cast<char *>("/A"), 2, const_cast<char *>("/D"), 2);
  cout<<r<<endl;


  // cout<<"Creating Files and Directories"<<endl;
  // res=c1->myFS->createDirectory(const_cast<char *>("/d"),2);
  // cout<<"Create Directory d:"<< (res == 0? "Success": "Failure")<<endl;
  // res=c1->myFS->createDirectory(const_cast<char *>("/d/d"),4);
  // cout<<"Create Directory d:"<< (res == 0? "Success": "Failure")<<endl;
  // res=c1->myFS->createDirectory(const_cast<char *>("/g"),2);
  // cout<<"Create Directory g:"<< (res == 0? "Success": "Failure")<<endl;
  // res=c1->myFS->createFile(const_cast<char *>("/e"), 2);
  // cout<<"Create File:"<<(res == 0? " Success": "Failure")<<endl;
  // res=c1->myFS->createFile(const_cast<char *>("/d/a"), 4);
  // cout<<"Create File:"<<(res == 0? " Success": "Failure")<<endl;
  // res=c1->myFS->createDirectory(const_cast<char *>("/d/e/f"), 6);
  // cout<<"Create Directory:"<<(res == -4? " Invalid Directory": "Failure")<<endl;
  // res=c1->myFS->createFile(const_cast<char *>("/d/d/e"), 6);
  // cout<<"Create File:"<<(res == 0? "Success": "Failure")<<endl;
  // res=c1->myFS->createDirectory(const_cast<char *>("/d"), 2);
  // cout<<"Create Directory:"<<(res == -1? " Already exist": "Failure")<<endl;
  // res=c1->myFS->createFile(const_cast<char *>("/d/a"), 4);
  // cout<<"Create File:"<<(res == -1? " Already exist": "Failure")<<endl;

  // cout<<"Deleting Files and Directories"<<endl;
  // res = c1->myFS->deleteFile(const_cast<char *>("/d/d/e"),6);
  // cout<<"Delete File e:"<< (res == 0? "Success": "Failure")<<endl;

  // res = c1->myFS->deleteDirectory(const_cast<char *>("/d/d"),4);
  // cout<<"Delete Directory g:"<< (res == 0? "Success": "Failure")<<endl;
  // cout<<res<<endl;

  // res = c1->myFS->deleteFile(const_cast<char *>("/d/a"),4);
  // cout<<"Delete File e:"<< (res == 0? "Success": "Failure")<<endl;
  // int lockid;
  // lockid = c1->myFS->lockFile(const_cast<char *>("/e"), 2);
  // cout<<"File Locked: " <<lockid<<(lockid> 0? " Success": "Failure")<<endl;

  // res = c1->myFS->lockFile(const_cast<char *>("/e"), 2);
  // cout<<"File Locked: " <<res<<(res == -1? "Success: Already Locked": "Failure")<<endl;

  // res = c1->myFS->lockFile(const_cast<char *>("/z"), 2);
  // cout<<"File Locked: " <<res<<(res == -2? "Success: Doesn't Exist": "Failure")<<endl;

  // //Unlocking the file
  // // 0, -1: invalid lockId, -2 for other reason
  // res = c1->myFS->unlockFile(const_cast<char *>("/e"), 2, 667 );
  // cout<<"File Unlocked: " <<res<<(res == -1? " Success: Invalid lockID": "Failure")<<endl;

  // res = c1->myFS->unlockFile(const_cast<char *>("/e"), 2,lockid );
  // cout<<"File Unlocked: " <<res<<(res == 0? " Success: Unlocked": "Failure")<<endl;

  // res = c1->myFS->unlockFile(const_cast<char *>("/e"), 2, 667 );
  // cout<<"File Unlocked: " <<res<<(res == -2? " Success: Already Unlocked": "Failure")<<endl;

  // //Opening the file
  // res=c1->myFS->createFile(const_cast<char *>("/d/a"), 4);
  // cout<<"Create File:"<<(res == 0? " Success": "Failure")<<endl;

  // lockid = c1->myFS->lockFile(const_cast<char *>("/d/a"), 4);
  // cout<<"File Locked: " <<lockid<<(lockid> 0? " Success": "Failure")<<endl;

  // int fd;
  // fd = c1->myFS->openFile(const_cast<char *>("/d/a"),4, 'r', lockid );
  // cout<<"File Opened: " <<fd<<(fd>0? " Success": " Failure")<<endl;

  // fd = c1->myFS->openFile(const_cast<char *>("/e"),2, 'r', -1 );
  // cout<<"File Opened: " <<fd<<(fd>0? " Success": " Failure")<<endl;

  // //Deleting and locking the open file
  // int l;
  // l = c1->myFS->lockFile(const_cast<char *>("/e"), 2);
  // cout<<"File Locked: " <<l << (l ==-3 ? " Success: File is open" : "Failure")<<endl;


  // res = c1->myFS->deleteFile(const_cast<char *>("/e"), 2);
  // cout<<"File Locked: " <<res << (res ==-2 ? " Success: File is open/locked" : "Failure")<<endl;

  // //Closing the file

  // res = c1->myFS->closeFile(fd);
  // cout<<"File Closed: " <<res << (res == 0 ? " Success: File is closed" : "Failure")<<endl;

  // //Lock again since it is closed
  // l = c1->myFS->lockFile(const_cast<char *>("/e"), 2);
  // cout<<"File Locked: " <<l << (l> 0 ? " Success: File is locked" : "Failure")<<endl;

  // fd = c1->myFS->openFile(const_cast<char *>("/e"),2, 'm', l );
  // cout<<"File Opened: " <<fd<<(fd>0? " Success": " Failure")<<endl;

  // //Writing
  // char hello[500];
  // for(int i = 0; i< 500; ++i){
  //   hello[i] = 'd';
  // }
  // char hello1[500];
  // for(int i = 0; i< 500; ++i){
  //   hello1[i] = 'e';
  // }
  // char hello2[500];
  // for(int i = 0; i< 500; ++i){
  //   hello2[i] = 'a';
  // }
  


  // res = c1->myFS->writeFile(fd, hello,  64);
  // cout<<"File Written: " <<res<<(res == 64? " Success": " Failure")<<endl;

  // //Appending
  // res = c1->myFS->appendFile(fd, hello1,  64);
  // cout<<"File Written: " <<res<<(res == 64? " Success": " Failure")<<endl;

  // res = c1->myFS->writeFile(fd, hello2,  64);
  // cout<<"File Written: " <<res<<(res == 64? " Success": " Failure")<<endl;

  // res = c1->myFS->writeFile(fd, hello,  64);
  // cout<<"File Written: " <<res<<(res == 64? " Success": " Failure")<<endl;

  // //Reading
  // char message[500];
  // for(int i = 0; i< 500; ++i){
  //   message[i] = '#';
  // }
  

  // //Closing this file
  // res = c1->myFS->closeFile(fd);
  // cout<<"File Closed: " <<res<<(res== 0? " Success": " Failure")<<endl;

  // fd = c1->myFS->openFile(const_cast<char *>("/e"),2, 'm', l );
  // cout<<"File Opened: " <<fd<<(fd>0? " Success": " Failure")<<endl;

  // res = c1->myFS->readFile(fd, message,  5);
  // cout<<"File Read: " <<res<<(res == 5? " Success": " Failure")<<endl;

  // res = c1->myFS->readFile(fd, message,  5);
  // cout<<"File Read: " <<res<<(res == 5? " Success": " Failure")<<endl;

  // res = c1->myFS->writeFile(fd, hello1,  5);
  // cout<<"File Written: " <<res<<(res == 5? " Success": " Failure")<<endl;

  // //Seek 
  // res = c1->myFS->seekFile(fd, 5, 5);
  // cout<<"File seek at 5 "<< res <<(res == 0? "Success" : "Failure")<<endl;

  // res = c1->myFS->writeFile(fd, hello1,  5);
  // cout<<"File Written: " <<res<<(res == 5? " Success": " Failure")<<endl;

  // res = c1->myFS->seekFile(fd, 600, 5);
  // cout<<"File seek at 600 "<< res <<(res == -2? "Success" : "Failure")<<endl;

  // res = c1->myFS->seekFile(fd, 10, 0);
  // cout<<"File seek at 15 "<< res <<(res == 0? "Success" : "Failure")<<endl;

  // res = c1->myFS->writeFile(fd, hello1,  5);
  // cout<<"File Written: " <<res<<(res == 5? " Success": " Failure")<<endl;

  // res = c1->myFS->seekFile(fd, -30, 0);
  // cout<<"File seek at -5 "<< res <<(res == -2? "Success" : "Failure")<<endl;


















  













  


  delete []dp;
  delete d;
  delete dm;
  delete fs1;


}