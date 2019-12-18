# Modified xv6 file system implementation

## Details

|               |                                                    |
| ------------- | -------------------------------------------------- |
| Filenames:    | fsaccess.c, common.h, littleutils.h, xv6.h         |
| Team Members: | Man Parvesh Singh Randhawa and Aikansh Priyam      |
| UTD_ID:       | 2021468453 and 2021489135                          |
| NetID:        | mxr180061 and axp190019                            |
| Class:        | CS 5348.001 (Operating System Concepts)            |
| Project:      | Project 3: Modified xv6 file system implementation |

## How to run and use

- On a UNIX machine, run `make` to build the executable file.
- Run `./fsaccess` to start the program
- Initialize a file system using this command: `initfs "Filepath" "Block_number" "Inode_numbers"`
- Or use an existing file system using the command: `open 'existing-filesystem'`

## Files and their function

- `fsaccess.c` is the main C program file which conatins the main method and the read-eval-print loop for our program
- `xv6.h` conatins the implementations of all the functions
- `littleutils.h` contains some utility functions that are used in the function implementations in `xv6.h`
- `common.h` contains all the structures defined and function prototypes
