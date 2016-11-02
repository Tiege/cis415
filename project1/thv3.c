#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include "p1fxns.h"

#define num_args 10
#define INTERVAL 500

typedef struct processManager {
	int nprocesses, nprocessors, count, index;
	pid_t *pid;
} procMngr;

static procMngr *ptrMngr;
static pcount = 0;
int nproc, mproc;
pid_t procs[1024];
void sig_handler(int sig) {

	int i;
	//first set of processes initiated
	if (sig == SIGUSR2) {
		for (i = 0; i < ptrMngr->nprocessors; i++)
			kill(ptrMngr->pid[i], SIGCONT);

	ptrMngr->index = 0;
	ptrMngr->count = ptrMngr->nprocessors;
	}
	
	int ret;
	//handle rest of process scheduling
	if (sig == SIGALRM) {

		for (i = 0; i < ptrMngr->nprocessors; i++) {


			kill(ptrMngr->pid[ptrMngr->index], SIGSTOP);
			ptrMngr->index++; 
			printf("STOPPED\n");
			if (ptrMngr->index == ptrMngr->nprocesses)
				ptrMngr->index = 0;

		}

		for (i = 0; i < ptrMngr->nprocessors; i++) {

			kill(ptrMngr->pid[ptrMngr->count], SIGCONT);
			ptrMngr->count++;
			printf("CONTINUED\n");

			if (ptrMngr->count == ptrMngr->nprocesses)
				ptrMngr->count = 0;
		}
		pcount++;
		printf("%d\n", pcount);
		if (pcount == 10) {
		for (i = 0; i < ptrMngr->nprocesses; i++) {

			kill(ptrMngr->pid[i], SIGCONT); }
		}	
	}
	
}
void reap_zombies(int sig) {

	pid_t proc;
	int status;

	while((proc=waitpid(-1, &status, WNOHANG)) > 0) {
		printf("REAPED---");
		printf("%d\n", proc);
	}
	//sleep(10);
}

int main(int argc, char *argv[]) {

	char *p, *command[num_args];
	struct timeval start, stop;
	sigset_t sigSet;
	struct itimerval itimer;

	char *testcmd[] = { "date", NULL };

	ptrMngr = (procMngr*) malloc(sizeof(ptrMngr));

	//set up signal set
	sigemptyset(&sigSet);
	sigaddset(&sigSet, SIGUSR1);

	//implement SIGALRM handler for TH
	signal(SIGALRM, &sig_handler);
	signal(SIGUSR2, &sig_handler);

	//signal(SIGCHLD, &reap_zombies);

	//fetch environment variables TH_NPROCESSES and TH_NPROCESSORS
	if ((p = getenv("TH_NPROCESSES")) != NULL)
		ptrMngr->nprocesses = atoi(p);
	if ((p = getenv("TH_NPROCESSORS")) != NULL)
		ptrMngr->nprocessors = atoi(p);

	//override variables with arguements if applicable
	if (argc == 4 ) { 
		if (argv[1][0] == '-' && argv[1][1] == '-')
			ptrMngr->nprocesses = atoi(&argv[1][2]);
		if (argv[2][0] == '-' && argv[2][1] == '-')
			ptrMngr->nprocessors = atoi(&argv[2][2]);
		command[0] = p1strdup(argv[3]);	/*fetch command from argv[3]*/
	}
	//no nprocesses/nprocessors parameter defined
	else if (argc < 3) {
		command[0] = p1strdup(argv[1]);	/*fetch command from argv[1]*/
	}
	else {
		printf("ERROR: invalid set of arguements\n");
		return 0;
	}

	/*******Add error handling for variations of input for args*******/

	ptrMngr->pid = (pid_t*) malloc(sizeof(pid_t) * ptrMngr->nprocesses);



	mproc = ptrMngr->nprocesses;
	//child process creation loop
	int i;
	for (i = 0; i < ptrMngr->nprocesses; i++) {
		ptrMngr->pid[i] = fork();
		if (ptrMngr->pid[i] == 0) { //CHILD PROCESS
			//set signal for child to wait for = SIGUSR1
			signal(SIGUSR1, &sig_handler);
			int sig;
			sigwait(&sigSet, &sig); /*suspend child proc til signal*/
			if (sig == SIGUSR1) {  /*when SIGUSR1 is received*/
				kill(getpid(), SIGSTOP); //pause for scheduler
				execvp(testcmd[0], testcmd); }
			}
	} 

	//'wake' each process then suspend all
	for (i = 0; i < ptrMngr->nprocesses; i++) 
		kill(ptrMngr->pid[i], SIGUSR1);

	raise(SIGUSR2); /*start first set of processes*/

	//start timer
	gettimeofday(&start, NULL);

	//setup values for interval timer, INTERVAL = 250
	nproc = 0;
	ptrMngr->count = 0;
	ptrMngr->index = 0;
	itimer.it_value.tv_sec = INTERVAL / 1000;
	itimer.it_value.tv_usec = (INTERVAL*1000) % 1000000;
	itimer.it_interval = itimer.it_value;
	//start interval timer = 250ms
	setitimer(ITIMER_REAL, &itimer, NULL);


	int returnstatus;
	//wait for each child process to terminate
	for (i = 0; i < ptrMngr->nprocesses; i++) {
		printf("WAITING?\n");
		waitpid(ptrMngr->pid[i], &returnstatus, 0);
		
		if (returnstatus == 0) {
			printf("REAPED---");
			printf("%d\n", ptrMngr->pid[i]); }
		else {
			printf("FAILED---");
			printf("%d\n", ptrMngr->pid[i]); }
	}
	//raise(SIGCHLD);

	//stop timer, compute difference = elapsed time
	gettimeofday(&stop, NULL);

	//DISPLAY OUTPUT
	float time;
	int itime;
	p1putstr(1, "The elapsed time to execute ");
	p1putint(1, ptrMngr->nprocesses);
	p1putstr(1, " copies of ");
	p1putstr(1, command[0]);
	p1putstr(1, " on ");
	p1putint(1, ptrMngr->nprocessors);
	p1putstr(1, " processors is ");
	time = ((stop.tv_sec * 1000000.0 + stop.tv_usec) - (start.tv_sec * 1000000.0 + start.tv_usec)) / 1000000.0; /*compute time elapsed*/
	p1putint(1, (int)time);
	p1putstr(1, ".");
	itime = (int)(time * 1000) % ((int)time*1000); /*obtain remainder*/
	p1putint(1, itime);
	p1putstr(1, "sec\n");

	return 0;
}


