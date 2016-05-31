/* Ben Haase
 * CS 4760
 * Assignment 3
 *$Author: o-haase $
 *$Date: 2016/03/10 04:54:38 $
 *$Log: slave.c,v $
 *Revision 1.4  2016/03/10 04:54:38  o-haase
 *Clean up and comments
 *
 *Revision 1.3  2016/03/09 23:40:49  o-haase
 *Minor changes to work with enter monitor function
 *
 *Revision 1.2  2016/03/07 03:24:33  o-haase
 *added use of semaphores and monitor to critical section function
 *
 *Revision 1.1  2016/03/05 03:10:53  o-haase
 *Initial revision
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

void critical_section();
void pmsg(char*, char*);
void sigtimeout_handler(int);
void sigctlc_handler(int);

//globals for use in signal handlers
char * paddr; //memory address returned from shmat
int pn; //integer representation of process number (from master)
FILE * f;
char * pnc; //character representation of process number (from master)
sst * ss;  //format and reference shared memory struct

int main(int argc, char* argv[]){
	pn = atoi(argv[1]);
	pnc = argv[1];
	srand((unsigned)(time(NULL) + pn)); //initialize srand

	//mount shared memory
	int shmid = shmget(SHMKEY, BUFF_SZ, 0777);
	if(shmid == -1){
		perror("Slave: Error in shmget.");
		exit(1);
	}
	paddr = (char*)(shmat(shmid, 0, 0));
	ss= (sst*)(paddr);

	//setup signal handlers (ignore SIGINT that will be intercepted by master)
	signal(SIGQUIT, sigtimeout_handler);
	signal(SIGTERM, sigctlc_handler);
	signal(SIGINT, SIG_IGN);

	//request access to critical section
	int i;
	for(i = 0; i < 3; i++){
		enter_monitor(critical_section, pmsg, pnc);
		sleep((rand() % 3));
	}
	
	//dismount memory and print finished message
	shmdt(paddr);
	pmsg(pnc, "done and detached");
	return 0;
}

//function for critical section, open file, sleep, write to file, close file
void critical_section(){
	pmsg(pnc, "is in CS");
	f = fopen("cstest", "a");
	time_t x;
	sleep((rand() % 3));
	time(&x);
	char * t = ctime(&x);

	if(f != NULL){
		fprintf(f, "%s %02s at time %.8s.\n", "File modified by process number", pnc, (t + 11));
	}

	fclose(f);
	f = NULL;
	pmsg(pnc, "out of CS");
}

//function to print process messages with time
void pmsg(char *p, char *msg){
	time_t tm;
	char * tms;
	time(&tm);
	tms = ctime(&tm);
	fprintf(stderr, "Process: %02s %s at %.8s.\n", p, msg, (tms + 11));
}

//signal handler for SIGQUIT (sent from master on timeout)
void sigtimeout_handler(int s){
	if(f != NULL){
		fclose(f);
		f = NULL;
	}
	shmdt(paddr);
	fprintf(stderr, "Slave process: %02s dying due to timeout \n", pnc);
	exit(1);
}

//signal handle for SIGTERM (sent from master on ^C)
void sigctlc_handler(int s){
	if(f != NULL){
		fclose(f);
		f = NULL;
	}
	shmdt(paddr);
	fprintf(stderr, "Slave process: %02s dying due to interrupt \n", pnc);
	exit(1);
}
