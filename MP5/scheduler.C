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
  assert(false);
}
