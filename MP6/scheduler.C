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

extern MemPool *MEMORY_POOL;

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler()
{
  Console::puts("Constructed Scheduler::Scheduler() - start.\n");
  head = NULL;
  current = NULL;
  Console::puts("Constructed Scheduler::Scheduler() - end.\n");
}

void Scheduler::yield()
{
  // assert(false);
  if (Machine::interrupts_enabled())
  {
    Console::puts("Interrupts Disabled.\n");
    Machine::disable_interrupts();
  }
  Console::puts("Scheduler::yield() - start.\n");
  Thread_List *temp = head;
  if (head == NULL)
  {
    Console::puts("Empty Ready Queue\n");
    assert(false);
  }
  if (head->next == NULL)
  {
    Console::puts("Before last Thread\n");
  }
  head = head->next;
  head->prev = NULL;
  Console::puts("Thread Dispatched to : ");
  Console::puti(temp->thread->ThreadId() + 1);
  Console::puts("\n");
  Thread::dispatch_to(temp->thread);
  MEMORY_POOL->release((unsigned long)temp);
  Console::puts("Scheduler::yield() - end.\n");
  if (!Machine::interrupts_enabled())
  {
    Console::puts("Interrupts Enabled.\n");
    Machine::enable_interrupts();
  }
}

void Scheduler::add(Thread *_thread)
{
  // assert(false);
  if (Machine::interrupts_enabled())
  {
    Console::puts("Interrupts Disabled.\n");
    Machine::disable_interrupts();
  }
  Console::puts("Scheduler::add() - start.\n");
  Thread_List *new_thread = (Thread_List *)(MEMORY_POOL->allocate(sizeof(Thread_List)));
  new_thread->thread = _thread;
  new_thread->next = NULL;
  new_thread->prev = NULL;
  if (head == NULL && current == NULL)
  {
    head = new_thread;
    current = new_thread;
  }
  else
  {
    current->next = new_thread;
    new_thread->prev = current;
    current = new_thread;
  }
  Console::puts("Thread Added : ");
  Console::puti(_thread->ThreadId() + 1);
  Console::puts("\n");
  Console::puts("Scheduler::add() - end.\n");
  if (!Machine::interrupts_enabled())
  {
    Console::puts("Interrupts Enabled.\n");
    Machine::enable_interrupts();
  }
}

void Scheduler::resume(Thread *_thread)
{
  // assert(false);
  Console::puts("Scheduler::resume() - start.\n");
  add(_thread);
  Console::puts("Thread Resume:");
  Console::putui(_thread->ThreadId() + 1);
  Console::puts("\n");
  Console::puts("Scheduler::resume() - end.\n");
}

void Scheduler::terminate(Thread *_thread)
{
  // assert(false);
  if (Machine::interrupts_enabled())
  {
    Console::puts("Interrupts Disabled.\n");
    Machine::disable_interrupts();
  }
  Console::puts("Scheduler::terminate() - start.\n");
  if (Thread::CurrentThread() == _thread)
  {
    yield();
  }
  else if (head->thread == _thread)
  {
    head = head->next;
    head->prev = NULL;
  }
  else
  {
    Thread_List *itr = head;
    for (itr = head; itr->next->thread != _thread; itr = itr->next)
    {
    }

    if (itr != NULL)
    {

      Thread_List *n_temp = itr->next;
      Thread_List *p_temp = itr->prev;
      itr->next = n_temp->next;
      itr->prev = p_temp->prev;

      MEMORY_POOL->release((unsigned long)n_temp);
      MEMORY_POOL->release((unsigned long)p_temp);
    }
  }
  Console::puts("Thread Terminated : ");
  Console::puti(_thread->ThreadId() + 1);
  Console::puts("\n");
  Console::puts("Scheduler::terminate() - end.\n");
  if (!Machine::interrupts_enabled())
  {
    Console::puts("Interrupts Enabled.\n");
    Machine::enable_interrupts();
  }
}
