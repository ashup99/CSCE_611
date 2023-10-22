/*
 File: vm_pool.C

 Author: Ashutosh Punyani
 Date  : October,20 2023

 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long _base_address,
               unsigned long _size,
               ContFramePool *_frame_pool,
               PageTable *_page_table)
{
    // Constructor for the VMPool class
    Console::puts("Constructed VMPool object - start.\n");
    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;

    next = NULL;
    available_size = _size;
    total_count = 0;

    page_table->register_pool(this);

    // Using the first pool to store region data
    region_data *temp_region = (region_data *)base_address;
    temp_region[0].base_addr = base_address;
    temp_region[0].size = PageTable::PAGE_SIZE;
    regions = temp_region;
    available_size -= PageTable::PAGE_SIZE;
    total_count += 1;

    Console::puts("Constructed VMPool object - end.\n");
}

unsigned long VMPool::allocate(unsigned long _size)
{
    // Allocate a region of memory
    Console::puts("Allocated region of memory - start. \n");
    if (available_size < _size)
    {
        Console::puts("No free size available.\n");
        assert(false);
    }
    unsigned number_of_pages = _size / PageTable::PAGE_SIZE;
    number_of_pages = _size % PageTable::PAGE_SIZE > 0 ? number_of_pages + 1 : number_of_pages;
    regions[total_count].base_addr = regions[total_count - 1].base_addr + regions[total_count - 1].size;
    regions[total_count].size = number_of_pages * PageTable::PAGE_SIZE;
    total_count += 1;
    available_size -= number_of_pages * PageTable::PAGE_SIZE;

    Console::puts("Allocated region of memory - end.\n");
    return regions[total_count - 1].base_addr;
}

void VMPool::release(unsigned long _start_address)
{
    // Release a region of memory
    Console::puts("Released region of memory - start.\n");
    if(!is_legitimate(_start_address)){
        Console::puts("Not Legitimate - start address");
        assert(false);
    }
    unsigned long region_relase_index = -1;
    for (unsigned long i = 1; i < total_count; i++)
    {
        if (regions[i].base_addr == _start_address)
        {
            region_relase_index = i;
            break;
        }
    }
    if (region_relase_index < 0)
    {
        Console::puts("No such region found starting with this start address");
        assert(false);
    }
    else
    {
        unsigned long curr_addr = _start_address;
        unsigned long number_of_pages = regions[region_relase_index].size / PageTable::PAGE_SIZE;
        for (unsigned long i = 0; i < number_of_pages; i++)
        {
            page_table->free_page(curr_addr);
            curr_addr += PageTable::PAGE_SIZE;
        }

        for (unsigned long i = region_relase_index; i < total_count; i++)
        {
            regions[i] = regions[i + 1];
        }

        total_count -= 1;
        available_size += regions[total_count].size;

        regions[region_relase_index].base_addr = regions[total_count].base_addr;
        regions[region_relase_index].size = regions[total_count].size;
    }
    // Flushing the TLB
    page_table->load();
    Console::puts("Released region of memory - end.\n");
}

bool VMPool::is_legitimate(unsigned long _address)
{
    // Checking whether the address is part of an allocated region
    Console::puts("Checked whether address is part of an allocated region - start.\n");
    for (unsigned long i = 0; i < total_count; i++)
    {
        if (regions[i].base_addr <= _address && regions[i].base_addr + regions[i].size >= _address)
        {
            Console::puts("Legitimate.\n");
            Console::puts("Checked whether address is part of an allocated region - end.\n");
            return true;
        }
    }
    Console::puts("Not Legitimate.\n");
    Console::puts("Checked whether address is part of an allocated region - end.\n");
    return false;
}
