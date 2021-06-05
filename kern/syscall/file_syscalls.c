#include <types.h>
#include <kern/unistd.h>
#include <stdarg.h>
#include <lib.h>
#include <syscall.h>
#include <limits.h>
#include <vfs.h>
#include <uio.h>
#include <vnode.h>
#include <synch.h>
#include <copyinout.h>

#if OPT_LAB5
struct vnode* filetable[OPEN_MAX];
#endif

//test with conman(requires read,write, exit)
ssize_t sys_read(int filehandle, char* buf, size_t size){
	(void)filehandle;
	//(void)buf;
	//(void)size;
	#if OPT_LAB5
	int result;
	struct iovec iov;
	struct uio ku;
	char kbuf[(int)size];
	#endif

int i;//uso improprio, sarebbe meglio un cast a int di size
	
	//kgets(buf,size);
	if ( filehandle == STDIN_FILENO){
	for (i=0; i<(int)size; i++)
	{
		//kprintf("inside loop\n");
		//non funziona al momento, probabilmente perchè manca sincronizzazione
		buf[i] = getch();	
	}
	
	if (i>0 && i<=(int)size)	
	return (ssize_t)i;
	
	else	return (ssize_t)-1;
	}
	#if OPT_LAB5
	else{
	//read from file
	uio_kinit(&iov, &ku, kbuf, size, filetable[filehandle]->offset, UIO_READ);
//	lock_acquire(filetable[filehandle]->v_lock);
	result = VOP_READ(filetable[filehandle], &ku);
//	lock_release(filetable[filehandle]->v_lock);
	copyout(kbuf,(userptr_t)buf,size);
	filetable[filehandle]->offset =+ (int)size; 
	return (ssize_t) result;	
	}
	#endif
		
	//not sure but code must not reach this point, just to avoid compiler error
	return -1;
	
}
ssize_t sys_write(int filehandle, char* buf, size_t size){

        int i;
    	
        (void)filehandle;
    	
	#if OPT_LAB5
	int result;
	struct iovec iov;
	struct uio ku;
	#endif
	
	if ( filehandle == STDOUT_FILENO || filehandle == STDERR_FILENO){
        for (i=0; i<(int)size; i++) {
            putch(buf[i]);
        }
	
	if (i>0 && i<= (int)size)	
	return (ssize_t)i;
	else 
		return (ssize_t)-1;
	}
	#if OPT_LAB5
	else{
	//write to file
//	lock_acquire(filetable[filehandle]->v_lock);
	uio_kinit(&iov, &ku, buf, size, filetable[filehandle]->offset, UIO_WRITE);
//	lock_release(filetable[filehandle]->v_lock);
	result = VOP_WRITE(filetable[filehandle], &ku);
	filetable[filehandle]->offset =+ (int)size; 
	return (ssize_t) result;	
	}
	#endif
	
	//not sure but code must not reach this point, just to avoid compiler error
	return -1;

    //manca check sul fatto che file handle debba essere stdout
}

#if OPT_LAB5
//1.note that in this implementation same files are opened in a system wide table in different entries
//so processes reference global file descriptors (no isolation). Entries of the table can reference
//same vnode, and each call to vfs_open and vfs_close increment the vn_refcounter of the vnode
//2. are vfs_open and vfs_close atomical operations?
int sys_open(userptr_t filename, int flags, mode_t mode){
 int i;
 int fd;	
 int result;
 //look for the first free file descriptor
 for (i=3; i< OPEN_MAX +1; i++){
	if (filetable[i]==NULL){
	 fd=i; 	
	 break;
	  }		
	}
 (void)mode;
 //use result then for error handling! In further implementation
 result = vfs_open((char*)filename, flags, 0, &filetable[fd]);
 (void) result;
 filetable[fd]->offset=0;
 //filetable[fd]->v_lock = lock_create(filename);
 //spinlock_acquire(&filetable[fd]->vn_countlock);

 //filetable[fd]->vn_refcount++; NOT NEEDED vfs_open updates the refcount for the vnode!!!

 //spinlock_release(&filetable[fd]->vn_countlock);
 return fd;
}

int sys_close(int fd){
 //spinlock_acquire(&filetable[fd]->vn_countlock);

     vfs_close(filetable[fd]);
     filetable[fd]= NULL;

  //spinlock_release(&filetable[fd]->vn_countlock);
 //error condition?in vfs.h says it does not fail, so 0 always returned
 return 0;
}
#endif
