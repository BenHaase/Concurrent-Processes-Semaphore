/* Ben Haase
 * CS 4760
 * Assignment 3
 * $Author: o-haase $
 * $Date: 2016/03/10 04:55:27 $
 * $Log: monitor.h,v $
 * Revision 1.3  2016/03/10 04:55:27  o-haase
 * Clean up, comments
 *
 * Revision 1.2  2016/03/07 03:23:40  o-haase
 * function prototypes and struct
 *
 * Revision 1.1  2016/03/05 03:11:57  o-haase
 * Initial revision
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <pthread.h>

//key and size for shared memory
#define SHMKEY 56
#define BUFF_SZ sizeof(sst)

//"condition" struct to share semaphores and count among processes
typedef struct{
	int n_waiting_procs;
	sem_t sem;
	int next_count;
	sem_t next;
	sem_t mutex;
	}sst;

//function prototypes
	void enter_monitor(void (*critical_section)(void), void (*pmsg)(char*, char*), char* pm);
	void semwait();
	void semsignal();
	void sem_setup();
	void sem_cleanup();
