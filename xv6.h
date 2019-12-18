/***********************************************************************

Filename:             xv6.h 
Team Members:         Aikansh Priyam and Man Parvesh Singh Randhawa
UTD_ID:               2021489135 and 2021468453
NetID:                axp190019 and mxr180061
Class:                OS 5348.001
Project:              Project 3

NOTE: The skeleton code for this project was taken from elearning.utdallas.edu
and it contained initfs and quit commands already implemented. Permission was gained
from the original author to develop our project on their code.

***********************************************************************/

#include "common.h"
#include "littleutils.h"

int preInitialization() {  // Function to check if the filesystem have been created, if not initfs will be called
    char *n1, *n2;
    unsigned int numBlocks = 0, numInodes = 0;
    char* filepath;

    filepath = strtok(NULL, " ");
    n1 = strtok(NULL, " ");  // number of blocks
    n2 = strtok(NULL, " ");  // number of inodes

    if (access(filepath, F_OK) != -1) {
        // if file system already exists at the provided filepath
        if ((fileDescriptor = open(filepath, O_RDWR, 0600)) == -1) {
            printf("\n filesystem already exists but open() failed with error [%s]\n", strerror(errno));
            return 1;
        }
        printf("filesystem already exists and the same will be used.\n");

    } else {
        // if some argument is missing
        if (!n1 || !n2)
            printf(" All arguments(path, number of inodes and total number of blocks) have not been entered\n");
        else {
            // all arguments exist
            numBlocks = atoi(n1);  // converts char to int
            numInodes = atoi(n2);  // converts char to int
                                   // If initialization is success (returns 1) then enter if
            if (initfs(filepath, numBlocks, numInodes)) {
                printf("The file system is initialized\n");
            } else {
                // If initialization  fails (returns 0) then enter else
                printf("Error initializing file system. Exiting... \n");
                return 1;
            }
        }
    }

    return 0;
}

int initfs(char* path, unsigned short blocks, unsigned short inodes) {
    char buffer[BLOCK_SIZE] = {0};
    unsigned short i = 0;
    int bytes_written;

    superBlock.fsize = blocks;                                  // set block size in superblock
    unsigned short inodes_per_block = BLOCK_SIZE / INODE_SIZE;  // number of inodes per block

    int data_blocks = blocks - 2 - superBlock.isize;  // number of available blocks for writing
    int data_blocks_for_free_list = data_blocks - 1;

    // if all inodes can not be perfectly divided to number of inodes per block, isize would be 1 more
    if ((inodes % inodes_per_block) == 0)
        superBlock.isize = inodes / inodes_per_block;
    else
        superBlock.isize = (inodes / inodes_per_block) + 1;

    if ((fileDescriptor = open(path, O_RDWR | O_CREAT, 0700)) == -1) {
        // unable to open file with path=path
        printf("\n open() failed with the following error [%s]\n", strerror(errno));
        return 0;
    }

    // write empty buffer to boot sector and superblock
    for (i = 0; i < 2; i++) {
        writeToBlock(i, buffer, BLOCK_SIZE);
    }

    // write empty buffer to inode blocks
    for (i = 0; i < superBlock.isize; i++) {
        writeToBlock(i + 2, buffer, BLOCK_SIZE);
    }

    // write empty buffer to data blocks
    for (i = 0; i < data_blocks_for_free_list; i++) {
        writeToBlock(i + 2 + superBlock.isize, buffer, BLOCK_SIZE);
    }

    for (i = 0; i < data_blocks_for_free_list; i++) {
        addBlockToFreeList(i + 2 + superBlock.isize);  // Adding Data blocks to free list
    }

    for (i = 0; i < FREE_SIZE; i++)
        superBlock.free[i] = 0;  //initializing free array to 0 to remove junk data. free array will be stored with data block numbers shortly.

    superBlock.nfree = 0;  // Initializing nfree to 0
    superBlock.ninode = I_SIZE;

    for (i = 0; i < I_SIZE; i++)
        superBlock.inode[i] = i + 1;  //Initializing the inode array to inode numbers

    superBlock.flock = 'a';  //flock,ilock and fmode are not used.
    superBlock.ilock = 'b';
    superBlock.fmod = 0;
    superBlock.time[0] = 0;
    superBlock.time[1] = 1970;  // beginning of time

    writeToBlock(SUPERBLOCK_LOCATION, &superBlock, BLOCK_SIZE);

    // Create root directory
    create_root();

    // set current working directory to root
    cwdInodeNumber = 0;
    strcpy(cwdPath, "/");

    return 1;
}

