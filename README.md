# Final Project

Group name: Group_8

group members : Nolan,Reis, Kamal

What is working and not working?
Driver1- Driver4 are all correct.
Driver6,7,8 has few errors.



What are you attributes?

---

* base grade: /100
  * group eval:  individual grades will be emailed.
* create file (imples r/w in dm, pm r/w, getfree, and returnblock) (10 points):  
* openfile (and close 3points of) opentable, etc ignoring unlock/lock (10 points):  
* readfile (10 points):  
* writefile (10 points): 
* truncfile (10 points): 
* seekfile (5 points):  
* appendfile ( 5 points, since just call see to end and write):  
* createdir (10 points) => implies create,open work as well: 
* lock and unlock (5 points each, 10 points)  looped into open/close as well: 
* rename (5 points): 
* deletefile (5 points), remember lock again: 
* deletedir (5 points), remember empty: 
* attributes read/set (5 points): 

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


