/***********************************************************************

Filename:             fsaccess.c 
Team Members:         Aikansh Priyam and Man Parvesh Singh Randhawa
UTD_ID:               2021489135 and 2021468453
NetID:                axp190019 and mxr180061
Class:                OS 5348.001
Project:              Project 3

NOTE: The skeleton code for this project was taken from elearning.utdallas.edu
and it contained initfs and quit commands already implemented. Permission was gained
from the original author to develop our project on their code.

***********************************************************************/

/***********************************************************************
 This program allows user to use 10 commands: 
   1. initfs: Initilizes the file system and redesigning the Unix file system to accept large 
      files of up tp 4GB, expands the free array to 152 elements, expands the i-node array to 
      200 elemnts, doubles the i-node size to 64 bytes and other new features as well.
   
 User Input:
     - initfs (file path) (# of total system blocks) (# of System i-nodes)
     - cpin external_file v6-file: Creat a new file called v6-file fill the contents with the contents of the externalfile. 
     - cpout v6-file external_file: If the v6-file exists, create externalfile and make the externalfile's contents equal to v6-file.
     - mkdir v6-dir: If v6-dir does not exist, create the directory and set its first two entries . and ..
     - rm v6-file: If v6-file exists, delete the file, free the i-node, remove the file name from the (parent) directory
     - ls : List the contents of the current directory
     - pwd : List the fill pathname of the current directory
     - cd dirname : change current (working) directory to the dirname
     - rmdir dir: Remove the directory specified
     - open filename : Open the external file filename, which has a v6 filesystem installed
     - q
     
 File name is limited to 14 characters.
***********************************************************************/

/***********************************************************************

Bugs in previous code:

1. Superblock size is 1028 instead of 1024

Reason: If the the short int element is immediately allocated after the char element, it will start at an odd address boundary. 
The compiler will insert a padding byte after the char to ensure short int will have an address multiple of 2

2. Superblock was not being written to the file after free blocks were initialized

3. In create_root(), lseek(fileDescriptor, root_data_block, 0); shoule be lseek(fileDescriptor, root_data_block * BLOCK_SIZE, 0);

Bugs from Warnings in the previous code:

1. %d in printf:

unsigned long was being printed with %d instead of %lu which was giving warning

2. preInitialization():

function prototype was not defined for the function because of which warning was coming

3. if((fileDescriptor = open(filepath, O_RDWR, 0600)) == -1)

This statement was giving warning because parenthesis was not added correctly

***********************************************************************/

#include "xv6.h"  // Contains the implementations of all the functions implemented

int main() {
    char input[INPUT_SIZE];                     // character array for user input of commands
    char *splitter;                             // character pointer for splitting the user input
    unsigned int numBlocks = 0, numInodes = 0;  // variable for storing number of blocks and inodes
    char *filepath;
    printf("Size of super block = %lu , size of i-node = %lu\n", sizeof(superBlock), sizeof(inode_type));

    printf("Enter command:\n");

    while (1) {
        if (fileDescriptor != -2) {
            printf("\033[32;1m%s \033[0m", cwdPath);
        }
        printf("\033[32;1m$ \033[0m");
        scanf(" %[^\n]s", input);  // read whole line from the user input

#ifdef DEBUG
        printf("input = %s\n", input);
#endif
        splitter = strtok(input, " ");  // split line by spaces and save as an array into splitter

        if (strcmp(splitter, "initfs") == 0) {  // If filesystem is not initialized initialize the filesystem
                                                // if the command called is initfs, we initialize the file system
            preInitialization();
            splitter = NULL;

        } else if (strcmp(splitter, "q") == 0) {  //q-> means exit
            // when q is entered, stop the loop
            lseek(fileDescriptor, BLOCK_SIZE, 0);            // reset read/write pointer and move to beginning of the file
            write(fileDescriptor, &superBlock, BLOCK_SIZE);  // writing superblock to file system
            return 0;
        } else if (strcmp(splitter, "open") == 0) {  //if command is open then an external filename is opened which has a v6 filesystem installed
            char *existingFileSystemPath;
            existingFileSystemPath = strtok(NULL, " ");
            openFileSystem(existingFileSystemPath);
        } else if (strcmp(splitter, "mkdir") == 0) {  // create a new directory
            if (fileDescriptor == -2) {
                printf("Please initialize the file system before utilizing this command.\n");
                continue;
            }
            char *newDirectoryPath;
            newDirectoryPath = strtok(NULL, " ");
            makeNewDirectory(newDirectoryPath);
        } else if (strcmp(splitter, "cpin") == 0) {  // To Creat a new file called v6-file and fill the contents with the contents of the externalfile.
            if (fileDescriptor == -2) {
                printf("Please initialize the file system before utilizing this command.\n");
                continue;
            }
            char *fromFilePathExternal, *toFilePathInternal;  // fromFilePathExternal-> External file, toFilePathInternal-> Internal file
            fromFilePathExternal = strtok(NULL, " ");
            toFilePathInternal = strtok(NULL, " ");
            cpin(fromFilePathExternal, toFilePathInternal);
        } else if (strcmp(splitter, "ls") == 0) {
            if (fileDescriptor == -2) {
                printf("Please initialize the file system before utilizing this command.\n");
                continue;
            }
            ls();
        } else if (strcmp(splitter, "pwd") == 0) {  // to print the path of the current working directory
            if (fileDescriptor == -2) {
                printf("Please initialize the file system before utilizing this command.\n");
                continue;
            }
            cwd();
        } else if (strcmp(splitter, "cd") == 0) {  // to change the current directory
            if (fileDescriptor == -2) {
                printf("Please initialize the file system before utilizing this command.\n");
                continue;
            }
            char *targetDir;  // variable to store target directory
            targetDir = strtok(NULL, " ");
            cd(targetDir);
        } else if (strcmp(splitter, "cpout") == 0) {  // to copy the contents of a file in filesystem to an external file
            if (fileDescriptor == -2) {
                printf("Please initialize the file system before utilizing this command.\n");
                continue;
            }
            char *from = strtok(NULL, " ");
            char *to = strtok(NULL, " ");
            cpout(from, to);
        } else if (strcmp(splitter, "rm") == 0) {  // rm-> remove a file from the directory
            if (fileDescriptor == -2) {
                printf("Please initialize the file system before utilizing this command.\n");
                continue;
            }
            char *fileName = strtok(NULL, " ");
            rm(fileName);
        } else if (strcmp(splitter, "rmdir") == 0) {  // rmdir-> remove directory
            if (fileDescriptor == -2) {
                printf("Please initialize the file system before utilizing this command.\n");
                continue;
            }
            char *directoryPath = strtok(NULL, " ");
            rm_dir(directoryPath);
        } else {
            printf("Invalid input command. Supported commands include: initfs, cpin, cpout, mkdir, rm, ls, pwd, cd, rmdir, open, q\n");
        }
    }
}
