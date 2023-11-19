/*
     File        : mirroring_disk.C

     Author      : Ashutosh Punyani
     Modified    : November 14, 2023
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
#include "mirroring_disk.H"
#include "scheduler.H"
#include "machine.H"

extern Scheduler *SYSTEM_SCHEDULER;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

MirroringDisk::MirroringDisk(DISK_ID _disk_id, unsigned int _size) : BlockingDisk(_disk_id, _size)
{
  MASTER_DISK = new BlockingDisk(DISK_ID::MASTER, _size);
  DEPENDENT_DISK = new BlockingDisk(DISK_ID::DEPENDENT, _size);
}

/*--------------------------------------------------------------------------*/
/* MIRROING_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void MirroringDisk::wait_until_ready_mirroring()
{
  while (!MASTER_DISK->is_ready_blocked() && !DEPENDENT_DISK->is_ready_blocked())
  {
    SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
    SYSTEM_SCHEDULER->yield();
  }
}

void MirroringDisk::issue_operation_mirroring(DISK_OPERATION _op, unsigned long _block_no, DISK_ID disk_id)
{

  Machine::outportb(0x1F1, 0x00); /* send NULL to port 0x1F1         */
  Machine::outportb(0x1F2, 0x01); /* send sector count to port 0X1F2 */
  Machine::outportb(0x1F3, (unsigned char)_block_no);
  /* send low 8 bits of block number */
  Machine::outportb(0x1F4, (unsigned char)(_block_no >> 8));
  /* send next 8 bits of block number */
  Machine::outportb(0x1F5, (unsigned char)(_block_no >> 16));
  /* send next 8 bits of block number */
  unsigned int disk_no = disk_id == DISK_ID::MASTER ? 0 : 1;
  Machine::outportb(0x1F6, ((unsigned char)(_block_no >> 24) & 0x0F) | 0xE0 | (disk_no << 4));
  /* send drive indicator, some bits,
     highest 4 bits of block no */

  Machine::outportb(0x1F7, (_op == DISK_OPERATION::READ) ? 0x20 : 0x30);
}

void MirroringDisk::read(unsigned long _block_no, unsigned char *_buf)
{
  /* Reads 512 Bytes in the given block of the given disk drive and copies them
     to the given buffer. No error check! */

  issue_operation_mirroring(DISK_OPERATION::READ, _block_no, DISK_ID::MASTER);
  issue_operation_mirroring(DISK_OPERATION::READ, _block_no, DISK_ID::DEPENDENT);

  wait_until_ready_mirroring();

  /* read data from port */
  int i;
  unsigned short tmpw;
  for (i = 0; i < 256; i++)
  {
    tmpw = Machine::inportw(0x1F0);
    _buf[i * 2] = (unsigned char)tmpw;
    _buf[i * 2 + 1] = (unsigned char)(tmpw >> 8);
  }
}

void MirroringDisk::write(unsigned long _block_no, unsigned char *_buf)
{
  MASTER_DISK->write(_block_no, _buf);
  DEPENDENT_DISK->write(_block_no, _buf);
}
