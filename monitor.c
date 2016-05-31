/* Ben Haase
 * CS 4760
 * Assignment 3
 * $Author: o-haase $
 * $Date: 2016/03/10 04:54:55 $
 * $Log: monitor.c,v $
 * Revision 1.4  2016/03/10 04:54:55  o-haase
 * Clean up, comments, added messages when waiting for and acquiring semaphores
 *
 * Revision 1.3  2016/03/09 23:40:11  o-haase
 * Semaphore implementation of mutual exclusion now working
 *
 * Revision 1.2  2016/03/07 03:23:14  o-haase
 * added implementation of monitor functions
 *
 * Revision 1.1  2016/03/07 03:18:32  o-haase
 * Initial revision
 *
 * Revision 1.1  2016/03/05 03:11:24  o-haase
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
#include "monitor.h"

	extern sst * ss;
	//function to guard access to critical section, implementation of a monitor
        void enter_monitor(void (*critical_section)(void), void (*pmsg)(char*, char*), char * pm){

		//control access to critical section function
		(*pmsg)(pm, "waiting for sem");
		ss->n_waiting_procs++;
		sem_wait(&(ss->sem));
		ss->n_waiting_procs--;
		(*pmsg)(pm, "acquired sem");
		ss->next_count++;
		(*pmsg)(pm, "waiting for mutex");
		sem_wait(&(ss->mutex));
		ss->next_count--;
		(*pmsg)(pm, "acquired mutex");
		(*critical_section)();
		sem_post(&(ss->sem));
		sem_post(&(ss->mutex));
	}

        void semwait(){
		ss->n_waiting_procs++;
		if(ss->next_count > 0){sem_post(&(ss->next));}
		else{sem_post(&(ss->mutex));}
		sem_wait(&(ss->sem));
		ss->n_waiting_procs--;
	}
	
        void semsignal(){
		if(ss->n_waiting_procs <= 0) return;
		ss->next_count++;
		sem_post(&(ss->sem));
		sem_wait(&(ss->next));
		ss->next_count--;
	}	

	//function to initialize semaphores in shared memory
	void sem_setup(){
		ss->n_waiting_procs = 0;
		if((sem_init(&(ss->sem), 1, 0)) < 0) perror("semaphore initialization error");
		if((sem_init(&(ss->mutex), 1, 1)) < 0) perror("semaphore initialization error");
		if((sem_init(&(ss->next), 1, 0)) < 0) perror("semaphore initialization error");
		ss->next_count = 0;

	}

	//function to destroy semaphores upon termination
	void sem_cleanup(){
		if((sem_destroy(&(ss->sem))) < 0) perror("semaphore destruction error");
		if((sem_destroy(&(ss->mutex))) < 0) perror("semaphore destruction error");
		if((sem_destroy(&(ss->next))) < 0) perror("semaphore destruction error");
	}
