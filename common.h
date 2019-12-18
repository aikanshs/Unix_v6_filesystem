/***********************************************************************

Filename:             common.h 
Team Members:         Aikansh Priyam and Man Parvesh Singh Randhawa
UTD_ID:               2021489135 and 2021468453
NetID:                axp190019 and mxr180061
Class:                OS 5348.001
Project:              Project 3

NOTE: The skeleton code for this project was taken from elearning.utdallas.edu
and it contained initfs and quit commands already implemented. Permission was gained
from the original author to develop our project on their code.

***********************************************************************/

#ifndef COMMON_H
#define COMMON_H

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FREE_SIZE 152          // Maximum Size of unsigned int free
#define I_SIZE 200             // Maximum number of inodes that can be saved
#define BLOCK_SIZE 1024        // A block can have a maximum size of 1024 bytes
#define ADDR_SIZE 11           // Number of data blocks that inodes can store
#define INPUT_SIZE 256         // Maximum user input size
#define PATH_MAX_LENGTH 64     // Max length of file path
#define SUPERBLOCK_LOCATION 1  // superblock is always in location 1

//Used for bitwise operations to split file size before storing in inode
#define LAST(k, n) ((k) & ((1 << (n)) - 1))
#define MID(k, m, n) LAST((k) >> (m), ((n) - (m)))

// #define DEBUG 1

// Superblock Structure
typedef struct {
    char flock;                    // flag maintained in the core copy of the file system when it's mounted. value immaterial
    char ilock;                    // flag maintained in the core copy of the file system when it's mounted. value immaterial
    unsigned short isize;          // number of blocks devoted to i-list
    unsigned short fsize;          // first block not potentially available for allocation to a file
    unsigned short nfree;          // number of free blocks
    unsigned short ninode;         // number of free i-numbers in inode array
    unsigned short inode[I_SIZE];  // addresses of inodes allocated
    unsigned short fmod;           // flag to indicate that the superblock block. value immaterial
    unsigned short time[2];        // array to store timestamp of the latest update of the superblock
    unsigned int free[FREE_SIZE];  // addresses of free blocks
} superblock_type;

superblock_type superBlock;

// I-Node Structure
typedef struct {
    unsigned short flags;          // used to store information related to file properties and execution
    unsigned short nlinks;         // number of links to the file
    unsigned short uid;            // user id of the creator of this file
    unsigned short gid;            // group id of the owner of this file
    unsigned int size;             // 32 bit size value
    unsigned int addr[ADDR_SIZE];  // block numbers where we can store data
    unsigned short actime[2];      // time of last access
    unsigned short modtime[2];     // time of last modification
} inode_type;

typedef struct {
    unsigned short inode;        // inode number
    unsigned char filename[14];  // file name
} directory_entry_type;

int cwdInodeNumber;        // we store current working directory's inode number in this variable
char cwdPath[INPUT_SIZE];  // to store current working directory's complete path

int fileDescriptor = -2;                          //file descriptor
const unsigned short inode_alloc_flag = 0100000;  // flag to denote that inode is allocated
const unsigned short dir_flag = 040000;           // flag to denote that this is a directory
const unsigned short dir_large_file = 010000;     // flag to denote that this is a large file
const unsigned short dir_access_rights = 000777;  // User, Group, & World have all access privileges
const unsigned short INODE_SIZE = 64;             // inode has been doubled

int initfs(char *path, unsigned short total_blcks, unsigned short total_inodes);  // initialize file system
void add_block_to_free_list(int blocknumber, unsigned int *empty_buffer);         // add given block to the free list
void create_root();                                                               // create root directory
int preInitialization();                                                          // steps to be taken before initializing fs
void ls();                                                                        // list all files in current directory
int cpin(char *fromFile, char *toFile);                                           // copy fromFile from external system to xv6
int makeNewDirectory(char *dirname);                                              // create new directory
void openFileSystem(char *path);                                                  // open existing file system
void checkInitialization();                                                       // checks if file system is initialized before running a command

#endif