// Create root directory
void create_root() {
    int root_data_block = 2 + superBlock.isize;  // Allocating first data block to root directory
#ifdef DEBUG
    printf("root_data_block = %d\n", root_data_block);
#endif
    int i;

    inode_type inode;

    // Adding 1st file whose name is . (denotes current directory)
    directory_entry_type firstFile;
    firstFile.inode = 0;  // root directory's inode number is 1.
    firstFile.filename[0] = '.';
    firstFile.filename[1] = '\0';

    inode.flags = inode_alloc_flag | dir_flag | dir_access_rights;  // flag for root directory
    inode.nlinks = 0;
    inode.uid = 0;
    inode.gid = 0;
    inode.size = 2 * sizeof(directory_entry_type);
    inode.addr[0] = root_data_block;

    for (i = 1; i < ADDR_SIZE; i++) {
        inode.addr[i] = 0;
    }

    inode.actime[0] = 0;
    inode.modtime[0] = 0;
    inode.modtime[1] = 0;

    writeInodeStructToInodeNumber(0, inode);

    lseek(fileDescriptor, root_data_block * BLOCK_SIZE, 0);  // reset read/write pointer to the next block after inodes block(data block)
    write(fileDescriptor, &firstFile, 16);                   // writing root to the data block

    // Adding 2nd file whose name is .. (denotes parent directory)
    directory_entry_type secondFile;
    secondFile.inode = 0;  // root directory's inode number is 1.
    secondFile.filename[0] = '.';
    secondFile.filename[1] = '.';
    secondFile.filename[2] = '\0';
    write(fileDescriptor, &secondFile, 16);
}

int makeNewDirectory(char* dirName) {    // Function for creating new Directory
    int blockNumber = getFreeBlock();    // block to store directory table
    int inodeNumber = getFreeInode();    // inode number for directory
    directory_entry_type directory[2];   // directory is a variable of structure directory_entry_type
    directory[0].inode = inodeNumber;    // assigning inodenumber
    strcpy(directory[0].filename, ".");  // copying . for root directory

    directory[1].inode = cwdInodeNumber;  // assigning current node number
    strcpy(directory[1].filename, "..");  // copying .. for parent directory

    writeToBlock(blockNumber, directory, 2 * sizeof(directory_entry_type));  // writing to block
    // write directory i node
    inode_type dir;
    dir.flags = inode_alloc_flag | dir_flag | dir_access_rights;  // setting 14th and 15th bit to 1, 15: allocated and 14: directory
    dir.nlinks = 1;                                               // Initializing all the variables
    dir.uid = 0;
    dir.gid = 0;
    dir.size = 2 * sizeof(directory_entry_type);
    dir.addr[0] = blockNumber;
    dir.actime[0] = 0;
    dir.modtime[0] = 0;
    dir.modtime[1] = 0;

    writeInodeStructToInodeNumber(inodeNumber, dir);  // writing Inodestruct using inode number

    // update pRENT DIR I NODE
    inode_type curINode = readInodeFromInodeNumber(cwdInodeNumber);  // reading inode from current indoe number
    blockNumber = curINode.addr[0];                                  // fetching the current blocknumber
    directory_entry_type newDir;
    newDir.inode = inodeNumber;
    strcpy(newDir.filename, dirName);                                                           // copying dirName to newDir
    writeToBlockWithOffset(blockNumber, &newDir, sizeof(directory_entry_type), curINode.size);  // Function for writing to data block with the offset
    curINode.size += sizeof(directory_entry_type);                                              // incrementing the current inode_size
    writeInodeStructToInodeNumber(cwdInodeNumber, curINode);                                    // writing Inodestruct using the current inode number
}

