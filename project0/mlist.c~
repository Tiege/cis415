/***TREVOR JONES duckid=tjones9 (951384223) CIS 415 PROJECT 0*******
This file is my own work, although I did use references from the C
Programming Manual for help implementating parts of the hashtable such as the
lookup(). I was unable to correctly implement resizing for the hashtable. The
program runs correctly when testing against 30,000 entries (M.txt). There are no memory leaks.
********************************************************************/
#include "mlist.h"
#include <stdlib.h>
#include <stdio.h>


typedef struct mlistnode {
	struct mlistnode *next;
	int num_links;
	MEntry *entry;
} MListNode;

struct mlist {
	struct mlistnode **hashTable;
	unsigned long list_size;
};

/* ml_create - created a new mailing list */
MList *ml_create(void) {
	MList *p = NULL;
	int i;
	if ((p = (MList *)malloc(sizeof(MList))) != NULL) {
		p->list_size = 5381;
		p->hashTable = malloc (sizeof(MListNode*) * p->list_size);
	}
	return p;
}

/* ml_add - adds a new MEntry to the list;
 * returns 1 if successful, 0 if error (malloc)
 * returns 1 if it is a duplicate */
int ml_add(MList **ml, MEntry *me) {
	
	MList *p, *r;
	MListNode *q, *m;
	p = *ml;
	r = NULL;
	int prev_size;

	if (ml_lookup(p, me) != NULL)
		return 1;
	if ((q = (MListNode *)malloc(sizeof(MListNode))) == NULL)
		return 0;

		//add me to head of hashbucket and current to next
		q->entry = me;
		q->next = p->hashTable[me_hash(me, p->list_size)];

		//add nummber of links per bucket entry, 0 if none
		if (q->next == NULL)
			q->num_links = 0;
		else
			q->num_links = q->next->num_links + 1;

		//insert mail entry into hash table
		p->hashTable[me_hash(me, p->list_size)] = q;

		/****Attempt at resizing hashtable === UNSUCCESSFUL
		* Unable to successfully copy over elements of old table
		* into new resized table once a bucket hits 20 elements
		
		if (q->num_links == 20) { 
			//update number of buckets
			prev_size = p->list_size;  
			p->list_size = (p->list_size * 2);  

			//resize the hashtable
			r = (MList *)malloc(sizeof(MList));
			r->hashTable = malloc (sizeof(MListNode*) * p->list_size); 
		//loop through old table, copy old elements to new hash	
		int n;
		for (n = 0; n < prev_size; n++) {		
			while (p->hashTable[n] != NULL) {
				m = (MListNode *)malloc(sizeof(MListNode));
				m = p->hashTable[n];
				m->num_links = 0;
				r->hashTable[me_hash(m->entry, p->list_size)] = m;
				p->hashTable[n] = p->hashTable[n]->next;
				free(m);
			}
		}
		p->hashTable = r->hashTable;
		}*/
	return 1;
}

/* ml_lookup - looks for MEntry in the list, returns matching entry or     NULL */
MEntry *ml_lookup(MList *ml, MEntry *me) {
	MList *p;
	MListNode *q;
	
	p = ml;
	for (q = p->hashTable[me_hash(me, p->list_size)]; q != NULL; q = q->next) {
		
		if (me_compare(me, q->entry) == 0) 
			return q->entry;
	}
	return NULL;
}

/* ml_destroy - destroy the mailing list */
void ml_destroy(MList *ml) {
	MList * p;
	MListNode *q;
	int i;
	p = ml;
	for (i = 0; i < ml->list_size; i++) {
		q = p->hashTable[i];
		while (q != NULL) {
			MListNode *r = q->next;
			me_destroy(q->entry);
			free(q);
			q = r;
		}
	}
	free(p->hashTable);
	free(ml);
}
