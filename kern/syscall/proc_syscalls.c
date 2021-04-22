/*
 * AUthor: G.Cabodi
 * Very simple implementation of sys__exit.
 * It just avoids crash/panic. Full process exit still TODO
 * Address space is released
 */

#include <types.h>
#include <kern/unistd.h>
#include <clock.h>
#include <copyinout.h>
#include <syscall.h>
#include <lib.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>

#if OPT_LAB2
#include <current.h>
#endif

/*
 * simple proc management system calls
 */
void
sys__exit(int status)
{
  /* get address space of current process and destroy */
  struct addrspace *as = proc_getas();
  as_destroy(as);
  

  //(void) status; // TODO: status handling 
  #if OPT_LAB2
  struct thread *t = curthread;
  t->t_retcode = status;
  //still to be handled
  #endif
  /* thread exits. proc data structure will be lost */
  thread_exit();

  panic("thread_exit returned (should not happen)\n"); 

}
