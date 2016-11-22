#include <pthread.h>
#include <stdio.h>
#include "sectordescriptor.h"
#include "block.h"
#include "pid.h"
#include "freesectordescriptorstore.h"
#include "freesectordescriptorstore_full.h"
#include "sectordescriptorcreator.h"
#include "diskdevice.h"
#include "voucher.h"
#include "BoundedBuffer.h"

//global buffers shared between disk device and application threads
BoundedBuffer *rbuff, *wbuff;

Block sec;

//semafores
int writeSEM, readSEM;

struct voucher {
	int val;
};

struct voucher vou[10];

int buffer[10];


void* writeDisk(void* arg) {

		while (1) {
		if (writeSEM > 0) {
		SectorDescriptor *wSD;

		if(nonblockingReadBB(wbuff, &wSD) == 0)
			wSD = (SectorDescriptor*) blockingReadBB(wbuff);
			

		printf("[DRIVER WRITE:%u\n", sector_descriptor_get_pid(wSD));
		
		write_sector(arg, wSD);
		writeSEM--;
		}
		}
	
	return NULL;
}

void* readDisk(void* arg) {

		while (1) {
		if (readSEM > 0) {
		SectorDescriptor *rSD;

		if(nonblockingReadBB(rbuff, &rSD) == 0)
			rSD = blockingReadBB(rbuff);

		printf("[DRIVER READ:%u\n", sector_descriptor_get_pid(rSD));
		

		read_sector(arg, rSD);
		readSEM--;
		}
		}
	
	return NULL;
}



/*
 * the following calls must be implemented by the students
 */

/*
 * called before any other methods to allow you to initialize data
 * structures and to start any internal threads.
 *
 * Arguments:
 *   dd: the DiskDevice that you must drive
 *   mem_start, mem_length: some memory for SectorDescriptors
 *   fsds_ptr: you hand back a FreeSectorDescriptorStore constructed
 *             from the memory provided in the two previous arguments
 */
void init_disk_driver(DiskDevice *dd, void *mem_start, unsigned long mem_length,
		      FreeSectorDescriptorStore **fsds) {

	FreeSectorDescriptorStore *tempFSDS;
		
	//create FSDS
	tempFSDS = create_fsds();
	//populate FSDS with sector descriptors using mem_start and mem_len
	create_free_sector_descriptors(tempFSDS, mem_start, mem_length);

	//create bounded buffer: one for read, one for write
	rbuff = createBB(500);
	wbuff = createBB(500);

	writeSEM = 0;
	readSEM = 0;

	//create threads
	pthread_t write_thread;
	pthread_t read_thread;

	pthread_create (&write_thread, NULL, &writeDisk, &dd);
	pthread_create (&read_thread, NULL, &readDisk, &dd);

	
	//return back FSDS to call
	*fsds = tempFSDS;



}

/*
 * the following calls are used to write a sector to the disk
 * the nonblocking call must return promptly, returning 1 if successful at
 * queueing up the write, 0 if not (in case internal buffers are full)
 * the blocking call will usually return promptly, but there may be
 * a delay while it waits for space in your buffers.
 * neither call should delay until the sector is actually written to the disk
 * for a successful nonblocking call and for the blocking call, a voucher is
 * returned that is required to determine the success/failure of the write
 */
void blocking_write_sector(SectorDescriptor *sd, Voucher **v) {

	
	//Voucher *result = NULL;

	blockingWriteBB(wbuff, sd); //send sd to buffer

	
	writeSEM++;


}

int nonblocking_write_sector(SectorDescriptor *sd, Voucher **v) {

	
	Voucher *result = NULL;

	int value = nonblockingWriteBB(wbuff, sd); //send sd to buffer
	//*v = result;

	if ( value != 0)
		writeSEM++;


	return value;

}

/*
 * the following calls are used to initiate the read of a sector from the disk
 * the nonblocking call must return promptly, returning 1 if successful at
 * queueing up the read, 0 if not (in case internal buffers are full)
 * the blocking callwill usually return promptly, but there may be
 * a delay while it waits for space in your buffers.
 * neither call should delay until the sector is actually read from the disk
 * for successful nonblocking call and for the blocking call, a voucher is
 * returned that is required to collect the sector after the read completes.
 */
void blocking_read_sector(SectorDescriptor *sd, Voucher **v) {


	Voucher *result = NULL;

	blockingWriteBB(rbuff, sd); //send sd to buffer
	readSEM++;

	//result->val = 1;

	//*v = result;

}
int nonblocking_read_sector(SectorDescriptor *sd, Voucher **v) {

	//printf("READ\n");

	Voucher *result = NULL;

	int value = nonblockingWriteBB(rbuff, sd); //send sd to buffer

	//*v = result;

	if ( value != 0)
		readSEM++;

	return value;

}

/*
 * the following call is used to retrieve the status of the read or write
 * the return value is 1 if successful, 0 if not
 * the calling application is blocked until the read/write has completed
 * if a successful read, the associated SectorDescriptor is returned in *sd
 */
int redeem_voucher(Voucher *v, SectorDescriptor **sd) {

	return 1;
}

