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
   Console::puts("page_directory ");
   Console::putui(*page_directory);
   Console::puts("\n");

   unsigned long page_table_frame_number = kernel_mem_pool->get_frames(1);
   unsigned long *page_table = (unsigned long*)(page_table_frame_number * PAGE_SIZE);
   Console::puts("page table ");
   Console::putui(*page_table);
   Console::puts("\n");

  

   // mappng the first 4MB of memory
   for(unsigned int i = 0, physical_address=0; i < ENTRIES_PER_PAGE; i++,physical_address += PAGE_SIZE)
   { 
         // attribute set to: supervisor level, 
         // read/write, present(011 in binary)
      page_table[i] = physical_address | 0x3; 
   }

   // attribute set to: supervisor level, 
   // read/write, present(011 in binary)
   page_directory[0] = (unsigned long)page_table | 0x3; 

   for(unsigned int i = 1; i < ENTRIES_PER_PAGE; i++)
   {
      // attribute set to: supervisor level, 
      // read/write, not present(010 in binary)
      page_directory[i] = 0 | 0x2; 
   }

   current_page_table = this;
   Console::puts("Constructed Page Table object End\n");
}


void PageTable::load()
{
   Console::puts("Loaded page table Start\n");
   write_cr3((unsigned long)page_directory);
   Console::puts("Loaded page table End\n");
}

void PageTable::enable_paging()
{
   Console::puts("Enabled paging Start\n");
   unsigned long cr0_reg = (unsigned long)(read_cr0() | 0x80000000);
   paging_enabled = 1;
   write_cr0(cr0_reg);
   Console::puts("Enabled paging End\n");

}

void PageTable::handle_fault(REGS * _r)
{
   if((_r->err_code & 0x1) == 0x0){
   unsigned long err_code = _r->err_code;
   Console::puts("err_code ");
   Console::putui(err_code);
   Console::puts("\n");

   unsigned long  faulty_address = (unsigned long)(read_cr2());
   Console::puts("faulty_address ");
   Console::putui(faulty_address);
   Console::puts("\n");
   unsigned long * page_directory_list = (unsigned long *)(read_cr3());
   Console::puts("(unsigned long)page_directory_list ");
   Console::putui((unsigned long)page_directory_list);
   Console::puts("\n");
   // unsigned long directory_location = faulty_address  >> 22;
   unsigned long directory_location = (faulty_address & 0xFFC00000) >> 22;
   Console::puts("directory_location ");
   Console::putui(directory_location);
   Console::puts("\n");
   // unsigned long page_location = (faulty_address & ((1 << 10) - 1))>>12;
   unsigned long page_location = (faulty_address & 0x003FF000) >> 12;
   Console::puts("page_location ");
   Console::putui(page_location);
   Console::puts("\n");

   bool page_table_fault = false;

   Console::puts("page_directory_list[directory_location] ");
   Console::putui(page_directory_list[directory_location]);
   Console::puts("\n");

   Console::puts("page_directory_list[directory_location] & 0x1 ");
   Console::putui(page_directory_list[directory_location] & 0x1);
   Console::puts("\n");

   // if((page_directory_list[directory_location] & 0x1) == 0x0){
   if((page_directory_list[directory_location] & 0x1) != 0x1){
      Console::puts("directory issue ");
      Console::puts("\n");
      unsigned long new_page_table_frame_number = kernel_mem_pool->get_frames(1);
      unsigned long * new_page_table = (unsigned long*)((new_page_table_frame_number * PAGE_SIZE));
      

      // attribute set to: supervisor level, 
      // read/write, not present(010 in binary)
      page_directory_list[directory_location] = (unsigned long) new_page_table | 0x3; 
      Console::puts("new_page_table page_directory_list[directory_location] ");
      Console::putui(page_directory_list[directory_location]);
      Console::puts("\n");
      // initializing the page table enteries
      for(unsigned int i = 0; i < ENTRIES_PER_PAGE; i++)
      { 
         // attribute set to: supervisor level, 
         // read/write, not present(010 in binary)
         new_page_table[i] = 0 | 0x2; 
      }
      unsigned long physical_frame_number = process_mem_pool->get_frames(1);
      new_page_table[page_location] = (unsigned long)(physical_frame_number* PAGE_SIZE)| 0x3;
      Console::puts("new_page_table[page_location] ");
      Console::putui(new_page_table[page_location]);
      Console::puts("\n");

      }
      else {
         unsigned long * existing_page_table = (unsigned long *)(page_directory_list[directory_location] & 0xFFFFF000);
         unsigned long physical_frame_number = process_mem_pool->get_frames(1);
         existing_page_table[page_location] = (unsigned long)(physical_frame_number * PAGE_SIZE) | 0x3;
         Console::puts("(unsigned long)existing_page_table ");
         Console::putui((unsigned long)existing_page_table);
         Console::puts("\n");
         Console::puts("existing_page_table[page_location] ");
         Console::putui(existing_page_table[page_location]);
         Console::puts("\n");
      }
  
      Console::puts("resolved page fault\n");
   }
   else{
      Console::puts("Something went wrong\n");
      assert(false);
   }
      Console::puts("handled page fault\n");

}

