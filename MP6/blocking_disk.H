/*
     File        : blocking_disk.H

     Author      : Ashutosh Punyani
     Modified    : November 13, 2023

     Description :

*/

#ifndef _BLOCKING_DISK_H_
#define _BLOCKING_DISK_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "simple_disk.H"
#include "mem_pool.H"
#include "thread.H"

extern MemPool *MEMORY_POOL;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* B l o c k i n g D i s k  */
/*--------------------------------------------------------------------------*/

class BlockingDisk : public SimpleDisk
{

protected:
   virtual void wait_until_ready();
   /* Is called after each read/write operation to check whether the disk is ready to start transfering the data from/to the disk. */
public:
   BlockingDisk(DISK_ID _disk_id, unsigned int _size);
   /* Creates a BlockingDisk device with the given size connected to the
      MASTER or SLAVE slot of the primary ATA controller.
      NOTE: We are passing the _size argument out of laziness.
      In a real system, we would infer this information from the
      disk controller. */

   /* DISK OPERATIONS */

   virtual void read(unsigned long _block_no, unsigned char *_buf);
   /* Reads 512 Bytes from the given block of the disk and copies them
      to the given buffer. No error check! */

   virtual void write(unsigned long _block_no, unsigned char *_buf);
   /* Writes 512 Bytes from the buffer to the given block on the disk. */

   virtual bool is_ready_blocked();
   /* Return true if disk is ready to transfer data from/to disk, false otherwise. */
};

#endif
