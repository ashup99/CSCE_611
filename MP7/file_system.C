/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define INODES_INDEX 0
#define FREELIST_INDEX 1
#define USED 'u'
#define FREE 'f'
// #define _LARGE_FILE_

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store
   inodes from and to disk. */

/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* INODE FUNCTIONS */
/*--------------------------------------------------------------------------*/
#ifdef _LARGE_FILE_
void Inode::init(FileSystem *_fs, long _file_id, unsigned long _index_block_no)
{
    Console::puts("Large Files Inode Init\n");
    fs = _fs;
    id = _file_id;
    index_block_no = _index_block_no;
    is_inode_free = false;
    file_size = 0;
    number_of_blocks = 1;
}
#else
void Inode::init(FileSystem *_fs, long _file_id, unsigned long _block_no)
{
    Console::puts("Small Files Inode Init\n");
    fs = _fs;
    id = _file_id;
    block_no = _block_no;
    is_inode_free = false;
    file_size = 0;
}
#endif

void Inode::updateInodesList()
{
    fs->WriteToDisk(INODES_INDEX, (unsigned char *)fs->inodes);
}

#ifdef _LARGE_FILE_
unsigned long Inode::getAndWriteFreeBlock()
{
    unsigned long data_block_no = fs->GetFreeBlock();
    if (data_block_no != FileSystem::MAX_FREE_BLOCKS)
    {
        fs->WriteToDisk(FREELIST_INDEX, fs->free_blocks);
    }
    return data_block_no;
}
#endif

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem()
{
    Console::puts("FileSystem - start\n");
    Console::puts("In file system constructor.\n");
    size = 0;
    disk = NULL;
    inodes = new Inode[MAX_INODES];
    free_blocks = new unsigned char[MAX_FREE_BLOCKS];
    Console::puts("FileSystem - end\n");
    // assert(false);
}

FileSystem::~FileSystem()
{
    Console::puts("~FileSystem - start\n");
    Console::puts("unmounting file system\n");
    /* Make sure that the inode list and the free list are saved. */
    WriteToDisk(INODES_INDEX, (unsigned char *)inodes);
    WriteToDisk(FREELIST_INDEX, free_blocks);
    disk = NULL;
    delete[] inodes;
    delete[] free_blocks;
    Console::puts("~FileSystem - end\n");
    // assert(false);
}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

void FileSystem::ReadFromDisk(unsigned long _block_no, unsigned char *_buf)
{
    disk->read(_block_no, _buf);
}

void FileSystem::WriteToDisk(unsigned long _block_no, unsigned char *_buf)
{
    disk->write(_block_no, _buf);
}

unsigned long FileSystem::GetFreeBlock()
{
    Console::puts("GetFreeBlock - start\n");
    unsigned long free_block_no = MAX_FREE_BLOCKS;
    for (int itr = 0; itr < MAX_FREE_BLOCKS; itr++)
    {
        if (free_blocks[itr] == FREE)
        {
            free_blocks[itr] = USED;
            free_block_no = itr;
            break;
        }
    }
    if (free_block_no != MAX_FREE_BLOCKS)
    {
        Console::puts("Found a free block at: ");
        Console::puti(free_block_no);
        Console::puts("\n");
    }

    Console::puts("GetFreeBlock - end\n");
    return free_block_no;
}

Inode *FileSystem::GetFreeInode()
{
    Console::puts("GetFreeInode - start\n");

    Inode *free_inode = NULL;
    for (int itr = 0; itr < MAX_INODES; itr++)
    {
        if (inodes[itr].is_inode_free)
        {
            inodes[itr].is_inode_free = false;
            free_inode = &inodes[itr];
            break;
        }
    }
    if (free_inode != NULL)
    {
        Console::puts("Found a free inode\n");
    }
    Console::puts("GetFreeInode - end\n");
    return free_inode;
}

