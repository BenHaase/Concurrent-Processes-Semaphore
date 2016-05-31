/* Ben Haase
 * CS 4760
 * Assignment 3
 *$Author: o-haase $
 *$Date: 2016/03/10 04:54:11 $
 *$Log: master.c,v $
 *Revision 1.3  2016/03/10 04:54:11  o-haase
 *Clean up, changed timeout from 60 seconds to 5 min
 *
 *Revision 1.2  2016/03/07 03:22:49  o-haase
 *added use of semiphores
 *
 *Revision 1.1  2016/03/07 03:17:43  o-haase
 *Initial revision
 *
 *Revision 1.1  2016/03/05 03:10:02  o-haase
 *Initial revision
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <pthread.h>
#include "monitor.h"
	
void test(sst*, int);
void sigint_handler(int);
void timeout_handler(int);

sst * ss; //pointer to set up and access shared memory struct "condition"
int shmid; //memory var for shmat return val
pid_t slvs[19]; //hold slave process actual pid numbers

int main(int argc, char* argv[]){
	int t;
	int tkill = 300;
	//getopt for time limit if not specied time limit = 60 seconds
	if((t = getopt(argc, argv, "t::")) != -1){
		if(t == 't') tkill = atoi(argv[2]);
		else{
			perror("Error: Options character invalid");
			exit(0);
		}
	}

	srand(time(NULL)); //initialize srand

	//request and format shared memory
	shmid = shmget(SHMKEY, BUFF_SZ, 0777 | IPC_CREAT);
	if(shmid == -1){
		perror("Master: Error in shmget. \n");
		exit(1);
	}
	char * paddr = (char*)(shmat(shmid, 0, 0));
	ss = (sst*)(paddr);
	sem_setup();

	test(ss, tkill); //spawn processes

	sem_cleanup();
	shmctl(shmid, IPC_RMID, (struct shmid_ds*)NULL); //clean up memory

	return 0;
}

//function to spawn the process and run the program
void test(sst * m, int timekill){
	pid_t x;
	int status; //hold status of finished slave process
	int n = 19; //number of processes
	int i;
	int fail = 0; //incremented if failure, flag for termination
	char t[3]; //hold slave process number

	//fork and exec processes 1-19
	for(i = 0; i < 19; i++){
		if((slvs[i] = fork()) < 0){
			perror("Fork Failed \n");
			fail = 1;
			return;
		}

		if(slvs[i] == 0){
			sprintf(t, "%i", (i + 1));
			if((execl("./slave", "slave", t, NULL)) == -1){
				perror("Exec Failed");
				fail = 1;
				return;
			}
		}
	}

	//register signal handlers and set time limit
	signal(SIGINT, sigint_handler);
	signal(SIGALRM, timeout_handler);
	alarm(timekill);

	sem_post(&(ss->sem)); //set processes in motion

	if(fail > 0) alarm(0); //send signal to end processes if error occured

	//wait for the processes to finish
	while(n > 0){
		x = wait(&status);
		//fprintf(stderr, "Child PID: %ld is done, status: 0x%x \n", (long)x, status);
		--n;
	}
}

//signal handler for SIGINT
void sigint_handler(int s){
	int i;
	printf("\n");
	for(i = 0; i < 19; i++) kill(slvs[i], SIGTERM);
	i = 19;
	while(i > 0){
		wait();
		--i;
	}
	sem_cleanup();
	shmctl(shmid, IPC_RMID, (struct shmid_ds*)NULL);
	fprintf(stderr, "Master process: 0 dying due to interrupt \n");
	exit(1);
}

//signal handler for SIGALRM (timeout)
void timeout_handler(int s){
	int i;
	printf("\n");
	for(i = 0; i < 19; i++) kill(slvs[i], SIGQUIT);
	i = 19;
	while(i > 0){
		wait();
		--i;
	}
	sem_cleanup();
	shmctl(shmid, IPC_RMID, (struct shmid_ds*)NULL);
	fprintf(stderr, "Master process: 0 dying due to timeout \n");
	exit(1);
}
