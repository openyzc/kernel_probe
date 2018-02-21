
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    /*Pointer to shared memory region*/    
    int *addr;   

    addr = mmap(NULL, sizeof(int),
	PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) {
        fprintf(stderr, "mmap() failed\n");
        exit(EXIT_FAILURE);
    }

    *addr = 1;      /*Initialize integer in mapped region*/    

    switch(fork()) {        /*Parent and child share mapping*/     
    case -1:    
        fprintf(stderr, "fork() failed\n");
        exit(EXIT_FAILURE);    

    case 0:         /*Child: increment shared integer and exit*/     
        printf("Child started, value = %d\n", *addr);    
        (*addr)++;    

        if (munmap(addr, sizeof(int)) == -1) {    
            fprintf(stderr, "munmap()() failed\n");    
            exit(EXIT_FAILURE);    
        }     
        exit(EXIT_SUCCESS);     

    default:        /*Parent: wait for child to terminate*/      
        if (wait(NULL) == -1) {    
            fprintf(stderr, "wait() failed\n");    
            exit(EXIT_FAILURE);      
        }     

        printf("In parent, value = %d\n", *addr);         
        if (munmap(addr, sizeof(int)) == -1) {       
            fprintf(stderr, "munmap()() failed\n");      
            exit(EXIT_FAILURE);       
        }        
        exit(EXIT_SUCCESS);
	}
}