bool FileSystem::Mount(SimpleDisk *_disk)
{
    /* Here you read the inode list and the free list into memory */
    Console::puts("Mount - start\n");
    Console::puts("mounting file system from disk\n");
    if (disk != NULL)
    {
        Console::puts("Disk is already mounted\n");
        Console::puts("Mount - end\n");
        return false;
    }
    disk = _disk;
    ReadFromDisk(INODES_INDEX, (unsigned char *)inodes);
    ReadFromDisk(FREELIST_INDEX, free_blocks);
    Console::puts("Mount - end\n");
    return true;
    // assert(false);
}

bool FileSystem::Format(SimpleDisk *_disk, unsigned int _size)
{ // static!
    Console::puts("Format - start\n");
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */
    Inode *temp_inodes = new Inode[MAX_INODES];
    for (int itr = 0; itr < MAX_INODES; itr++)
        temp_inodes[itr].is_inode_free = true;
    _disk->write(INODES_INDEX, (unsigned char *)temp_inodes);

    unsigned char *temp_free_blocks = new unsigned char[MAX_FREE_BLOCKS];
    temp_free_blocks[INODES_INDEX] = USED;
    temp_free_blocks[FREELIST_INDEX] = USED;
    for (int itr = 2; itr < MAX_FREE_BLOCKS; itr++)
        temp_free_blocks[itr] = FREE;
    _disk->write(FREELIST_INDEX, (unsigned char *)temp_free_blocks);
    delete[] temp_inodes;
    delete[] temp_free_blocks;
    Console::puts("Format - end\n");
    return true;
}

Inode *FileSystem::LookupFile(int _file_id)
{
    Console::puts("LookupFile - start\n");
    Console::puts("looking up file with id = ");
    Console::puti(_file_id);
    Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    Inode *inode_found = NULL;
    int found = 0;
    for (int itr = 0; itr < MAX_INODES; itr++)
    {
        if (inodes[itr].id == _file_id && !inodes[itr].is_inode_free)
        {
            Console::puts("Inode Found For: ");
            Console::puti(_file_id);
            Console::puts("\n");
            inode_found = &inodes[itr];
            found = 1;
            break;
        }
    }
    if (inode_found != NULL)
    {
        Console::puts("Found a inode for the given file id\n");
    }
    Console::puts("LookupFile - end\n");
    return inode_found;
    // assert(false);
}

bool FileSystem::CreateFile(int _file_id)
{
    Console::puts("CreateFile - start\n");
    Console::puts("creating file with id: ");
    Console::puti(_file_id);
    Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
    // assert(false);
    Inode *inode_found = LookupFile(_file_id);
    if (inode_found != NULL)
    {
        Console::puts("File Already Exists for: ");
        Console::puti(_file_id);
        Console::puts("\n");
        Console::puts("CreateFile - end\n");
        return false;
    }
#ifdef _LARGE_FILE_
    Inode *free_inode = GetFreeInode();
    unsigned long index_block_no = GetFreeBlock();
    unsigned long data_block_no = GetFreeBlock();
    if (free_inode != NULL && index_block_no != MAX_FREE_BLOCKS && data_block_no != MAX_FREE_BLOCKS)
    {
        free_inode->init(this, _file_id, index_block_no);
        WriteToDisk(INODES_INDEX, (unsigned char *)inodes);
        WriteToDisk(FREELIST_INDEX, free_blocks);

        unsigned long *index_block = new unsigned long[MAX_FREE_BLOCKS / 4];
        index_block[0] = data_block_no;
        WriteToDisk(index_block_no, (unsigned char *)index_block);
        delete[] index_block;
        Console::puts("File Created SuccessFully For: ");
        Console::puti(_file_id);
        Console::puts("\n");
        Console::puts("CreateFile - end\n");
        return true;
    }
    else
    {
        if (free_inode != NULL)
            free_inode->is_inode_free = true;
        if (index_block_no != MAX_FREE_BLOCKS)
            free_blocks[index_block_no] = FREE;
        if (data_block_no != MAX_FREE_BLOCKS)
            free_blocks[data_block_no] = FREE;
        Console::puts("File Creation Failed for: ");
        Console::puti(_file_id);
        Console::puts("\n");
        Console::puts("CreateFile - end\n");
        return false;
    }
#else
    Inode *free_inode = GetFreeInode();
    unsigned long block_no = GetFreeBlock();
    if (free_inode != NULL && block_no != MAX_FREE_BLOCKS)
    {
        free_inode->init(this, _file_id, block_no);
        WriteToDisk(INODES_INDEX, (unsigned char *)inodes);
        WriteToDisk(FREELIST_INDEX, free_blocks);
        Console::puts("File Created SuccessFully For: ");
        Console::puti(_file_id);
        Console::puts("\n");
        Console::puts("CreateFile - end\n");
        return true;
    }
    else
    {
        if (free_inode != NULL)
            free_inode->is_inode_free = true;
        if (block_no != MAX_FREE_BLOCKS)
            free_blocks[block_no] = FREE;
        Console::puts("File Creation Failed for: ");
        Console::puti(_file_id);
        Console::puts("\n");
        Console::puts("CreateFile - end\n");
        return false;
    }
#endif
}

