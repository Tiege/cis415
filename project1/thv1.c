#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "p1fxns.h"

#define num_args 10

int main(int argc, char *argv[]) {

	int nprocesses, nprocessors;
	char *p, *command[num_args];
	pid_t pid[1024];
	struct timeval start, stop;


	//fetch environment variables TH_NPROCESSES and TH_NPROCESSORS
	if ((p = getenv("TH_NPROCESSES")) != NULL)
		nprocesses = atoi(p);
	if ((p = getenv("TH_NPROCESSORS")) != NULL)
		nprocessors = atoi(p);

	//override variables with arguements if applicable
	if (argc == 4 ) { 
		if (argv[1][0] == '-' && argv[1][1] == '-')
			nprocesses = atoi(&argv[1][2]);
		if (argv[2][0] == '-' && argv[2][1] == '-')
			nprocessors = atoi(&argv[2][2]);
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

	//start timer
	//start = clock();
	gettimeofday(&start, NULL);

	//child process creation loop
	int i;
	for (i = 0; i < nprocesses; i++) {
		pid[i] = fork();
		if (pid[i] == 0)
			execvp(command[0], command);
	} 
	//wait for each child process to terminate
	for (i = 0; i < nprocesses - 1; i++)
		wait(pid[i]);

	//stop timer, compute difference = elapsed time
	gettimeofday(&stop, NULL);

	//DISPLAY OUTPUT
	float time;
	p1putstr(1, "The elapsed time to execute ");
	p1putint(1, nprocesses);
	p1putstr(1, " copies of ");
	p1putstr(1, command[0]);
	p1putstr(1, " on ");
	p1putint(1, nprocessors);
	p1putstr(1, " processors is ");
	time = ((stop.tv_sec * 1000000.0 + stop.tv_usec) - (start.tv_sec * 1000000.0 + start.tv_usec)) / 1000000.0;
	printf("%7.3f", time); //FIX ME: must use sys call for output
	//try: convert time -> char * string -> reverse forloop -> float string
	p1putstr(1, "sec\n");
	


	return 0;
}
