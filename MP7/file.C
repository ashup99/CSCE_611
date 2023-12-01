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

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id) {
    Console::puts("File - start\n");
    Console::puts("Opening file: ");
    Console::puti(_id); 
    Console::puts("\n");
    current_position=0;
    fs=_fs;
    current_inode = fs->LookupFile(_id);
    fs->ReadFromDisk(current_inode->block_no,block_cache);
    Console::puts("File - end\n");
}

File::~File() {
    Console::puts("~File - start\n");
    Console::puts("Closing file: ");
    Console::puti(current_inode->id); 
    Console::puts("\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
    fs->WriteToDisk(current_inode->block_no,block_cache);
    current_inode->updateInodesList();
    Console::puts("~File - end\n");
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("Read - start\n");
    Console::puts("reading from file ");
     Console::puti(current_inode->id); 
    Console::puts("\n");
    int char_count=0;
     if(_n>SimpleDisk::BLOCK_SIZE){
        _n=SimpleDisk::BLOCK_SIZE;
        Console::puts("Beyond 512 byte cannot be read from the file\n");
    }
    // int current_position_itr=current_position;
    while(!EoF() && char_count<_n){
        _buf[char_count]=block_cache[current_position];
        // _buf[char_count]=block_cache[current_position_itr];
        char_count+=1;
        current_position+=1;
        // current_position_itr+=1;
    }
    Console::puts("Read - end\n");
    return char_count;
}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("Write- start\n");
    Console::puts("writing to file ");
    Console::puti(current_inode->id); 
    Console::puts("\n");
    
    int char_count=0;
    if(_n>SimpleDisk::BLOCK_SIZE){
        _n=SimpleDisk::BLOCK_SIZE;
        Console::puts("Beyond 512 byte cannot be written to the file\n");
    }
    // int current_position_itr=current_position;
    unsigned int file_size=current_position+_n;
    if(file_size>current_inode->file_size){
        if(file_size<=SimpleDisk::BLOCK_SIZE){
            current_inode->file_size=file_size;
        }
        else{
            current_inode->file_size=SimpleDisk::BLOCK_SIZE;
        }
    }
    while(current_position<FileSystem::MAX_FILE_SIZE && char_count<_n){
    // while(!EoF() && char_count<_n){
        block_cache[current_position]=_buf[char_count];
        char_count+=1;
        current_position+=1;
    }
    if(current_position>current_inode->file_size){
        current_inode->file_size=current_position;
    }
    Console::puts("Write- end\n");
    return char_count;
    // assert(false);
}

void File::Reset() {
    Console::puts("Reset - start\n");
    Console::puts("resetting file: ");
    Console::puti(current_inode->id); 
    Console::puts("\n");
    current_position=0;
    Console::puts("Reset - end\n");
    // assert(false);
}

bool File::EoF() {
    // Console::puts("checking for EoF - start\n");
    Console::puts("checking for EoF: ");
    Console::puti(current_inode->id); 
    Console::puts("\n");
    // Console::puts("checking for EoF - end\n");
    return current_position==current_inode->file_size;
    // assert(false);
}
