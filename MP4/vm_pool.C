/*
 File: vm_pool.C
 
 Author:
 Date  :
 
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

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    // assert(false);
    Console::puts("Constructed VMPool object - start.\n");
    base_address=_base_address;
    size=_size;
    frame_pool=_frame_pool;
    page_table=_page_table;
    
    next = NULL;
    available_size=_size;
    total_count=0;


    // using the first pool to store region data
    region_data *temp_region = (region_data*) base_address;
    temp_region[0].base_addr = base_address;
    temp_region[0].size = PAGE_SIZE;
    last_region_end_address = base_address + PAGE_SIZE;
    regions=temp_region;
    available_size -= PAGE_SIZE;
    total_count+=1;

    page_table->register_pool(this);

    Console::puts("Constructed VMPool object - end.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    // assert(false);
    Console::puts("Allocated region of memory - start. \n");
    if(available_size<_size){
        Console::puts("No free size available.");
        assert(false);
    }
    unsigned number_of_pages = _size/PAGE_SIZE;
    number_of_pages = _size%PAGE_SIZE>0?number_of_pages+1:number_of_pages
    regions[total_count].base_addr=last_region_end_address;
    regions[total_count].size= number_of_pages * PAGE_SIZE;
    total_count+=1;
    available_size-=number_of_pages * PAGE_SIZE;
    last_region_end_address+=number_of_pages * PAGE_SIZE;
    Console::puts("Allocated region of memory - end.\n");
    return regions[total_count-1].base_addr;
}

void VMPool::release(unsigned long _start_address) {
    // assert(false);
    Console::puts("Released region of memory - start.\n");
    unsigned long region_relase_index=-1;
    for(unsigned long i =0 ;i<total_count;i++){
        if(regions[i].base_addr==_start_address){
            region_relase_index=i;
            break;
        }
    }
    if(region_relase_index<0){
        Console::puts("No such region found starting with this start address");
        assert(false);
    }
    else{
        unsigned long curr_addr= _start_address;
        unsigned long number_of_pages=regions[region_relase_index].size/PAGE_SIZE;
        for(unsigned long i =0;i<number_of_pages;i++){
            page_table->free_page(curr_addr);
            curr_addr+=PAGE_SIZE;
        }

        for(unsigned long i =region_relase_index;i<total_count-1;i++){
            regions[i]=regions[i+1];
        }

        total_count-=1;
        available_size+=regions[total_count].size;

        if(regions[region_relase_index].base_addr+regions[region_relase_index].size==last_region_end_address){
            last_region_end_address =regions[region_relase_index].base_addr;
        }
        page_table->load();
    }
    page_table->load();
    Console::puts("Released region of memory - end.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    // assert(false);
    Console::puts("Checked whether address is part of an allocated region - start.\n");
    for(unsigned long i =0; i<total_count;i++){
        if(_address>=regions[i].base_addr && _address<=regions[i].base_addr+regions[i].size){
            Console::puts("Legitimate");
            return true;
        }
    }
    Console::puts("Not Legitimate");
    return false;
    Console::puts("Checked whether address is part of an allocated region - end.\n");
}

