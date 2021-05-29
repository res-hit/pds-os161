/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Synchronization primitives.
 * The specifications of the functions are in synch.h.
 */

#include <types.h>
#include <lib.h>
#include <spinlock.h>
#include <wchan.h>
#include <thread.h>
#include <current.h>
#include <synch.h>

////////////////////////////////////////////////////////////
//
// Semaphore.

struct semaphore *
sem_create(const char *name, unsigned initial_count)
{
        struct semaphore *sem;

        sem = kmalloc(sizeof(*sem));
        if (sem == NULL) {
                return NULL;
        }

        sem->sem_name = kstrdup(name);
        if (sem->sem_name == NULL) {
                kfree(sem);
                return NULL;
        }

	sem->sem_wchan = wchan_create(sem->sem_name);
	if (sem->sem_wchan == NULL) {
		kfree(sem->sem_name);
		kfree(sem);
		return NULL;
	}

	spinlock_init(&sem->sem_lock);
        sem->sem_count = initial_count;

        return sem;
}

void
sem_destroy(struct semaphore *sem)
{
        KASSERT(sem != NULL);

	/* wchan_cleanup will assert if anyone's waiting on it */
	spinlock_cleanup(&sem->sem_lock);
	wchan_destroy(sem->sem_wchan);
        kfree(sem->sem_name);
        kfree(sem);
}

void
P(struct semaphore *sem)
{
        KASSERT(sem != NULL);

        /*
         * May not block in an interrupt handler.
         *
         * For robustness, always check, even if we can actually
         * complete the P without blocking.
         */
        KASSERT(curthread->t_in_interrupt == false);

	/* Use the semaphore spinlock to protect the wchan as well. */
	spinlock_acquire(&sem->sem_lock);
        while (sem->sem_count == 0) {
		/*
		 *
		 * Note that we don't maintain strict FIFO ordering of
		 * threads going through the semaphore; that is, we
		 * might "get" it on the first try even if other
		 * threads are waiting. Apparently according to some
		 * textbooks semaphores must for some reason have
		 * strict ordering. Too bad. :-)
		 *
		 * Exercise: how would you implement strict FIFO
		 * ordering?
		 */
		wchan_sleep(sem->sem_wchan, &sem->sem_lock);
        }
        KASSERT(sem->sem_count > 0);
        sem->sem_count--;
	spinlock_release(&sem->sem_lock);
}

void
V(struct semaphore *sem)
{
        KASSERT(sem != NULL);

	spinlock_acquire(&sem->sem_lock);

        sem->sem_count++;
        KASSERT(sem->sem_count > 0);
	wchan_wakeone(sem->sem_wchan, &sem->sem_lock);

	spinlock_release(&sem->sem_lock);
}

////////////////////////////////////////////////////////////
//
// Lock.

struct lock *
lock_create(const char *name)
{
        struct lock *lock;

        lock = kmalloc(sizeof(*lock));
        if (lock == NULL) {
                return NULL;
        }

        lock->lk_name = kstrdup(name);
        if (lock->lk_name == NULL) {
                kfree(lock);
                return NULL;
        }

        // add stuff here as needed

#if OPT_LAB3
	lock->lk_wchan = wchan_create(lock->lk_name);
	if (lock->lk_wchan == NULL) {
	  kfree(lock->lk_name);
	  kfree(lock);
	  return NULL;
	}
	lock->lk_owner = NULL;
	spinlock_init(&lock->lk_lock);
#endif	
        return lock;
}

void
lock_destroy(struct lock *lock)
{
        KASSERT(lock != NULL);

        // add stuff here as needed
#if OPT_LAB3
	spinlock_cleanup(&lock->lk_lock);
	wchan_destroy(lock->lk_wchan);
#endif
        kfree(lock->lk_name);
        kfree(lock);
}

void
lock_acquire(struct lock *lock)
{
        // Write this
#if OPT_LAB3
        KASSERT(lock != NULL);
	if (lock_do_i_hold(lock)) {
	  kprintf("AAACKK!\n");
	}
	KASSERT(!(lock_do_i_hold(lock)));

        KASSERT(curthread->t_in_interrupt == false);

	spinlock_acquire(&lock->lk_lock);        
	while (lock->lk_owner != NULL) {
	  wchan_sleep(lock->lk_wchan, &lock->lk_lock);
        }
        KASSERT(lock->lk_owner == NULL);
        lock->lk_owner=curthread;
	spinlock_release(&lock->lk_lock);
#endif
        (void)lock;  // suppress warning until code gets written
}

void
lock_release(struct lock *lock)
{
        // Write this
#if OPT_LAB3
	KASSERT(lock != NULL);
	KASSERT(lock_do_i_hold(lock));
	spinlock_acquire(&lock->lk_lock);
        lock->lk_owner=NULL;
	/*  G.Cabodi - 2019: no problem here owning a spinlock, as V/wchan_wakeone 
	    do not lead to wait state */
        wchan_wakeone(lock->lk_wchan, &lock->lk_lock);
	spinlock_release(&lock->lk_lock);
#endif

        (void)lock;  // suppress warning until code gets written
}

