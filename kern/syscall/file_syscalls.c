#include <types.h>
#include <kern/unistd.h>
#include <stdarg.h>
#include <lib.h>
#include "syscall.h"

//test with conman(requires read,write, exit)
ssize_t sys_read(int filehandle, char* buf, size_t size){
	(void)filehandle;
	//(void)buf;
	//(void)size;

int i;//uso improprio, sarebbe meglio un cast a int di size
	
	//kgets(buf,size);
	for (i=0; i<(int)size; i++)
	{
		kprintf("inside loop\n");
		//non funziona al momento, probabilmente perchè manca sincronizzazione
		buf[i] = getch();	
	}
	
	if (i>0 && i<=(int)size)	
	return (ssize_t)i;
	
	else	return (ssize_t)-1;
}
ssize_t sys_write(int filehandle, char* buf, size_t size){

        int i;
    
        (void)filehandle;
    
        for (i=0; i<(int)size; i++) {
            putch(buf[i]);
        }
	
	if (i>0 && i<= (int)size)	
	return (ssize_t)i;
	else 
		return (ssize_t)-1;

    //manca check sul fatto che file handle debba essere stdout
}