bool FileSystem::DeleteFile(int _file_id)
{
#ifdef _LARGE_FILE_
    Console::puts("DeleteFile - start\n");
    Console::puts("deleting file with id: ");
    Console::puti(_file_id);
    Console::puts("\n");
    /* First, check if the file exists. If not, throw an error.
       Then free all blocks that belong to the file and delete/invalidate
       (depending on your implementation of the inode list) the inode. */
    // assert(false);

    Inode *inode_found = LookupFile(_file_id);
    if (inode_found == NULL)
    {
        Console::puts("File Does not Exists for: ");
        Console::puti(_file_id);
        Console::puts("\n");
        Console::puts("DeleteFile - end\n");
        return false;
    }
    inode_found->is_inode_free = true;
    unsigned long index_block_no = inode_found->index_block_no;
    free_blocks[index_block_no] = FREE;

    unsigned long *data_index_block = new unsigned long[MAX_FREE_BLOCKS / 4];
    ReadFromDisk(index_block_no, (unsigned char *)data_index_block);
    for (int itr = 0; itr < inode_found->number_of_blocks; itr++)
    {
        unsigned long data_index_block_no = data_index_block[itr];
        free_blocks[data_index_block_no] = FREE;
    }
    delete[] data_index_block;
    WriteToDisk(INODES_INDEX, (unsigned char *)inodes);
    WriteToDisk(FREELIST_INDEX, free_blocks);
    Console::puts("File Deleted ");
    Console::puti(_file_id);
    Console::puts("\n");
    Console::puts("DeleteFile - end\n");
    return true;
#else
    Console::puts("DeleteFile - start\n");
    Console::puts("deleting file with id: ");
    Console::puti(_file_id);
    Console::puts("\n");
    /* First, check if the file exists. If not, throw an error.
       Then free all blocks that belong to the file and delete/invalidate
       (depending on your implementation of the inode list) the inode. */
    // assert(false);

    Inode *inode_found = LookupFile(_file_id);
    if (inode_found == NULL)
    {
        Console::puts("File Does not Exists for: ");
        Console::puti(_file_id);
        Console::puts("\n");
        Console::puts("DeleteFile - end\n");
        return false;
    }
    inode_found->is_inode_free = true;
    unsigned long block_no = inode_found->block_no;
    free_blocks[block_no] = FREE;
    WriteToDisk(INODES_INDEX, (unsigned char *)inodes);
    WriteToDisk(FREELIST_INDEX, free_blocks);
    Console::puts("File Deleted ");
    Console::puti(_file_id);
    Console::puts("\n");
    Console::puts("DeleteFile - end\n");
    return true;
#endif
}