bool
lock_do_i_hold(struct lock *lock)
{
        // Write this
#if OPT_LAB3
        bool res;
	/*  this could possibly work without spinlock for mutual 
	    exclusion, which could simplify the semaphore-based solution, by 
	    removing the spinlock. 
	    Whenever the current thread owns the lock, the test is safe without 
	    guaranteeing mutual exclusion. 
	    If NOT the owner, a wrong verdict could happen (very low chance!!!)
            by wrongly reading a pointer == curthread. However, using the spinlock 
	    is good practice for shared data. */
	spinlock_acquire(&lock->lk_lock);
	res = lock->lk_owner == curthread;
	spinlock_release(&lock->lk_lock);
	return res;
#endif

        (void)lock;  // suppress warning until code gets written

        return true; // dummy until code gets written
}

////////////////////////////////////////////////////////////
//
// CV


struct cv *
cv_create(const char *name)
{
        struct cv *cv;

        cv = kmalloc(sizeof(*cv));
        if (cv == NULL) {
                return NULL;
        }

        cv->cv_name = kstrdup(name);
        if (cv->cv_name==NULL) {
                kfree(cv);
                return NULL;
        }

        // add stuff here as needed
#if OPT_LAB3
	cv->cv_wchan = wchan_create(cv->cv_name);
	if (cv->cv_wchan == NULL) {
	        kfree(cv->cv_name);
		kfree(cv);
		return NULL;
	}	
	spinlock_init(&cv->cv_lock);	
#endif
        return cv;
}

void
cv_destroy(struct cv *cv)
{
        KASSERT(cv != NULL);

        // add stuff here as needed
#if OPT_LAB3
	wchan_destroy(cv->cv_wchan);
	spinlock_cleanup(&cv->cv_lock);
#endif
        kfree(cv->cv_name);
        kfree(cv);
}

//wait operation in condition varialbe is a two layered access
//first: release(safely) the lock which can be used from other threads
//second: thread sleeps until an event is signaled, goes to a queue of sleeping thread and release the lock owned in that time
void
cv_wait(struct cv *cv, struct lock *lock)
{
        // Write this
#if OPT_LAB3
	//add a spinlock in order to guarantee mutual exclusion for lock release since the second is a pointer
	//(or is it just to make wchan work?)--> both!!!
	/* G.Cabodi - 2019: spinlock already owned as ***atomic lock_release+wchan_sleep
	   needed*****<---r: imagine to release the lock and another thread aquires it calls wait and reaches
	wchan before this thread */
	spinlock_acquire(&cv->cv_lock);        
	lock_release(lock);
	//thread will be sleeping, signaled at proper time. note: it needs a spinlock to work properly
	//which is the one defined in order to safely release the *actual* lock	
	wchan_sleep(cv->cv_wchan, &cv->cv_lock);
	spinlock_release(&cv->cv_lock);
	
	//r:when thread is again active acquire the *actual* lock

	/* G.Cabodi - 2019: spinlock already  released to avoid ownership while
	   (possibly) going to wait state in lock_acquire. 
	   ******Atomicity wakeup+lock_acquire not guaranteed****** (but not necessary!) */
	lock_acquire(lock);
#endif

        (void)cv;    // suppress warning until code gets written
        (void)lock;  // suppress warning until code gets written
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
        // Write this
#if OPT_LAB3
	//spinlock can be acquired since wchan is sleeping, but has released inside the wchan the spinlock-> no deadlock can 		//occur
	
	/* g.Cabodi - 2019: here the spinlock is NOT required, as no atomic operation 
	   has to be done. *****The spinlock is just acquired because needed by wakeone**** */
	spinlock_acquire(&cv->cv_lock);
	//wake up one thread sleeping which had the spinlock we are using in the wchan
	wchan_wakeone(cv->cv_wchan, &cv->cv_lock);
	spinlock_release(&cv->cv_lock);

	//lock_release(lock); stupid idea: thread which calls cv_signal acquires the lock before calling, and releases externally
	//after calling it. Struct lock is only formally there(r: is it a Linux convention?)
#endif
	(void)cv;    // suppress warning until code gets written
	(void)lock;  // suppress warning until code gets written
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	// Write this
	
#if OPT_LAB3
        spinlock_acquire(&cv->cv_lock);
	//wake up one thread sleeping which had the spinlock we are using in the wchan
	wchan_wakeall(cv->cv_wchan, &cv->cv_lock);
	spinlock_release(&cv->cv_lock);	
#endif
	(void)cv;    // suppress warning until code gets written
	(void)lock;  // suppress warning until code gets written
}
