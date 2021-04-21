#include <types.h>
#include <kern/unistd.h>
#include <stdarg.h>
#include <lib.h>
#include "syscall.h"

ssize_t sys_read(int filehandle, char* buf, size_t size){
	(void)filehandle;
	(void)buf;
	(void)size;
	return size;
}
ssize_t sys_write(int filehandle, char* buf, size_t size){

        size_t i;
    
        (void)filehandle;
    
        for (i=0; i<size; i++) {
            putch(buf[i]);
        }
	
	if (i>0 && i<=size)	
	return i;
	else 
		return -1;

    //manca check sul fatto che file handle debba essere stdout
}