// Function to create a new file called v6-file and fill the contents with the contents of the externalfile.
int cpin(char* fromFile, char* toFile) {
    int source, blockNumber;
    if ((source = open(fromFile, O_RDWR | O_CREAT, 0600)) == -1) {  // checking if the file from which contents need to be copied exist or not
        printf("\n Unable to open source file [%s]\n", strerror(errno));
        return -1;
    }

    long int fileSize = lseek(source, 0, SEEK_END);
#ifdef DEBUG
    printf("Input file size = %ld, limit = %ld\n", fileSize, (1L << 32));
#endif
    if (fileSize > (1L << 32)) {
        printf("Source file too large. (size = %ld bytes)\n", fileSize);
        return -1;
    }

    if (getInodeNumberFromFileName(toFile) != -1) {  // Checking if the file to be created already exists or not
        printf("Source file already exists.\n");
        return -1;
    }

    // for now, ensure file is small enough to fit in addresses from the addr array
    // todo remove this after implementing large files
    if (fileSize > (11 * 1024)) {
        printf("Sorry, files larger than %d bytes are not supported. (size = %ld bytes)\n", (11 * 1024), fileSize);
        return -1;
    }

    int inodeNumber = getFreeInode();  // get a free inode
    inode_type file;
    file.flags = 1 << 15;  // setting 15th bit to 1, 15: allocated
    file.nlinks = 1;       // Initializing all other variables
    file.uid = 0;
    file.gid = 0;
    file.size = 0;
    file.actime[0] = 0;
    file.modtime[0] = 0;
    file.modtime[1] = 0;

    // read source and copy to destination, block by block
    int bytesRead = BLOCK_SIZE;
    char buffer[BLOCK_SIZE] = {0};
    int i = 0;
    while (bytesRead == BLOCK_SIZE) {  // loop for copying contents
        bytesRead = read(source, buffer, BLOCK_SIZE);
        file.size += bytesRead;
        blockNumber = getFreeBlock();
        file.addr[i] = blockNumber;
        writeToBlock(blockNumber, buffer, bytesRead);
        i++;
    }
    writeInodeStructToInodeNumber(inodeNumber, file);  // writing Inodestruct using inode number

    inode_type curINode = readInodeFromInodeNumber(cwdInodeNumber);  // reading inode from current inode number
    blockNumber = curINode.addr[0];                                  // fetching the block number
    directory_entry_type newFile;
    newFile.inode = inodeNumber;
    strcpy(newFile.filename, toFile);  // copy toFile filename to newFile
    writeToBlockWithOffset(blockNumber, &newFile, sizeof(directory_entry_type), curINode.size);
    curINode.size += sizeof(directory_entry_type);  // incrementing the inode size
    writeInodeStructToInodeNumber(cwdInodeNumber, curINode);

    return 1;  // return 1 means success!
}

// list all files and folders inside the provided path
void ls() {
    inode_type cwdInode = readInodeFromInodeNumber(cwdInodeNumber);  // reading inode from current inode number
    int filesBeginningAddr = cwdInode.addr[0];                       // fetching begining address of current inode
    directory_entry_type files[I_SIZE];
    readAtLocationWithOffset(filesBeginningAddr, files, cwdInode.size, 0);  // read at loaction with given inode

#ifdef DEBUG
    printf("cwdInode.size = %d, sizeof(directory_entry_type) = %d\n", cwdInode.size, (int)(sizeof(directory_entry_type)));
#endif

    for (int i = 0; i < (cwdInode.size / sizeof(directory_entry_type)); i++) {
        printf("%s\t", files[i].filename);  // printing the contents of the directory
    }

    printf("\n");
}

void cd(char* dirName) {                                             // Function to change current directory to the given directory
    inode_type curINode = readInodeFromInodeNumber(cwdInodeNumber);  // reading inode from current inode number
    int blockNumber = curINode.addr[0];                              // fetching block number of current inode
    directory_entry_type directory[100];
    readAtLocation(blockNumber, directory, curINode.size);                    // reading at location at the given inode
    for (int i = 0; i < curINode.size / sizeof(directory_entry_type); i++) {  // loop to go through the size of the inode
        if (strcmp(dirName, directory[i].filename) == 0) {
            inode_type dir = readInodeFromInodeNumber(directory[i].inode);
            if (dir.flags == (inode_alloc_flag | dir_flag | dir_access_rights)) {
                if (strcmp(dirName, ".") == 0) {  // If parent directory return
                    return;
                } else if (strcmp(dirName, "..") == 0) {  //
                    if (cwdInodeNumber == directory[i].inode) {
                        // reached root, don't do anything
                        continue;
                    }
                    if (directory[i].inode == 0) {
                        cwdInodeNumber = directory[i].inode;
                        strcpy(cwdPath, "/");
                        continue;
                    }
                    cwdInodeNumber = directory[i].inode;
                    int lastSlashPosition = strrchr(cwdPath, '/') - cwdPath;
                    char temp[INPUT_SIZE];
                    strncpy(temp, cwdPath, lastSlashPosition + 1);  // copying the different directory path to the the current directory path
                    strcpy(cwdPath, temp);
                } else {
                    if (cwdInodeNumber != 0) {
                        strcat(cwdPath, "/");
                    }
                    cwdInodeNumber = directory[i].inode;
                    strcat(cwdPath, dirName);
                }
            } else {
                printf("Entered path is not a directory\n");
            }
            return;
        }
    }
}

