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

extern Scheduler *SYSTEM_SCHEDULER;
/*--------------------------------------------------------------------------*/
/* THREAD SAFE DISK SYSTEM IMPLMENTATION*/
/*--------------------------------------------------------------------------*/

int isLocked;
int TestAndSet(int *isLocked, int new_lock_state)
{
  int previous_state = *isLocked;
  *isLocked = new_lock_state;
  return previous_state;
}

void initialize_lock(int *isLocked)
{
  *isLocked = 0;
}

void acquire_lock()
{
  while (TestAndSet(&isLocked, 1))
    ;
}

void release_lock()
{
  isLocked = 0;
}

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size)
    : SimpleDisk(_disk_id, _size)
{
  Console::puts("Constructed BlockingDisk::BlockingDisk() - start.\n");
  initialize_lock(&isLocked);
  Console::puts("Constructed BlockingDisk::BlockingDisk() - end.\n");
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char *_buf)
{
  acquire_lock();
  Console::puts("BlockingDisk::read() - start.\n");
  SimpleDisk::read(_block_no, _buf);
  Console::puts("BlockingDisk::read() - end.\n");
  release_lock();
}

void BlockingDisk::write(unsigned long _block_no, unsigned char *_buf)
{
  acquire_lock();
  Console::puts("BlockingDisk::write() - start.\n");
  SimpleDisk::write(_block_no, _buf);
  Console::puts("BlockingDisk::write() - end.\n");
  release_lock();
}

bool BlockingDisk::is_ready_blocked()
{
  return SimpleDisk::is_ready();
}

void BlockingDisk::wait_until_ready()
{
  while (!SimpleDisk::is_ready())
  {
    SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
    SYSTEM_SCHEDULER->yield();
  }
}