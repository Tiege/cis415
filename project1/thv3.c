#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include "p1fxns.h"

#define num_args 10
#define INTERVAL 250 //250ms

typedef struct processManager {
	int nprocesses, nprocessors, count, index;
	pid_t *pid;
} procMngr;

static procMngr *ptrMngr;
static pcount = 0;

/*Main signal handler***********************************************
*Called when SIGALRM or SIGUSR2 signalled. Initial loop to start first processes.
*Alternate between two loops of size 'nprocessors', one to stop, next to continue for pid[n]
********************************************************************/
void sig_handler(int sig) {

	int i;
	//first set of processes initiated
	if (sig == SIGUSR2) {
		for (i = 0; i < ptrMngr->nprocessors; i++)
			kill(ptrMngr->pid[i], SIGCONT); /* resume initial child processes*/

	//set marks
	ptrMngr->index = 0;	
	ptrMngr->count = ptrMngr->nprocessors;
	}

	//handle rest of process scheduling
	if (sig == SIGALRM) {

		//loop through nprocessor times STOP children processes 
		for (i = 0; i < ptrMngr->nprocessors; i++) {

			kill(ptrMngr->pid[ptrMngr->index], SIGSTOP);
			ptrMngr->index++;  //increment mark

			if (ptrMngr->index == ptrMngr->nprocesses) /*wrap around back to 0 index when at end*/
				ptrMngr->index = 0;
		}

		//loop through nprocessor times CONTINUE children processes 
		for (i = 0; i < ptrMngr->nprocessors; i++) {

			kill(ptrMngr->pid[ptrMngr->count], SIGCONT);
			ptrMngr->count++; //increment mark

			if (ptrMngr->count == ptrMngr->nprocesses) /*wrap around back to 0 index when at end*/
				ptrMngr->count = 0;
		}
	}
	
}
/*SIGHLD handler, called when a child terminates************
*when child terminated detected, decriments process counter by 1
************************************************************/
void reap_zombies(int sig) {

	pid_t proc;
	int status;

	//detect when child process terminates
	while((proc=waitpid(-1, &status, WNOHANG)) > 0)
		pcount--; /*reap -> decrement process counter*/

}

int main(int argc, char *argv[]) {

	char *p, *command[num_args];
	struct timeval start, stop;
	sigset_t sigSet;
	struct itimerval itimer;

	char *testcmd[] = { "./test", NULL };

	ptrMngr = (procMngr*) malloc(sizeof(ptrMngr));

	//set up signal set
	sigemptyset(&sigSet);
	sigaddset(&sigSet, SIGUSR1);

	//implement SIGALRM handler for TH
	signal(SIGALRM, &sig_handler);
	signal(SIGUSR2, &sig_handler);

	//set SIGCHLD for handler to reap
	signal(SIGCHLD, &reap_zombies);



	//fetch environment variables TH_NPROCESSES and TH_NPROCESSORS
	if ((p = getenv("TH_NPROCESSES")) != NULL)
		ptrMngr->nprocesses = atoi(p);
	if ((p = getenv("TH_NPROCESSORS")) != NULL)
		ptrMngr->nprocessors = atoi(p);

	//int a;
	//for (a = 1; a < argc; a++) {
		//if (argv[a][0] == '-' && argv[a][--nprocesses

	//}
	//override variables with arguements if applicable
	/*if (argc == 4 ) { 
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
	*/
	/*******Add error handling for variations of input for args*******/

	//alloc array of process id's
	ptrMngr->pid = (pid_t*) malloc(sizeof(pid_t) * ptrMngr->nprocesses);

	//set process count for parent process to wait
	pcount = ptrMngr->nprocesses;

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

	int q;
	//'wake' each process then suspend all
	for (i = 0; i < ptrMngr->nprocesses; i++) {
		for (q=0;q<1000000;q++) ; //pause before waking children up
		kill(ptrMngr->pid[i], SIGUSR1);
	}

	raise(SIGUSR2); /*start first set of processes*/

	//start timer
	gettimeofday(&start, NULL);

	//setup values for interval timer, INTERVAL = 250
	ptrMngr->count = 0;
	ptrMngr->index = 0;
	itimer.it_value.tv_sec = INTERVAL / 1000;
	itimer.it_value.tv_usec = (INTERVAL*1000) % 1000000;
	itimer.it_interval = itimer.it_value;
	//start interval timer = 250ms
	setitimer(ITIMER_REAL, &itimer, NULL);

	//parent process waits for all children to terminate and be reaped
	while(pcount > 0)
		;

	//stop timer, compute difference = elapsed time
	gettimeofday(&stop, NULL);

	//DISPLAY OUTPUT
	p1putstr(1, "The elapsed time to execute ");
	p1putint(1, ptrMngr->nprocesses);
	p1putstr(1, " copies of ");
	p1putstr(1, command[0]);
	p1putstr(1, " on ");
	p1putint(1, ptrMngr->nprocessors);
	p1putstr(1, " processors is ");
	int stime, mtime;
	stime = (stop.tv_sec) - (start.tv_sec);
	p1putint(1, stime);
	p1putstr(1, ".");
	mtime = (stop.tv_usec - start.tv_usec) / 1000;
	if (mtime < 0)
		mtime *= -1;
	p1putint(1, mtime);
	p1putstr(1, "sec\n");

	//free memory
	free(command[0]);
	free(ptrMngr->pid);
	free(ptrMngr);

	return 0;
}