void cwd() {  // prints current working directory
    printf("%s\n", cwdPath);
}

// open existing xv6 filesystem file from external
void openFileSystem(char* path) {
    if ((fileDescriptor = open(path, O_RDWR | O_CREAT, 0700)) == -1) {
        // unable to open file with path=path
        printf("\n open() failed with the following error [%s]\n", strerror(errno));
        return;
    }

    // read super block and set current working directory to root
    readAtLocation(1, &superBlock, BLOCK_SIZE);
    cwdInodeNumber = 0;
    strcpy(cwdPath, "/");

    printf("Opened existing file system at location: %s\n", path);
}

void cpout(char* fromFileName, char* toFileName) {  // Function to copy the contents of a file in filesystem to an external file
    int toFileFD = open(toFileName, O_RDWR | O_CREAT, 0600);
    if (toFileFD == -1) {  // Checking if toFilename exists or not
        printf("Unable to create file %s [%s]\n", toFileName, strerror(errno));
        return;
    }

    int fromFileInodeNumber = getInodeNumberFromFileName(fromFileName);  // checking if the file from where contents need to be copied exists or not
    if (fromFileInodeNumber == -1) {
        printf("Source file %s does not exist in our xv6 filesystem\n", fromFileName);
        return;
    }

    inode_type fromFileInode = readInodeFromInodeNumber(fromFileInodeNumber);  // read from Inode number

    if (fromFileInode.flags != (1 << 15)) {  //  Chekcing if the given file is a file or not
        printf("Source file %s is not a file\n", fromFileName);
        return;
    }

    //// read from source and write to target file now
    unsigned char buffer[BLOCK_SIZE];
    int numberOfBlocks = fromFileInode.size / BLOCK_SIZE;
    for (int i = 0; i < numberOfBlocks; i++) {
        // read block at addr[i] and save in buffer
        readAtLocation(fromFileInode.addr[i], buffer, BLOCK_SIZE);
        // write contents of buffer to output file
        write(toFileFD, buffer, BLOCK_SIZE);
    }

    // read and write remaining content too
    readAtLocation(fromFileInode.addr[numberOfBlocks], buffer, fromFileInode.size % BLOCK_SIZE);
    write(toFileFD, buffer, fromFileInode.size % BLOCK_SIZE);

    close(toFileFD);
    return;
}

void rm(char* fileName) {                                        // Function to remove a file from directory
    int fileInodeNumber = getInodeNumberFromFileName(fileName);  // Function to get inode number from the given file name
#ifdef DEBUG
    printf("fileInodeNumber = %d\n", fileInodeNumber);
#endif
    if (fileInodeNumber == -1) {  // checking if file exists or not
        printf("File %s does not exist in our xv6 filesystem\n", fileName);
        return;
    }

    inode_type fileInode = readInodeFromInodeNumber(fileInodeNumber);  // reading inode from given inode number
    if (fileInode.flags == (inode_alloc_flag | dir_flag | dir_access_rights)) {
        printf("%s is a directory, not a file\n", fileName);
        return;
    }
    cleanUpInodeContents(fileInode);
    addFreeInode(fileInodeNumber);

    removeFileFromCwd(fileName, fileInodeNumber);  // remove file from the directory
}

void rm_dir(char* dirName) {                                   /// Function for removing a directoy
    int dirInodeNumber = getInodeNumberFromFileName(dirName);  // Function to get inode number from the given file name
#ifdef DEBUG
    printf("dirInodeNumber = %d\n", dirInodeNumber);
#endif
    if (dirInodeNumber == -1) {  // checking if file exists or not
        printf("Directory %s does not exist in our xv6 filesystem\n", dirName);
        return;
    }

    inode_type dirInode = readInodeFromInodeNumber(dirInodeNumber);  // reading inode from given inode number

    if (dirInode.flags != (inode_alloc_flag | dir_flag | dir_access_rights)) {
        printf("%s is not a directory\n", dirName);
        return;
    }

    if (dirInode.size > 32) {
        printf("%s - directory is not empty so cannot delete\n", dirName);
        return;
    }

    cleanUpInodeContents(dirInode);
    addFreeInode(dirInodeNumber);

    removeFileFromCwd(dirName, dirInodeNumber);  // removing the given directory
}
