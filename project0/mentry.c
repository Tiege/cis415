#include "mentry.h"
#include <stdio.h>
#include <ctype.h>

/* me_get returns the next file entry, or NULL if end of file */
MEntry *me_get(FILE *fd){
	MEntry *entry;
	char * line = NULL;
	size_t len = 0;
	char string[50];
	
	//allocate space for a new mail entry
	entry = (MEntry *) malloc(sizeof(MEntry));
	
	//fill three line address into full_address MEntry member
	int n = 3;
	do {
	entry->full_address = (char *) realloc(entry->full_address, 50);
    	getline(&line, &len, fd);
	strcat(entry->full_address, line);

	switch (n) {
		case 3:
		//fetch surname from first line for surname MEntry member
			strcpy(string, line);
			entry->surname = (char *) malloc(50);
			entry->surname = strtok(string, ",");
			
		break;	
		case 2:
		//store house number in entry struct, 0 if none provided
		if (isdigit(line[0]))
			sscanf(line, "%d", &entry->house_number);
		else
			entry->house_number = 0;
		break;
		case 1:
		//store zipcode in entry struct by scanning words between
		//spaces until digits are found
			entry->zipcode = (char *) malloc(12);
			entry->zipcode = strtok(line, " ");
			while (entry->zipcode != NULL && !isdigit(entry->zipcode[0])) {
				entry->zipcode = strtok(NULL, " ");
			}
			entry->zipcode = strtok(entry->zipcode, "\n");
		break;
	}
	n--;
	} while (n > 0);

	return entry;

}

/* me_hash computes a hash of the MEntry, mod size */
unsigned long me_hash(MEntry *me, unsigned long size);

/*me_print prints the full address on fd */
void me_print(MEntry *me, FILE *fd) {
	printf("%s", me->full_address);
}

/* me_compare compares two mail entries, returning <0, 0, >0 if
 * me1<me2, me1==me2, me1>me2
 */
int me_compare(MEntry *me1, MEntry *me2);

/* me_destroy destroys the mail entry */
void me_destroy(MEntry *me) {
	free(me->surname);
	free(me->zipcode);
	free(me->full_address);
	free(me);
}
