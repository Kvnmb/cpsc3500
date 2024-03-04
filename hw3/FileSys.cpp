// CPSC 3500: File System
// Implements the file system commands that are available to the shell.

#include <cstring>
#include <iostream>
#include <cmath>
using namespace std;

#include "FileSys.h"
#include "BasicFileSys.h"
#include "Blocks.h"

// mounts the file system
void FileSys::mount() {
    bfs.mount();
    curr_dir = 1;
}

// unmounts the file system
void FileSys::unmount() {
    bfs.unmount();
}

// make a directory
void FileSys::mkdir(const char *name)
{
    // Check for file name length
    if(strlen(name) > MAX_FNAME_SIZE){
        cout << "File name is too long.\n";
        return;
    }

    // initialize directory block to read values
    struct dirblock_t dirBlock;
    bfs.read_block(curr_dir, (void *) &dirBlock);

    if(dirBlock.num_entries == MAX_DIR_ENTRIES){
        cout << "Directory full.\n";
        return;
    }

    for(int x = 0; x < dirBlock.num_entries; x++){
        if(strcmp(dirBlock.dir_entries[x].name, name) == 0){
            cout << "File exists.\n";
            return;
        }
    }

    short blockNum = bfs.get_free_block();

    if(blockNum == 0){
        cout << "Disk is full.\n";
        return;
    }

    // initialize the new block
    struct dirblock_t newDirBlock;
    newDirBlock.magic = DIR_MAGIC_NUM;
    newDirBlock.num_entries = 0;
    
    // write it to disk
    bfs.write_block(blockNum, (void *) &newDirBlock);

    // set the new block in the current directory
    strcpy(dirBlock.dir_entries[dirBlock.num_entries].name, name);
    dirBlock.dir_entries[dirBlock.num_entries].block_num = blockNum;
    dirBlock.num_entries++;

    // write the change to disk
    bfs.write_block(curr_dir, (void *) &dirBlock);
}

// switch to a directory
void FileSys::cd(const char *name)
{   
    // looks for file with name
    short blockNum = findBlock(name);
    if(!blockNum) return;

    // file must be a directory
    if(!is_directory(blockNum)){
        cout << "File is not a directory.\n";
        return;
    }

    // set the current directory to the block
    curr_dir = blockNum;
}

// switch to home directory
void FileSys::home() {
    curr_dir = 1;
}

// remove a directory
void FileSys::rmdir(const char *name)
{   
    // search for block to be deleted
    short blockNum = findBlock(name);

    if(!blockNum) return;

    if(!is_directory(blockNum)){
        cout << "File is not a directory.\n";
        return;
    }

    struct dirblock_t dirBlock;
    bfs.read_block(blockNum, (void *) &dirBlock);

    if(dirBlock.num_entries != 0){
        cout << "Directory is not empty.\n";
        return;
    }

    // reclaim the block in superblock
    bfs.reclaim_block(blockNum);

    // get the current directory
    bfs.read_block(curr_dir, (void *) &dirBlock);

    int deleteIndex = 0;

    // find the entry and swap indexes if entry is not the last one in array
    for(int x = 0; x < dirBlock.num_entries; x++){
        if(strcmp(dirBlock.dir_entries[x].name, name) == 0){
            deleteIndex = x;
            break;
        }
    }

    dirBlock.num_entries--;

    swap(dirBlock.dir_entries[deleteIndex], 
    dirBlock.dir_entries[dirBlock.num_entries]);

    // write these changes to the directory block
    bfs.write_block(curr_dir, (void *) &dirBlock);
}

// list the contents of current directory
void FileSys::ls()
{
    // reads the current directory
    struct dirblock_t dirBlock;
    bfs.read_block(curr_dir, (void *) &dirBlock);

    if(dirBlock.num_entries == 0) return;

    // prints out all file names in directory
    for(int x = 0; x < dirBlock.num_entries; x++){
        cout << dirBlock.dir_entries[x].name;
        if(is_directory(dirBlock.dir_entries[x].block_num)){
            cout << "/";
        }
        cout << endl;
    }
}

