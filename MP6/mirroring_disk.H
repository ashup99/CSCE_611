/*
     File        : mirroring_disk.H

     Author      : Ashutosh Punyani
     Modified    : November 14, 2023
     Description :
*/

#ifndef _MIRRORING_DISK_H_
#define _MIRRORING_DISK_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "simple_disk.H"
#include "blocking_disk.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* M i r r o r i n g D i s k  */
/*--------------------------------------------------------------------------*/

class MirroringDisk : public BlockingDisk
{
private:
   BlockingDisk *MASTER_DISK;
   BlockingDisk *DEPENDENT_DISK;

   void issue_operation_mirroring(DISK_OPERATION _op, unsigned long _block_no, DISK_ID disk_id);

protected:
   /* -- HERE WE CAN DEFINE THE BEHAVIOR OF DERIVED DISKS */

   virtual void wait_until_ready_mirroring();
   /* Is called after each read operation to check whether the disk is
      ready to start transfering the data from the disk. */

public:
   MirroringDisk(DISK_ID _disk_id, unsigned int _size);
   /* Constructor for mirroring disk */

   /* DISK OPERATIONS */

   virtual void read(unsigned long _block_no, unsigned char *_buf);
   /* Reads 512 Bytes from the given block of the disk and copies them
      to the given buffer. No error check! */

   virtual void write(unsigned long _block_no, unsigned char *_buf);
   /* Writes 512 Bytes from the buffer to the given block on the disk. */
};

#endif
