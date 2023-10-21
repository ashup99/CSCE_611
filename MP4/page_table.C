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

// Static members for PageTable class
PageTable *PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool *PageTable::kernel_mem_pool = NULL;
ContFramePool *PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;
VMPool *PageTable::vm_pool_head = NULL;
VMPool *PageTable::vm_pool_current = NULL;

void PageTable::init_paging(ContFramePool *_kernel_mem_pool,
                            ContFramePool *_process_mem_pool,
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
    // Constructor for PageTable class
    Console::puts("Constructed Page Table object Start\n");
    unsigned long page_directory_frame_number = kernel_mem_pool->get_frames(1);
    page_directory = (unsigned long *)(page_directory_frame_number * PAGE_SIZE);
    // last pde as valid and pointing to first pde for recurive table look up
    page_directory[ENTRIES_PER_PAGE - 1] = (((unsigned long)page_directory) | 0x3);

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
    page_directory[0] = (((unsigned long)page_table) | 0x3);
    // page_directory[ENTRIES_PER_PAGE - 1] = (((unsigned long)page_directory) | 0x3);

    for (unsigned int i = 1; i < ENTRIES_PER_PAGE - 1; i++)
    {
        // attribute set to: supervisor level,
        // read/write, not present(010 in binary)
        page_directory[i] = 0 | 0x2;
    }

    vm_pool_head = NULL;
    vm_pool_current = NULL;
    Console::puts("Constructed Page Table object End\n");
}

void PageTable::load()
{
    // Load the page table into the CR3 register
    Console::puts("Loaded page table Start\n");
    current_page_table = this;
    write_cr3((unsigned long)current_page_table->page_directory);
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

void PageTable::handle_fault(REGS *_r)
{
    // Handle page faults
    Console::puts("handle_fault Start\n");
    unsigned long err_code = _r->err_code;
    if ((err_code & 0x1) == 0x0)
    {
        Console::puts("handle_fault err_occuured\n");
        unsigned long faulty_address = (unsigned long)(read_cr2());
        unsigned long *page_directory_list = (unsigned long *)(read_cr3());
        unsigned long directory_location = (faulty_address) >> 22;
        bool is_legitimate_vm_address = false;
        VMPool *iterartor;
        for (iterartor = vm_pool_head; iterartor != NULL; iterartor = iterartor->next)
        {
            bool temp_bool = iterartor->is_legitimate(faulty_address) == true;
            Console::puti(temp_bool);
            if (temp_bool)
            {
                is_legitimate_vm_address = true;
                break;
            }
        }
        if (!is_legitimate_vm_address && iterartor != NULL)
        {
            Console::puts("Not Legitimate address\n");
            assert(false);
        }
        if ((page_directory_list[directory_location] & 0x1) == 0x0)
        {
            Console::puts("directory issue and new page table");
            Console::puts("\n");
            unsigned long new_page_table_frame_number = process_mem_pool->get_frames(1);
            unsigned long *new_page_table = (unsigned long *)((new_page_table_frame_number * PAGE_SIZE));

            unsigned long *page_directory_entry_addr = (unsigned long *)(0xFFFFF << 12);
            // attribute set to: supervisor level,
            // read/write, not present(010 in binary)
            page_directory_entry_addr[directory_location] = (unsigned long)new_page_table | 0x3;

            // initializing the page table enteries
            for (unsigned int i = 0; i < ENTRIES_PER_PAGE; i++)
            {
                // attribute set to: supervisor level,
                // read/write, not present(010 in binary)
                new_page_table[i] = 0 | 0x2;
            }
        }
        else
        {
            Console::puts("existing page table issue");
            Console::puts("\n");
            unsigned long *existing_page_table = (unsigned long *)((0x3FF << 22) | (directory_location << 12));
            unsigned long physical_frame_number = process_mem_pool->get_frames(1);
            unsigned long page_table_entry_location = (faulty_address & (0x03FF << 12)) >> 12;
            existing_page_table[page_table_entry_location] = (unsigned long)(physical_frame_number * PAGE_SIZE) | 0x3;
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

void PageTable::register_pool(VMPool *_vm_pool)
{
    // Register a VMPool instance
    Console::puts("registered VM pool - start\n");
    if (vm_pool_head == NULL)
    {
        Console::puts("Empty Head.\n");
        // if list is empty assign it to head
        vm_pool_head = _vm_pool;
    }
    else
    {
        Console::puts("Non Empty Head.\n");
        // if list is not empty make next point to it
        vm_pool_current->next = _vm_pool;
    }
    // in both cases current next should be null and curent should be point to cureent _vm_pool
    vm_pool_current = _vm_pool;
    vm_pool_current->next = NULL;
    Console::puts("registered VM pool - end\n");
}

void PageTable::free_page(unsigned long _page_no)
{
    // Free a page and flush the TLB
    Console::puts("freed page - start\n");
    unsigned long page_directory_location = PDE_address(_page_no);
    unsigned long page_table_location = PTE_address(_page_no);
    unsigned long *page_directory_entry = (unsigned long *)((0x000003FF << 22) | (page_directory_location * PAGE_SIZE));
    unsigned long frame_no = (page_directory_entry[page_table_location] & 0xFFFFF000) / PAGE_SIZE;
    process_mem_pool->release_frames(frame_no);
    page_directory_entry[page_table_location] = page_directory_entry[page_table_location] | 2;
    // Flushing the TLB
    load();
    Console::puts("freed page - end\n");
}

// Return the address of the Page Directory Entry (PDE) Location
unsigned long PageTable::PDE_address(unsigned long addr)
{
    unsigned long page_directory_location = (addr & 0xFFC00000) >> 22;
    return page_directory_location;
}

// Return the address of the Page Table Entry (PTE) Location
unsigned long PageTable::PTE_address(unsigned long addr)
{
    unsigned long page_table_location = (addr & 0x003FF000) >> 12;
    return page_table_location;
}