// create an empty data file
void FileSys::create(const char *name)
{
    if(strlen(name) > MAX_FNAME_SIZE){
        cout << "File name is too long.\n";
        return;
    }

    struct dirblock_t dirBlock;
    bfs.read_block(curr_dir, (void *) &dirBlock);

    short blockNum = 0;

    // checks for file existence in current directory
    for(int x = 0; x < dirBlock.num_entries; x++){
        if(strcmp(dirBlock.dir_entries[x].name, name) == 0){
            blockNum = dirBlock.dir_entries[x].block_num;
            break;
        }
    }

    if(blockNum != 0){
        cout << "File exists.\n";
        return;
    }

    if(dirBlock.num_entries == MAX_DIR_ENTRIES){
        cout << "Directory is full.\n";
        return;
    }

    blockNum = bfs.get_free_block();
    if(!blockNum){
        cout << "Disk is full.\n";
        return;
    }   

    // initializes an inode
    struct inode_t inode;
    inode.magic = INODE_MAGIC_NUM;
    inode.size = 0;

    // writes initialization to disk
    bfs.write_block(blockNum, (void *) &inode);

    // insert inode into directory block
    strcpy(dirBlock.dir_entries[dirBlock.num_entries].name, name);
    dirBlock.dir_entries[dirBlock.num_entries].block_num = blockNum;
    dirBlock.num_entries++;

    // write directory block changes
    bfs.write_block(curr_dir, (void *) &dirBlock);
}

// append data to a data file
void FileSys::append(const char *name, const char *data)
{
    short blockNum = findBlock(name);
    if(!blockNum) return;
    
    if(is_directory(blockNum)){
        cout << "File is a directory.\n";
        return;
    }

    struct inode_t inode;

    bfs.read_block(blockNum, (void *) &inode);

    // check if data to be added will exceed file size, do not add
    int dataSize = strlen(data);
    if((dataSize + inode.size) > MAX_FILE_SIZE){
        cout << "Append exceeds maximum file size.\n";
        return;
    }

    struct datablock_t writeBlock;

    // for loop adds characters incrementally, when the current
    // working block is equal to the number of blocks a new
    // datablock must be added
    for(int x = 0; x < dataSize; x++){
        // when current block is full / no datablocks in file
        // get a new block
        if(inodeNumBlocks(blockNum) == inodeCurrBlock(blockNum)){
            short newBlockNum = bfs.get_free_block();

            // check if disk is full
            if(!newBlockNum){
                cout << "Disk is full.\n";
                return;
            }

            // if new block allocated, set in inode array
            inode.blocks[inodeCurrBlock(blockNum)] = newBlockNum;
        }

        // only read the data block if it is 
        // the start of loop or a new datablock
        if(x == 0 || inodeNumBlocks(blockNum) == inodeCurrBlock(blockNum)){
            bfs.read_block(inode.blocks[inodeCurrBlock(blockNum)],
            (void *) &writeBlock);
        }

        // write data to datablock
        writeBlock.data[inode.size % BLOCK_SIZE] = data[x];

        // write all changes to data block
        bfs.write_block(inode.blocks[inodeCurrBlock(blockNum)], (void *) &writeBlock);

        // increment file size and write all changes to inode block 
        inode.size++;
        bfs.write_block(blockNum, (void *) &inode);
    }
}

// display the contents of a data file
void FileSys::cat(const char *name)
{   
    // searches for inode
    short blockNum = findBlock(name);
    if(!blockNum) return;
    
    if(is_directory(blockNum)){
        cout << "File is a directory.\n";
        return;
    }

    struct inode_t inode;

    bfs.read_block(blockNum, (void *) &inode);

    struct datablock_t dataBlock;

    // iterations needed to read file
    int numBlocks = inodeNumBlocks(blockNum);

    int bytes = inode.size;

    for(int x = 0; x < numBlocks; x++){
        bfs.read_block(inode.blocks[x], (void *) &dataBlock);

        // if the inode size - datablock size is above or equal
        // to zero then read all 128 bytes
        if(bytes - BLOCK_SIZE >= 0){
            for(int y = 0; y < BLOCK_SIZE; y++){
                cout << dataBlock.data[y];
            }
            bytes -= BLOCK_SIZE;
        }else{
            // print out partially filled block
            for(int z = 0; z < inode.size % BLOCK_SIZE; z++){
                cout << dataBlock.data[z];
            }
        }
    }

    cout << endl;
}

