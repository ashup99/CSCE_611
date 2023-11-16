/*
     File        : blocking_disk.c

     Author      : Ashutosh Punyani
     Modified    : November 13, 2023

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "scheduler.H"

extern Scheduler* SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
  Console::puts("Constructed BlockingDisk::BlockingDisk() - start.\n");
  Console::puts("Constructed BlockingDisk::BlockingDisk() - end.\n");
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
  Console::puts("BlockingDisk::read() - start.\n");
  SimpleDisk::read(_block_no, _buf);
  Console::puts("BlockingDisk::read() - end.\n");

}

void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
  Console::puts("BlockingDisk::write() - start.\n");
  SimpleDisk::write(_block_no, _buf);
  Console::puts("BlockingDisk::write() - end.\n");
}

bool BlockingDisk::is_ready_blocked(){
  return SimpleDisk::is_ready();
}

void BlockingDisk::wait_until_ready(){
  while(!SimpleDisk::is_ready()){
    SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
    SYSTEM_SCHEDULER->yield();
  }
}