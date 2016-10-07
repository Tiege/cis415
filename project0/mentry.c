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

	entry->full_address = (char *) malloc(350);
	
	entry->surname = (char *) malloc(400);
		
	strncpy(entry->full_address, string, 350);
	strncpy(entry->surname, strtok(string, ","), 400);

	
	if(fgets(string2, sizeof(string2), fd) == NULL)
		return NULL;;
	strcat(entry->full_address, string2);
	sscanf(string2, "%d", &entry->house_number);

	//if no house number set to 0
	if (entry->house_number == '\0')
		entry->house_number = 0;

	entry->zipcode = (char *) malloc(500);

	if(fgets(string3, sizeof(string3), fd) == NULL)
		return NULL;
	strcat(entry->full_address, string3);

	char * zip;
	char * num = "0123456789";
	zip = strpbrk(string3, num);
	
	strncpy(entry->zipcode, zip, 500);
	entry->zipcode = strtok(entry->zipcode, "\n");

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
int me_compare(MEntry *me1, MEntry *me2){

	int result = strcmp(me1->surname, me2->surname);

	if (result == 0)
		result = me1->house_number == me2->house_number ? 0 : 1;

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
