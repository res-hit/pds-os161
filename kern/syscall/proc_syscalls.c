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
#if OPT_LAB4
#include <synch.h>
#endif
/*
 * simple proc management system calls
 */
void
sys__exit(int status)
{
  /* get address space of current process and destroy */
  /*	from lab4 not needed anymore, since all process structure released 
	from kernel waiting for process to finish
  struct addrspace *as = proc_getas();
  as_destroy(as);
	*/

  //(void) status; // TODO: status handling 
  #if OPT_LAB2
  struct thread *t = curthread;
  #if OPT_LAB4
	//!!!!!!!!!!!!!!!!!!!!!!////
 	 struct proc* p = t->t_proc; //store struct process address since from curthread we won't be able to acces anymore!
  //t->t_retcode = status;
  t->t_proc->p_status = status;
  #endif	
  //still to be handled
  #endif
  #if OPT_LAB4
  proc_remthread(t);
  V(p->p_sem);
  #endif
  /* thread exits. proc data structure will be lost */
  thread_exit();

  panic("thread_exit returned (should not happen)\n"); 

}
