/***TREVOR JONES***CIS 415***PROJECT 2*******************
The implementation of methods in this file are my original work. This code is animplementation of a diskdriver to bridge requests between X number of applications to read/write to the disk and the corresponding DiskDevice. The DiskDevice can only read and write one request at a time and may be slow in doing so while the requesting calls must return immediately to continue work. This results in adding a BoundedBuffer to queue the read/write requests to give the DiskDevice enough time to properly siphon through requests without impeding the calling requestapplications.
******************NOT COMPLETE*****************************
**********************************************************/
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

#define PROC_THREAD_MAX 20

/*******************GLOBAL VARIABLES****************************/
//global buffers shared between disk device and application threads
BoundedBuffer *rbuff, *wbuff;

//semaphores used to keep diskdriver threads idle
int writeSEM, readSEM;

//voucher data structure with flag for read or write 
struct voucher {
	int readORwrite; // Read/Write flag: 0 = write, 1 = read
};

//array of vouchers for passing pointers to vouchers to fake applications
//size: should not exceed MAX calls = 20 = 10 x 2 (10 reads, 10 writes)
//with corresponding index variable
Voucher va[PROC_THREAD_MAX];
static int index = 0;

//mutex lock for critical sections; initialized
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*****writeDisk/readDisk(DiskDevice)*********************
*The threads that run the writing/reading portion of the DiskDriver.
*Uses semaphores to remain idle or perform read/write
*********************************************************/
void* writeDisk(void* arg) {
	
	while(1) {			//loop keeps driver idle

		if ( writeSEM > 0 ) {	//check semaphore

		SectorDescriptor *wSD;
		if(nonblockingReadBB(wbuff, &wSD) == 0)
			wSD = blockingReadBB(wbuff);   //read SD from wbuffer
			
		printf("DRIVER WRITE: APPLICATION = %u SECTOR = %u\n", 
		sector_descriptor_get_pid(wSD), 
		sector_descriptor_get_block(wSD));

		write_sector(arg, wSD); //write to disk using 
		writeSEM--; 		//decrement semaphore
		}
	}
	return NULL;
}
void* readDisk(void* arg) {
	
	while (1) {			//loop keeps driver idle
		if ( readSEM > 0 ) {	//check semaphore

		SectorDescriptor *rSD;
		if(nonblockingReadBB(rbuff, &rSD) == 0)
			rSD = blockingReadBB(rbuff);  //read SD from rbuffer

		printf("DRIVER READ: APPLICATION = %u SECTOR = %u\n", 
		sector_descriptor_get_pid(rSD), 
		sector_descriptor_get_block(rSD));

		read_sector(arg, rSD);	//read from disk using SD

		readSEM--;		//decrement semaphore
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
	rbuff = createBB(50);
	wbuff = createBB(50);

	//initialize semaphores
	readSEM = 0;
	writeSEM = 0;

	//create threads for reading and writing
	pthread_t write_thread;
	pthread_t read_thread;

	//thread calls: passing pointer to DiskDevice
	pthread_create (&write_thread, NULL, &writeDisk, dd);
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

	pthread_mutex_lock(&lock);		//lock critical section

	*v = &va[index];			//assign thread voucher return
	va[index].readORwrite = 0;		//set flag: 0 (write)
	index++;				//increment voucher[index]

	if ( index == PROC_THREAD_MAX )		//reset index if exceeds MAX
		index = 0;

	pthread_mutex_unlock(&lock);		//unlock critical section
	

	blockingWriteBB(wbuff, sd); 		//send sd to buffer

	writeSEM++;				//increment write semaphore

}

int nonblocking_write_sector(SectorDescriptor *sd, Voucher **v) {


	pthread_mutex_lock(&lock);		//lock critical section

	*v = &va[index];			//assign thread voucher return
	va[index].readORwrite = 0;		//set flag: 0 (write)
	index++;				//increment voucher[index]

	if ( index == PROC_THREAD_MAX )		//reset index if exceeds MAX
		index = 0;

	pthread_mutex_unlock(&lock);		//unlock critical section
	

	blockingWriteBB(wbuff, sd); 		//send sd to buffer

	writeSEM++;				//increment semaphore

	int value = nonblockingWriteBB(wbuff, sd); //send sd to buffer
	
	if( value != 0)		//only increment if successful write to buffer
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

	pthread_mutex_lock(&lock);		//lock critical section

	*v = &va[index];			//assign thread voucher return
	va[index].readORwrite = 1;		//set flag: 1 (read)
	index++;				//increment voucher[index]

	if ( index == PROC_THREAD_MAX )		//reset index if exceeds MAX
		index = 0;

	pthread_mutex_unlock(&lock);		//unlock critical section


	blockingWriteBB(rbuff, sd);		 //send sd to buffer

	readSEM++;				//increment read semaphore
	
}

int nonblocking_read_sector(SectorDescriptor *sd, Voucher **v) {

	
	pthread_mutex_lock(&lock);		//lock critical section

	*v = &va[index];			//assign thread voucher return
	va[index].readORwrite = 1;		//set flag: 1 (read)
	index++;				//increment voucher[index]

	if ( index == PROC_THREAD_MAX )		//reset index if exceeds MAX
		index = 0;

	pthread_mutex_unlock(&lock);		//unlock critical section	

	int val = nonblockingWriteBB(rbuff, sd); //send sd to buffer

	if( val != 0)		//only increment if successful write to buffer
		readSEM++;

	return val;

}

/*
 * the following call is used to retrieve the status of the read or write
 * the return value is 1 if successful, 0 if not
 * the calling application is blocked until the read/write has completed
 * if a successful read, the associated SectorDescriptor is returned in *sd
 */
int redeem_voucher(Voucher *v, SectorDescriptor **sd) {

	int result;

	pthread_mutex_lock(&lock);		//lock critical section

	if (v->readORwrite == 0) {		//if voucher is for write
		//check if read:SD is still in buffer, return result
		result = nonblockingReadBB(rbuff, &sd);
	} else {
		//else check if write:SD is in buffer, return result
		result = nonblockingReadBB(wbuff, &sd);
	}	

	pthread_mutex_unlock(&lock);		//unlock critical section

	return !result;
}

