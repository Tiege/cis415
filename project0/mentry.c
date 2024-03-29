/***TREVOR JONES duckid=tjones9 (951384223) CIS 415 PROJECT 0*******
*This file is my own work. It works correctly and the expected output
*when linked to the given mListLL.c file was correct with no memory leaks
********************************************************************/

#include "mentry.h"
#include <stdio.h>
#include <ctype.h>

/* me_get returns the next file entry, or NULL if end of file */
MEntry *me_get(FILE *fd){
	MEntry *entry;
	char string[1024];
	char string2[1024];
	char string3[1024];

	//allocate space for a new mail entry
	entry = (MEntry *) malloc(sizeof(MEntry));
	if (entry == NULL)
		return NULL;

	//Reads first line, if NULL == EOF
	if(fgets(string, sizeof(string), fd) == NULL) {
		free(entry);
		return NULL;
	}

	//Allocating space for pointer variables
	entry->full_address = (char *) malloc(350);
	entry->surname = (char *) malloc(400);
	entry->zipcode = (char *) malloc(500);
		
	//Add first line to full_address and first word to surname
	strncpy(entry->full_address, string, 350);
	strncpy(entry->surname, strtok(string, ","), 400);

	//Reads second line of entry, adds to full_address
	//strips number for house_number
	if(fgets(string2, sizeof(string2), fd) == NULL)
		return NULL;
	strcat(entry->full_address, string2);
	sscanf(string2, "%d", &entry->house_number);

	//if no house number set to 0
	if (entry->house_number == '\0')
		entry->house_number = 0;

	//Read third line of mail entry, add to full_address
	if(fgets(string3, sizeof(string3), fd) == NULL)
		return NULL;
	strcat(entry->full_address, string3);

	//strip digits for zipcode, remove trailing '\n'
	char * zip;
	char * num = "0123456789";
	zip = strpbrk(string3, num);
	strncpy(entry->zipcode, zip, 500);
	entry->zipcode = strtok(entry->zipcode, "\n");

	return entry;

}

/* me_hash computes a hash of the MEntry, mod size */
unsigned long me_hash(MEntry *me, unsigned long size) {

	char * t;
	t = strdup(me->surname);
	free(t);//strdup returns malloced pointer, must free to avoid leak

	//calculate unique hash using surname+house_number+zipcode % size
	unsigned long hash;
	for (hash = 0; *t != '\0'; t++) {
		hash += (int)*t + 31 * hash;		
	}
	hash += me->house_number;
	hash += atoi(me->zipcode);
	
	printf("%d\n", hash);
	return hash % size;
}

/*me_print prints the full address on fd */
void me_print(MEntry *me, FILE *fd) {

	fprintf(fd, "%s", me->full_address);
}

/* me_compare compares two mail entries, returning <0, 0, >0 if
 * me1<me2, me1==me2, me1>me2
 */
int me_compare(MEntry *me1, MEntry *me2){

	int result = strcasecmp(me1->surname, me2->surname);

	if (result == 0)
		result = ( me1->house_number == me2->house_number ) ? 0 : 1;

	if (result == 0)
		result = strcmp(me1->zipcode, me2->zipcode);
	
	return result;
}

/* me_destroy destroys the mail entry */
void me_destroy(MEntry *me) {
	free(me->zipcode);
	free(me->surname);
	free(me->full_address);
	free(me);	
}
