/***********************************************************************

Filename:             littleutils.h 
Team Members:         Aikansh Priyam and Man Parvesh Singh Randhawa
UTD_ID:               2021489135 and 2021468453
NetID:                axp190019 and mxr180061
Class:                OS 5348.001
Project:              Project 3

NOTE: The skeleton code for this project was taken from elearning.utdallas.edu
and it contained initfs and quit commands already implemented. Permission was gained
from the original author to develop our project on their code.

***********************************************************************/

#ifndef LITTLEUTILS_H
#define LITTLEUTILS_H

#include "common.h"

/////////////////////////////////////////////////////////////////
// location change inside the file that describes the file system
/////////////////////////////////////////////////////////////////
void changeLocationWithOffset(int isInode, int index, int offset) {
    int jump = isInode == 1 ? INODE_SIZE : BLOCK_SIZE;
    int beginningIndex = isInode == 1 ? (1 + 1) : 0;
    lseek(fileDescriptor, beginningIndex * BLOCK_SIZE + (jump * index) + offset, SEEK_SET);
}

void changeLocation(int isInode, int index) {
    changeLocationWithOffset(isInode, index, 0);
}

void changeLocationInode(int inodeNumber) {
    changeLocation(1, inodeNumber);
}

void changeLocationDataBlockWithOffset(int index, int offset) {
    // boot sector + superblock + data blocks for inode list
    int dataBlockStartIndex = 1 + 1 + superBlock.isize;
    changeLocationWithOffset(0, dataBlockStartIndex + index, offset);
}

void changeLocationDataBlock(int index) {
    changeLocationDataBlockWithOffset(index, 0);
}
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// Write at a specified location
/////////////////////////////////////////////////////////////////
void writeToBlockWithOffset(int blockNumber, void* buffer, int size, int offset) {
    changeLocationWithOffset(0, blockNumber, offset);
    write(fileDescriptor, buffer, size);
}

void writeToBlock(int blockNumber, void* buffer, int size) {
    writeToBlockWithOffset(blockNumber, buffer, size, 0);
}

void writeInodeStructToInodeNumber(int inodeNumber, inode_type inode) {
    changeLocationInode(inodeNumber);
    write(fileDescriptor, &inode, sizeof(inode_type));
}
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// Read from a specified location
/////////////////////////////////////////////////////////////////
void readAtLocationWithOffset(int blockNumber, void* buffer, int size, int offset) {
    changeLocationWithOffset(0, blockNumber, offset);
    read(fileDescriptor, buffer, size);
}

void readAtLocation(int blockNumber, void* buffer, int size) {
    readAtLocationWithOffset(blockNumber, buffer, size, 0);
}

void readDataBlock(int blockNumber, void* buffer, int size) {
    readAtLocation(1 + 1 + superBlock.isize + blockNumber, buffer, size);
}

inode_type readInodeFromInodeNumber(int inodeNumber) {
    changeLocationInode(inodeNumber);
    inode_type inode;
    read(fileDescriptor, &inode, sizeof(inode_type));
    return inode;
}
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// Dealing with free data blocks and inodes
/////////////////////////////////////////////////////////////////
void addBlockToFreeList(int blockNumber) {
    if (superBlock.nfree == FREE_SIZE) {
        writeToBlock(blockNumber, superBlock.free, FREE_SIZE * 4);
        superBlock.nfree = 0;
    }
    superBlock.free[superBlock.nfree] = blockNumber;
    superBlock.nfree++;
}

int getFreeBlock() {
    if (superBlock.nfree == 0) {
        int blockNumber = superBlock.free[0];
        readDataBlock(blockNumber, superBlock.free, FREE_SIZE * 4);
        superBlock.nfree = 100;
        return blockNumber;
    }
    superBlock.nfree--;
    return superBlock.free[superBlock.nfree];
}

void addFreeInode(int inodeNumber) {
    if (superBlock.ninode == I_SIZE)
        return;
    superBlock.inode[superBlock.ninode] = inodeNumber;
    superBlock.ninode++;
}

int getFreeInode() {
    superBlock.ninode--;
    return superBlock.inode[superBlock.ninode];
}

int getInodeNumberFromFileName(char* fileName) {
    // start searching in current directory
    int inodeNumber = -1;
    inode_type directory_inode = readInodeFromInodeNumber(cwdInodeNumber);

    directory_entry_type files[FREE_SIZE];
    readAtLocation(directory_inode.addr[0], files, directory_inode.size);
    int numberOfFiles = directory_inode.size / sizeof(directory_entry_type);
    for (int i = 0; i < numberOfFiles; i++) {
        if (strcmp(fileName, files[i].filename) == 0) {
            inodeNumber = files[i].inode;
            break;
        }
    }

    return inodeNumber;
}

void cleanUpInodeContents(inode_type inode) {
    // add data blocks to free list
    // and inode number to the i-list
    int x;
    for (x = 0; x < inode.size / BLOCK_SIZE; x++) {
        addBlockToFreeList(inode.addr[x]);
    }
    if (0 < inode.size % BLOCK_SIZE) {
        addBlockToFreeList(inode.addr[x]);
    }
}

void removeFileFromCwd(char* fileName, int inodeNumber) {
    // start searching in current directory
    inode_type directory_inode = readInodeFromInodeNumber(cwdInodeNumber);

    directory_entry_type files[FREE_SIZE];
    readAtLocation(directory_inode.addr[0], files, directory_inode.size);
    int numberOfFiles = directory_inode.size / sizeof(directory_entry_type);
    for (int i = 0; i < numberOfFiles; i++) {
        if (strcmp(fileName, files[i].filename) == 0) {
            files[i] = files[numberOfFiles - 1];
            break;
        }
    }

    directory_inode.size -= sizeof(directory_entry_type);
    writeToBlock(directory_inode.addr[0], files, directory_inode.size);
    writeInodeStructToInodeNumber(cwdInodeNumber, directory_inode);
}
/////////////////////////////////////////////////////////////////
#endif  // LITTLEUTILS_H
