/***TREVOR JONES *** tjones9 *** PROJECT 1 *** THV(1) *** CIS 415*******
*This source file is my own original work. This is a process scheduler simply starting child processes and waiting for them to exit before terminating as the parent.***********************************/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "p1fxns.h"

#define num_args 10

int wait(int);

int main(int argc, char *argv[]) {

	int nprocesses, nprocessors;
	char *p, *buff, *buff2, *buff3, *buff4, *command[num_args];
	pid_t pid[1024];
	struct timeval start, stop;


	//fetch environment variables TH_NPROCESSES and TH_NPROCESSORS
	if ((p = getenv("TH_NPROCESSES")) != NULL)
		nprocesses = atoi(p);
	if ((p = getenv("TH_NPROCESSORS")) != NULL)
		nprocessors = atoi(p);

	//check and acquire cmdline arguement commands
	int a;
	for (a = 1; a < argc; a++) {
		if (argv[a][0] == '-' && argv[a][11] == 's' && argv[a][13] != '\0')
			buff = p1strdup(argv[a]); //first cmd

		if (argv[a][0] == '-' && argv[a][11] == 'r' && argv[a][14] != '\0')
			buff2 = p1strdup(argv[a]); //second cmd

		if (argv[a][0] == '-' && argv[a][8] == 'd' && argv[a][10] != '\0')
			buff3 = p1strdup(argv[a]); //third cmd
		}
		//cut off --nprocesses from nprocesses int
		int found = 0;
		for (;*buff != '\0'; ++buff) {
			if (found) {
				nprocesses = p1atoi(p1strdup(buff));
				break;
			}
			if (buff[0] == '=')
				found = 1;
		}
		//cut off --nprocessors from nprocessors int
		int found2 = 0;
		for (;*buff2 != '\0'; ++buff2) {	
			if (found2) {
				nprocessors = p1atoi(p1strdup(buff2));
				break;
			}
			if (buff2[0] == '=')
				found2 = 1;
		}
		//cut off --command= from command vector
		int found3 = 0;
		for (;*buff3 != '\0'; ++buff3) {	
			if (found3) {
				buff4 = p1strdup(buff3);
				break;
			}
			if (buff3[0] == '=')
				found3 = 1;
		}


	
	//fill command vector for execvp()
	char cmds[100];
	int m = 0;
	int n = 0;
	while((m = p1getword(buff4, m, cmds)) != -1) {
		command[n] = p1strdup(cmds);
		n++;
	}
	command[n] = NULL; //last element = NULL to end vector

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
