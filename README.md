# Final Project

Group name: Group_8

group members : Nolan,Reis, Kamal

What is working and not working?
Driver1- Driver4 are all correct.
Driver6,7,8 has few errors.



What are you attributes?

---

* base grade: 80/100
  * group eval:  individual grades will be emailed.
* create file (imples r/w in dm, pm r/w, getfree, and returnblock) (10 points):
  * DM that memcopy superblock is not enough -1
  * PM what don't you write the default bv?  so 0 and 1 are not set to in use.  that's not good.
  * fs why is mutex and atomic included?
    * why is the lock in fileinode?  that should be either in open table or a file lock table.
    * the casting structs don't work correctly, so the directory format is wrong -1
    *   why do you have single line function s freeblock and allocate block, instead just call myPM.  waste.
* openfile (and close 3points of) opentable, etc ignoring unlock/lock (10 points):  yes
 * other then file lock issues, yes it fine. 
* readfile (10 points):  yes
  * code looks good and works. 
* writefile (10 points): mostly -2
  * fails on edge cases and but mostly works.  
* truncfile (10 points): not really
  * disk shows it really doesn't work. there is code that could work (even says yes in drivers), but doesn't work.  not time to debug.  -6 
* seekfile (5 points):   yes
* appendfile ( 5 points, since just call see to end and write):  no.... there is code
  * this call seek (size of file) and write.  isntead there is code here.  bad style and code -1 
* createdir (10 points) => implies create,open work as well: 
* lock and unlock (5 points each, 10 points)  looped into open/close as well:
  * locks are not PART of the inode, not surive reboot, and have their own table or part of the file table. -2 
* rename (5 points): mostly
  * it seems to mostly work, but fs corrupted, it's hard to tell.  code looks like it shold mostly work, but disk says it doesn't always. -1 
* deletefile (5 points), remember lock again:  yes sort of
  * work for root, but not directoreis. help funciton fail or just bad code is hard to tell. -2
* deletedir (5 points), remember empty: same sort of work, but not really
  * code is there, but does seem to work or work well.  -2 
* attributes read/set (5 points): no
  * open count is not a file attribute.  also doesn't seem to work. -2 

other notes:
  * disk is 
    * driver 1: runs, says correct
      * there are no human readable numbers in the DISK.
      * superblock just has size, should have start position.
      * directory format was not followed.  block number first, then name and f/d I think.
      * file inodes a look correct.  data is there.
      * I guess there are attributes?  there is some stuff at the end of the file inode.  no idea what it is though.
    * driver 2:  all say pass.
      * the files all look to be there. 
    * driver 3: passes.
      * files are renamed, truncfile, and delete all seem to work.
      * no directory compression. 
    * driver 4: driver say everything passed.
      * no true. fs2 b  file k and l are still inthe directory, but the finodes have been deleted.
      * file /z which show truncfile correctly in dirver output, is still there complete.  the blocks not returned or data deleted.  size does say 192.  indirect block is still listed.  so the file is still there.
      * everything else seem okay.
    * driver 5: fails in rename
      * delets are not working on files and directories.  renames failing.  directories only sort of work.
      * reads and write/append do seem to work.
    * driver 6: lots say correct that have failed.
      * deletes of directoires fail completely.   also in fs2 truncfile didn't work, the file is still there, so didn't return the blocks.
      * file writes/read do appear to work, but not much else.
      * fs2 is corrupted.  fs3 is not much better.  fs4 is okay, so maybe driver 7 will be useful there. 
    * driver 7: fails almost complete
      * fs2 almost everyline is a fail.
      * fs4 all says correct.  and the disk shows
        * it's close.  the file is all there, but the filsize is 1150, not 1152  which is what the driver says.  the last write actually succeeded I can see it in the disk, even though it says fail. but didn't update size.  that could part of the problems elsewhere.  if the block didn't change it did update and fail??? who knows.  
    * driver 8: most of it says fail.  I still have no clue what the attributes are.  /e/f say creation time? what is open count mean? in the output.  everything say fail except set/get for /e/f
      * disk is useless here.  it just anumber maybe?  no idea. 
  * other
    * says driver 8 works, but what are the attributes then????
    * 


