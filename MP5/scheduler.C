/*
 File: scheduler.C
 
 Author: Ashutosh Punyani
 Date  : Nov 5, 2023
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "mem_pool.H"

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

extern MemPool* MEMORY_POOL;

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
  // assert(false);
  head=NULL;
  current=NULL;
  Console::puts("Constructed Scheduler::Scheduler().\n");
}

void Scheduler::yield() {
  // assert(false);
  Console::puts("Scheduler::yield().\n");
}

void Scheduler::resume(Thread * _thread) {
  // assert(false);
  Console::puts("Scheduler::resume().\n");
}

void Scheduler::add(Thread * _thread) {
  // assert(false);
  Console::puts("Scheduler::add().\n");
}

void Scheduler::terminate(Thread * _thread) {
  // assert(false);
  Console::puts("Scheduler::terminate().\n");
}

FIFOScheduler::FIFOScheduler() {
  // assert(false);
  head=NULL;
  current=NULL;
  Console::puts("Constructed FIFOScheduler::FIFOScheduler().\n");
}

void FIFOScqheduler::yield() {
  // assert(false);
  Console::puts("FIFOScheduler::yield() - start.\n");
  Thread_List* temp=head;
  if(head==NULL){
    Console::puts("Empty Ready Queue\n");
    assert(false);
  }
  if(head->next==NULL){
    Console::puts("Before last Thread\n");
  }
  head=head->next;
  // head->prev=NULL;
  Console::puts("Thread Dispatched to : ");
  Console::puti(temp->thread->ThreadId()+1);
  Console::puts("\n");
  Thread::dispatch_to(temp->thread);
  MEMORY_POOL->release((unsigned long)temp);
  Console::puts("FIFOScheduler::yield() - end.\n");
}

void FIFOScheduler::add(Thread * _thread) {
  // assert(false);
  Console::puts("FIFOScheduler::add() - start.\n");
  Thread_List* new_thread = (Thread_List*)(MEMORY_POOL->allocate(sizeof(Thread_List)));
  new_thread->thread=_thread;
  new_thread->next=NULL;
  new_thread->prev=NULL;
  if(head==NULL && current==NULL){
    head=new_thread;
    current=new_thread;
  }
  else{
    current->next=new_thread;
    new_thread->prev=current;
    current=new_thread;
  }
  Console::puts("Thread Added : ");
  Console::puti(_thread->ThreadId()+1);
  Console::puts("\n");
  Console::puts("FIFOScheduler::add() - end.\n");
}

