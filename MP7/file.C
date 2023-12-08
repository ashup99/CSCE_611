/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

// #define _LARGE_FILE_

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id)
{
#ifdef _LARGE_FILE_
    Console::puts("File - start\n");
    Console::puts("Opening file: ");
    Console::puti(_id);
    Console::puts("\n");
    current_position = 0;
    current_data_block_no = 0;
    fs = _fs;
    current_inode = fs->LookupFile(_id);
    fs->ReadFromDisk(current_inode->index_block_no, (unsigned char *)data_block);
    fs->ReadFromDisk(data_block[current_data_block_no], block_cache);
    Console::puts("File - end\n");
#else
    Console::puts("File - start\n");
    Console::puts("Opening file: ");
    Console::puti(_id);
    Console::puts("\n");
    current_position = 0;
    fs = _fs;
    current_inode = fs->LookupFile(_id);
    fs->ReadFromDisk(current_inode->block_no, block_cache);
    Console::puts("File - end\n");
#endif
}

File::~File()
{
#ifdef _LARGE_FILE_
    Console::puts("~File - start\n");
    Console::puts("Closing file: ");
    Console::puti(current_inode->id);
    Console::puts("\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
    current_inode->updateInodesList();
    fs->WriteToDisk(data_block[current_data_block_no], block_cache);
    fs->WriteToDisk(current_inode->index_block_no, (unsigned char *)data_block);
    Console::puts("~File - end\n");
#else
    Console::puts("~File - start\n");
    Console::puts("Closing file: ");
    Console::puti(current_inode->id);
    Console::puts("\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
    fs->WriteToDisk(current_inode->block_no, block_cache);
    current_inode->updateInodesList();
    Console::puts("~File - end\n");
#endif
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf)
{
#ifdef _LARGE_FILE_
    Console::puts("Read - start\n");
    Console::puts("reading from file ");
    Console::puti(current_inode->id);
    Console::puts("\n");
    int char_count = 0;
    while (!EoF() && char_count < _n)
    {
        unsigned int number_of_blocks = current_position / FileSystem::MAX_FREE_BLOCKS;
        unsigned int data_index_block = current_position % FileSystem::MAX_FREE_BLOCKS;
        if (number_of_blocks > current_data_block_no)
        {
            current_data_block_no = number_of_blocks;
            fs->ReadFromDisk(data_block[current_data_block_no], block_cache);
        }
        _buf[char_count] = block_cache[data_index_block];
        char_count += 1;
        current_position += 1;
    }
    Console::puts("Read - end\n");
    return char_count;
#else
    Console::puts("Read - start\n");
    Console::puts("reading from file ");
    Console::puti(current_inode->id);
    Console::puts("\n");
    int char_count = 0;
    if (_n > SimpleDisk::BLOCK_SIZE)
    {
        _n = SimpleDisk::BLOCK_SIZE;
        Console::puts("Beyond 512 byte cannot be read from the file\n");
    }
    // int current_position_itr=current_position;
    while (!EoF() && char_count < _n)
    {
        _buf[char_count] = block_cache[current_position];
        // _buf[char_count]=block_cache[current_position_itr];
        char_count += 1;
        current_position += 1;
        // current_position_itr+=1;
    }
    Console::puts("Read - end\n");
    return char_count;
#endif
}

int File::Write(unsigned int _n, const char *_buf)
{
#ifdef _LARGE_FILE_
    Console::puts("Write- start\n");
    Console::puts("writing to file ");
    Console::puti(current_inode->id);
    Console::puts("\n");

    int char_count = 0;

    while (current_position < FileSystem::MAX_FILE_SIZE && char_count < _n)
    {
        unsigned int number_of_blocks = current_position / FileSystem::MAX_FREE_BLOCKS;
        unsigned int data_index_block = current_position % FileSystem::MAX_FREE_BLOCKS;
        if (number_of_blocks > current_data_block_no)
        {
            fs->WriteToDisk(data_block[current_data_block_no], block_cache);
            current_data_block_no = number_of_blocks;
            if (current_data_block_no < current_inode->number_of_blocks)
            {
                fs->ReadFromDisk(data_block[current_data_block_no], block_cache);
            }
            else
            {
                unsigned long new_block = current_inode->getAndWriteFreeBlock();
                if (new_block != FileSystem::MAX_FREE_BLOCKS)
                {
                    data_block[current_data_block_no] = new_block;
                    fs->WriteToDisk(current_inode->index_block_no, (unsigned char *)data_block);
                    current_inode->number_of_blocks++;
                    current_inode->updateInodesList();
                }
                else
                {
                    Console::puts("MAX_FREE_BLOCKS reached.");
                    return char_count;
                }
            }
        }
        block_cache[data_index_block] = _buf[char_count];
        char_count += 1;
        current_position += 1;
    }
    if (current_position > current_inode->file_size)
    {
        current_inode->file_size = current_position;
    }
    Console::puts("Write- end\n");
    return char_count;
// assert(false);
#else
    Console::puts("Write- start\n");
    Console::puts("writing to file ");
    Console::puti(current_inode->id);
    Console::puts("\n");

    int char_count = 0;
    if (_n > SimpleDisk::BLOCK_SIZE)
    {
        _n = SimpleDisk::BLOCK_SIZE;
        Console::puts("Beyond 512 byte cannot be written to the file\n");
    }
    // int current_position_itr=current_position;
    unsigned int file_size = current_position + _n;
    if (file_size > current_inode->file_size)
    {
        if (file_size <= SimpleDisk::BLOCK_SIZE)
        {
            current_inode->file_size = file_size;
        }
        else
        {
            current_inode->file_size = SimpleDisk::BLOCK_SIZE;
        }
    }
    while (current_position < FileSystem::MAX_FILE_SIZE && char_count < _n)
    {
        block_cache[current_position] = _buf[char_count];
        char_count += 1;
        current_position += 1;
    }
    if (current_position > current_inode->file_size)
    {
        current_inode->file_size = current_position;
    }
    Console::puts("Write- end\n");
    return char_count;
// assert(false);
#endif
}

void File::Reset()
{
#ifdef _LARGE_FILE_
    Console::puts("Reset - start\n");
    Console::puts("resetting file: ");
    Console::puti(current_inode->id);
    Console::puts("\n");
    current_position = 0;
    if (current_data_block_no != 0)
    {
        for (int itr = current_data_block_no; itr >= 0; itr--)
        {
            fs->WriteToDisk(data_block[itr], block_cache);
        }
    }
    Console::puts("Reset - end\n");
// assert(false);
#else
    Console::puts("Reset - start\n");
    Console::puts("resetting file: ");
    Console::puti(current_inode->id);
    Console::puts("\n");
    current_position = 0;
    Console::puts("Reset - end\n");
// assert(false);
#endif
}

bool File::EoF()
{
    // Console::puts("checking for EoF - start\n");
    // Console::puts("checking for EoF: ");
    // Console::puti(current_inode->id);
    // Console::puts("\n");
    // Console::puts("checking for EoF - end\n");
    return current_position == current_inode->file_size;
    // assert(false);
}
