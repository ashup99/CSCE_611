#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;



void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   // initlizaing basic data structure for paging
   Console::puts("Initialized Paging System Start\n");

   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;

   Console::puts("Initialized Paging System End\n");
}

PageTable::PageTable()
{
   Console::puts("Constructed Page Table object Start\n");
   unsigned long page_directory_frame_number = kernel_mem_pool->get_frames(1);
   page_directory = (unsigned long *)(page_directory_frame_number * PAGE_SIZE);

   unsigned long page_table_frame_number = kernel_mem_pool->get_frames(1);
   unsigned long *page_table = (unsigned long*)(page_table_frame_number * PAGE_SIZE);

  

   // mappng the first 4MB of memory
   for(unsigned int i = 0, physical_address = 0; i < ENTRIES_PER_PAGE; i++,physical_address + PAGE_SIZE)
   { 
      page_table[i] = physical_address | 0x3; // attribute set to: supervisor level, read/write, present(011 in binary)
   }

   page_directory[0] = page_table; // attribute set to: supervisor level, read/write, present(011 in binary)
   page_directory[0] = page_directory[0] | 0x3;

   for(unsigned int i = 1; i < ENTRIES_PER_PAGE; i++)
   {
      page_directory[i] = 0 | 0x2; // attribute set to: supervisor level, read/write, not present(010 in binary)
   }

   Console::puts("Constructed Page Table object End\n");
}


void PageTable::load()
{
   assert(false);
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   assert(false);
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
  assert(false);
  Console::puts("handled page fault\n");
}

