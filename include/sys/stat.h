/*
 * stat.h
 *
 *  Created on: Dec 15, 2015
 *      Author: enerccio
 */

#ifndef SYS_STAT_H_
#define SYS_STAT_H_

#ifdef __CDT_INDEXER__
#include <gcc_defs.h>
#endif

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct stat {
	dev_t     st_dev;     //ID of device containing file
	ino_t     st_ino;     //file serial number
	mode_t    st_mode;    //mode of file (see below)
	nlink_t   st_nlink;   //number of links to the file
	uid_t     st_uid;     //user ID of file
	gid_t     st_gid;     //group ID of file
	dev_t     st_rdev;    //device ID (if file is character or block special)
	off_t     st_size;    //file size in bytes (if file is a regular file)
	time_t    st_atime;   //time of last access
	time_t    st_mtime;   //time of last data modification
	time_t    st_ctime;   //time of last status change
	blksize_t st_blksize; //a filesystem-specific preferred I/O block size for
	                     //this object.  In some filesystem types, this may
	                     //vary from file to file
	blkcnt_t  st_blocks;  //number of blocks allocated for this object
} stat_t;


#define S_IFMT  (1<<1) // type of file
#define S_IFBLK (1<<2) // block special
#define S_IFCHR (1<<3) // character special
#define S_IFIFO (1<<4) // FIFO special
#define S_IFREG (1<<5) // regular
#define S_IFDIR (1<<6) // directory
#define S_IFLNK (1<<7) // symbolic link

// File mode bits:

#define S_IRWXU (S_IRUSR | S_IWUSR | S_IXUSR) // read, write, execute/search by owner
#define S_IRUSR (1<<1) // read permission, owner
#define S_IWUSR (1<<2) // write permission, owner
#define S_IXUSR (1<<3) // execute/search permission, owner
#define S_IRWXG (S_IRGRP | S_IWGRP | S_IXGRP)// read, write, execute/search by group
#define S_IRGRP (1<<4) // read permission, group
#define S_IWGRP (1<<5) // write permission, group
#define S_IXGRP (1<<6) // execute/search permission, group
#define S_IRWXO (S_IROTH | S_IWOTH | S_IXOTH) // read, write, execute/search by others
#define S_IROTH (1<<7) // read permission, others
#define S_IWOTH (1<<8) // write permission, others
#define S_IXOTH (1<<9) // execute/search permission, others
#define S_ISUID (1<<10) // set-user-ID on execution
#define S_ISGID (1<<11) // set-group-ID on execution
#define S_ISVTX (1<<12) // on directories, restricted deletion flag


#define S_ISBLK(m) 		//    Test for a block special file.
#define S_ISCHR(m) 		//    Test for a character special file.
#define S_ISDIR(m) 		//    Test for a directory.
#define S_ISFIFO(m) 	//    Test for a pipe or FIFO special file.
#define S_ISREG(m) 		//    Test for a regular file.
#define IS_ISLNK(m) 	//    Test for a symbolic link.

#define S_TYPEISMQ(buf) 		// Test for a message queue
#define S_TYPEISSEM(buf)		// Test for a semaphore
#define S_TYPEISSHM(buf)		// Test for a shared memory object

int chmod(const char*, mode_t);
int fchmod(int, mode_t);
int fstat(int, struct stat*);
int lstat(const char*, struct stat*);
int mkdir(const char*, mode_t);
int mkfifo(const char*, mode_t);
int mknod(const char*, mode_t, dev_t);
int stat(const char*, struct stat*);
mode_t umask(mode_t);


#ifdef __cplusplus
}
#endif

#endif /* SYS_STAT_H_ */