// delete a data file
void FileSys::rm(const char *name)
{
    short blockNum = findBlock(name);
    if(!blockNum) return;

    if(is_directory(blockNum)){
        cout << "File is a directory.\n";
        return;
    }

    struct inode_t inode;
    bfs.read_block(blockNum, (void *) &inode);

    // deletes all data blocks in file
    int deleteBlocks = inodeNumBlocks(blockNum);
    for(int x = 0; x < deleteBlocks; x++){
        bfs.reclaim_block(inode.blocks[x]);
    }

    // reclaim inode block on superblock
    bfs.reclaim_block(blockNum);

    // load current directory block
    struct dirblock_t dirBlock;
    bfs.read_block(curr_dir, (void *) &dirBlock);

    int deleteIndex = 0;

    // find index in directory of the deleted file, delete and swap elements
    for(int x = 0; x < dirBlock.num_entries; x++){
        if(strcmp(dirBlock.dir_entries[x].name, name) == 0){
            deleteIndex = x;
            break;
        }
    }

    dirBlock.num_entries--;

    swap(dirBlock.dir_entries[deleteIndex], 
    dirBlock.dir_entries[dirBlock.num_entries]);

    // write changes
    bfs.write_block(curr_dir, (void *) &dirBlock);
}

// display stats about file or directory
void FileSys::stat(const char *name)
{
    short blockNum = findBlock(name);
    if(!blockNum) return;

    if(is_directory(blockNum)){
        cout << "Directory block: " << blockNum << endl;
    }else{
        struct inode_t inode;
        bfs.read_block(blockNum, (void *) &inode);
        cout << "Inode block: " << blockNum << endl;
        cout << "Bytes in file: " << inode.size << endl;
        // add 1 to data blocks to include inode
        cout << "Number of blocks: " << inodeNumBlocks(blockNum) + 1 << endl;
    }
}

// HELPER FUNCTIONS (optional)

// helper function to check if the block is a directory
bool FileSys::is_directory(short block_num)
{  
    // loads directory block and then checks magic number
    struct dirblock_t dirBlock;

    bfs.read_block(block_num, (void *) &dirBlock);
    if(dirBlock.magic == DIR_MAGIC_NUM) return true;

    return false;
}

// checks if name is valid and if block is in directory
// returns 0 or block num
short FileSys::findBlock(const char *name)
{
    if(strlen(name) > MAX_FNAME_SIZE){
        cout << "File name is too long.\n";
        return false;
    }

    struct dirblock_t dirBlock;
    bfs.read_block(curr_dir, (void *) &dirBlock);

    short blockNum = 0;

    // searches directory entries for matching file name
    for(int x = 0; x < dirBlock.num_entries; x++){
        if(strcmp(dirBlock.dir_entries[x].name, name) == 0){
            blockNum = dirBlock.dir_entries[x].block_num;
            break;
        }
    }

    if(blockNum == 0){
        cout << "File does not exist.\n";
        return false;
    }

    return blockNum;
}

// returns the number of blocks used by the inode 
int FileSys::inodeNumBlocks(short block_num)
{
    struct inode_t inode;
    bfs.read_block(block_num, (void *) &inode);

    return ceil((double) inode.size / BLOCK_SIZE);
}

// returns the current index of working block of the inode
int FileSys::inodeCurrBlock(short block_num)
{
    struct inode_t inode;
    bfs.read_block(block_num, (void *) &inode);

    return floor((double) inode.size / BLOCK_SIZE);
}
