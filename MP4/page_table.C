/*
    File: page_table.C

    Author: Ashutosh S Punyani
    Date  : September, 30 2023

    Description: Basic Paging.

*/

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
   unsigned long page_directory_frame_number = process_mem_pool->get_frames(1);
   page_directory = (unsigned long *)(page_directory_frame_number * PAGE_SIZE);

   unsigned long page_table_frame_number = process_mem_pool->get_frames(1);
   unsigned long *page_table = (unsigned long *)(page_table_frame_number * PAGE_SIZE);

   // mappng the first 4MB of memory
   for (unsigned long i = 0, physical_address = 0; i < ENTRIES_PER_PAGE; i++, physical_address += PAGE_SIZE)
   {
      // attribute set to: supervisor level,
      // read/write, present(011 in binary)
      page_table[i] = physical_address | 0x3;
   }

   // attribute set to: supervisor level,
   // read/write, present(011 in binary)
   // first pde as valid pointing to page table
   page_directory[0] = (unsigned long)page_table | 0x3;
   // last pde as valid and pointing to first pde for recurive table look up
   page_directory[ENTRIES_PER_PAGE - 1] = (unsigned long)page_directory | 0x3;

   for (unsigned int i = 1; i < ENTRIES_PER_PAGE - 1; i++)
   {
      // attribute set to: supervisor level,
      // read/write, not present(010 in binary)
      page_directory[i] = 0 | 0x2;
   }

   current_page_table = this;
   vm_pool_head = NULL;
   vm_pool_current = NULL;
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
    Console::puts("handle_fault Start\n");
   unsigned long err_code = _r->err_code;
   if ((err_code & 0x1) == 0x0)
   {
      Console::puts("handle_fault err_occuured\n");
      unsigned long faulty_address = (unsigned long)(read_cr2());
      unsigned long *page_directory_list = (unsigned long *)(read_cr3());
      unsigned long directory_location = (faulty_address & 0xFFC00000) >> 22;
      unsigned long page_location = (faulty_address & 0x003FF000) >> 12;
      bool page_table_fault = false;

      if ((page_directory_list[directory_location] & 0x1) == 0x0)
      {
         Console::puts("directory issue and new page table");
         Console::puts("\n");
         unsigned long new_page_table_frame_number = kernel_mem_pool->get_frames(1);
         unsigned long *new_page_table = (unsigned long *)((new_page_table_frame_number * PAGE_SIZE));

         // attribute set to: supervisor level,
         // read/write, not present(010 in binary)
         page_directory_list[directory_location] = (unsigned long)new_page_table | 0x3;

         // initializing the page table enteries
         for (unsigned int i = 0; i < ENTRIES_PER_PAGE; i++)
         {
            // attribute set to: supervisor level,
            // read/write, not present(010 in binary)
            new_page_table[i] = 0 | 0x2;
         }

         unsigned long physical_frame_number = process_mem_pool->get_frames(1);
         new_page_table[page_location] = (unsigned long)(physical_frame_number * PAGE_SIZE) | 0x3;
      }
      else
      {
         Console::puts("existing page table issue");
         Console::puts("\n");
         unsigned long *existing_page_table = (unsigned long *)(page_directory_list[directory_location] & 0xFFFFF000);
         unsigned long physical_frame_number = process_mem_pool->get_frames(1);
         existing_page_table[page_location] = (unsigned long)(physical_frame_number * PAGE_SIZE) | 0x3;
      }

      Console::puts("resolved page fault\n");
   }
   else
   {
      Console::puts("Something went wrong\n");
      assert(false);
   }
   Console::puts("handle_fault End\n");
}

void PageTable::register_pool(VMPool * _vm_pool)
{
    assert(false);
    Console::puts("registered VM pool - start\n");
    if(vm_pool_head==NULL){
        //if list is empty assign it to head
        vm_pool_head=_vm_pool;
        vm_pool_current= vm_pool_head;
    }
    else{
        // if list is not empty make next point to it
        vm_pool_current->next = _vm_pool;
        vm_pool_current=vm_pool_current->next;
    }
    // in both cases finally current next should be null
    vm_pool_current->next=NULL;
    Console::puts("registered VM pool - end\n");
}

void PageTable::free_page(unsigned long _page_no) {
    assert(false);
    Console::puts("freed page - start\n");
    unsigned long addr = (unsigned long)(_page_no * PAGE_SIZE);
    unsigned long* pte_address = (unsigned long *)PTE_address(addr);
    if(*pte_address & 0x1){
        unsigned long frame_no = *pte_address / PAGE_SIZE;
        process_mem_pool->release_frames(frame_no);
        *pte_address = 0 | 0x2;
        load();
    }
    Console::puts("freed page - end\n");
}


// return the address of the PDE
unsigned long * PageTable::PDE_address(unsigned long addr){
    unsigned long page_directory_location = (addr & 0xFFC00000) >> 22;
    unsigned long* pde_address = (page_directory_location<<2) | 0XFFFFF000;
    Console::puts("pde_address\n");
    Console::putui(pde_address);
    Console::puts("\n");
    return pde_address;
}

// return the address of the PTE
unsigned long * PageTable::PTE_address(unsigned long addr){
    unsigned long page_directory_location = (addr & 0xFFC00000) >> 22;
    unsigned long page_table_location = (addr & 0x003FF000) >> 12;
    unsigned long* pte_address = (page_table_location<<12) | (page_directory_location<<2) | 0XFFC00000;
    Console::puts("pte_address\n");
    Console::putui(pte_address);
    Console::puts("\n");
    return pte_address;
}
