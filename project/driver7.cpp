
/* Driver 5*/

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
 

  int i, r, l1, l2, f1, f2, f3, f4, f5, f6;
  char buf1[37], buf2[64], buf3[600], buf4[600];
  char rbuf1[37], rbuf2[64], rbuf3[600], rbuf4[1216];

  for (i = 0; i < 37; i++) buf1[i] = 'P';
  for (i = 0; i < 64; i++) buf2[i] = 'T';
  for (i = 0; i < 600; i++) buf3[i] = 'F';
  for (i = 0; i < 600; i++) buf4[i] = 'I';

  
  cout <<"Driver 7 begins\n";

  cout <<"Fill a partition test! This assumes driver1-6 were run\n";
  cout <<"if there are minor differences, you will have some serious explaining to do!\n";

  cout <<"First up, fs4 file test.\n";
   r = c7->myFS->createFile(const_cast<char *>("/f/a"), 4);
  cout << "rv from createFile /f/a is " << r <<(r==0 ? " correct": " fail") <<endl;

  f1 = c7->myFS->openFile(const_cast<char *>("/f/a"), 4, 'w', -1);
  cout << "rv from /f/a openFile is " << f1 << (f1>0 ? " correct": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /f/a is " << r <<(r==-3 ? " correct partition is full": " fail")<< endl;
  r = c7->myFS->closeFile(f1);
  cout << "rv from closeFile /f/a is " << r << (r==0 ? " correct":" fail")<<endl;

  f1 = c7->myFS->openFile(const_cast<char *>("/f/a"), 4, 'r', -1);
  cout << "rv from /f/a openFile is " << f1 << (f1>0 ? " correct": " fail")<< endl;
  r = c7->myFS->readFile(f1, rbuf4, 1216);
  cout << "rv from readFile is " << r << " Should be 1152" <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf4[i];
  cout << endl;
  r = c7->myFS->closeFile(f1);
  cout << "rv from closeFile /f/a is " << r << (r==0 ? " correct":" fail")<<endl;

  r = c7->myFS->deleteFile(const_cast<char *>("/f/a"), 4);
  cout << "rv from deleteFile /f/a is " << r <<(r==0 ? " correct":" fail")<<endl;

  r = c7->myFS->createFile(const_cast<char *>("/f/a"), 4);
  cout << "rv from createFile /f/a is " << r <<(r==0 ? " correct": " fail") <<endl;
  f1 = c7->myFS->openFile(const_cast<char *>("/f/a"), 4, 'w', -1);
  cout << "rv from /f/a openFile is " << f1 << (f1>0 ? " correct": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf3, 600);
  cout << "rv from writeFile /f/a is " << r << (r==600 ? " correct write 600 Fs": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf4, 550);
  cout << "rv from writeFile /f/a is " << r << (r==550 ? " correct write 550 Is": " fail")<< endl;
  r = c7->myFS->writeFile(f1, buf1, 5);
  cout << "rv from writeFile /f/a is " << r << (r==-3? " correct, but it does write out a couple of Ps.": " fail")<< endl;
  r = c7->myFS->closeFile(f1);
  cout << "rv from closeFile /f/a is " << r << (r==0 ? " correct":" fail")<<endl;

  f1 = c7->myFS->openFile(const_cast<char *>("/f/a"), 4, 'r', -1);
  cout << "rv from /f/a openFile is " << f1 << (f1>0 ? " correct": " fail")<< endl;
  r = c7->myFS->readFile(f1, rbuf4, 1216);
  cout << "rv from readFile is " << r << " Should be 1152" <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf4[i];
  cout << endl;
  r = c7->myFS->closeFile(f1);
  cout << "rv from closeFile /f/a is " << r << (r==0 ? " correct":" fail")<<endl;


  cout <<"\n\nBegin filling fs2\n";
  r = c2->myFS->createFile(const_cast<char *>("/o/o/o/a"), 8);
  cout << "rv from createFile /o/o/o/a is " << r <<(r==0 ? " correct": " fail") <<endl;
  f1 = c2->myFS->openFile(const_cast<char *>("/o/o/o/a"), 8, 'w', -1);
  cout << "rv from openFile /o/o/o/a is " << f1 << (f1>0 ? " correct": " fail")<< endl;
  r = c2->myFS->writeFile(f1, buf3, 600);
  cout << "rv from writeFile /o/o/o/b is " << r << (r==600 ? " correct write 600 Fs": " fail")<< endl;
  r = c2->myFS->writeFile(f1, buf4, 600);
  cout << "rv from writeFile /o/o/o/b is " << r << (r==600 ? " correct write 600 Is": " fail")<< endl;
  r = c2->myFS->closeFile(f1);
  cout << "rv from closeFile /o/o/o/b is " << r << (r==0 ? " correct":" fail")<<endl;


  r = c2->myFS->createFile(const_cast<char *>("/o/o/o/b"), 8);
  cout << "rv from createFile /o/o/o/b is " << r <<(r==0 ? " correct": " fail") <<endl;
  f1 = c2->myFS->openFile(const_cast<char *>("/o/o/o/b"), 8, 'w', -1);
  cout << "rv from openFile /o/o/o/b is " << f1 << (f1>0 ? " correct": " fail")<< endl;
  r = c2->myFS->writeFile(f1, buf4, 600);
  cout << "rv from writeFile is " << r << (r==600 ? " correct write 600 Is": " fail")<< endl;
  r = c2->myFS->appendFile(f1, buf2, 64);
  cout << "rv from appendFile /o/o/o/b is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c2->myFS->appendFile(f1, buf3, 64);
  cout << "rv from appendFile /o/o/o/b is " << r <<(r==64 ? " correct write 64 Fs": " fail")<< endl;
  r = c2->myFS->appendFile(f1, buf4, 64);
  cout << "rv from appendFile /o/o/o/b is " << r <<(r==64 ? " correct write 64 Is": " fail")<< endl;
  r = c2->myFS->closeFile(f1);
  cout << "rv from closeFile /o/o/o/b is " << r << (r==0 ? " correct":" fail")<<endl;

  r = c2->myFS->createFile(const_cast<char *>("/o/o/o/c"), 8);
  cout << "rv from createFile /o/o/o/c is " << r <<(r==0 ? " correct": " fail") <<endl;
  f1 = c2->myFS->openFile(const_cast<char *>("/o/o/o/c"), 8, 'w', -1);
  cout << "rv from /o/o/o/c openFile is " << f1 << (f1>0 ? " correct": " fail")<< endl;
  r = c2->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /o/o/o/c is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c2->myFS->writeFile(f1, buf3, 64);
  cout << "rv from writeFile /o/o/o/c is " << r <<(r==64 ? " correct write 64 Fs": " fail")<< endl;
    r = c2->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /o/o/o/c is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c2->myFS->writeFile(f1, buf3, 64);
  cout << "rv from writeFile /o/o/o/c is " << r <<(r==64 ? " correct write 64 Fs": " fail")<< endl;
  r = c2->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /o/o/o/c is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c2->myFS->writeFile(f1, buf3, 64);
  cout << "rv from writeFile /o/o/o/c is " << r <<(r==64 ? " correct write 64 Fs": " fail")<< endl;
  r = c2->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /o/o/o/c is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c2->myFS->writeFile(f1, buf3, 64);
  cout << "rv from writeFile /o/o/o/c is " << r <<(r==64 ? " correct write 64 Fs": " fail")<< endl;
  r = c2->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /o/o/o/c is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c2->myFS->writeFile(f1, buf3, 64);
  cout << "rv from writeFile /o/o/o/c is " << r <<(r==64 ? " correct write 64 Fs": " fail")<< endl;
  r = c2->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /o/o/o/c is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  
  r = c2->myFS->createFile(const_cast<char *>("/o/o/o/d"), 8);
  cout << "rv from createFile /o/o/o/d is " << r <<(r==0 ? " correct": " fail") <<endl;
  f2 = c2->myFS->openFile(const_cast<char *>("/o/o/o/d"), 8, 'w', -1);
  cout << "rv from /o/o/o/d openFile f2 is " << f2 << (f1>0 ? " correct": " fail")<< endl;
  r = c2->myFS->writeFile(f2, buf2, 64);
  cout << "rv from writeFile /o/o/o/d is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;
  r = c2->myFS->writeFile(f2, buf3, 64);
  cout << "rv from writeFile /o/o/o/d is " << r <<(r==64 ? " correct write 64 Fs": " fail")<< endl;
  r = c2->myFS->writeFile(f2, buf2, 64);
  cout << "rv from writeFile /o/o/o/d is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;


  r = c2->myFS->appendFile(f2, buf4, 64);
  cout << "rv from appendFile /o/o/o/d is " << r <<(r==-3 ? " correct fails, need index and data block": " fail")<< endl;

  r = c2->myFS->writeFile(f1, buf2, 64);
  cout << "rv from writeFile /o/o/o/c is " << r <<(r==64 ? " correct write 64 Ts": " fail")<< endl;

  //full
  r = c2->myFS->writeFile(f1, buf4, 64);
  cout << "rv from writeFile /o/o/o/c is " << r <<(r==-3 ? " correct fs2 is full": " fail")<< endl;
  
  r = c2->myFS->appendFile(f2, buf4, 64);
  cout << "rv from appendFile /o/o/o/d is " << r <<(r==-3 ? " correct fs2 is full": " fail")<< endl;
  r = c2->myFS->closeFile(f1);
  cout << "rv from closeFile /o/o/o/c is " << r << (r==0 ? " correct":" fail")<<endl;
  r = c2->myFS->closeFile(f2);
  cout << "rv from closeFile /o/o/o/d is " << r << (r==0 ? " correct":" fail")<<endl;


/* decided I don't care about reading it all back.
  r = c2->myFS->readFile(f4, rbuf2, 64);
  cout << "rv from readFile is " << r << endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf2[i];
  cout << endl;
*/
 

  cout <<"end driver 7\n";
  return 0;
}
