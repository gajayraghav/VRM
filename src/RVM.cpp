//============================================================================
// Name        : RVM.cpp
// Author      : Ajay
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/rvm.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <sstream>
using namespace std;

/*
 * rvm_t rvm_init(const char *directory) - Initialize the library with the specified directory as backing store.
 */

rvm_t rvm_init(const char *directory) {
	rvm_t rvm = new rvm_details;
	struct stat st = { 0 };

	if (stat(directory, &st) == -1) {
		mkdir(directory, 0766);
		rvm->backingStore = directory;
		rvm->backingStore.append("/");
		rvm->storage_size = 0;
		rvm->memSeg_count = 0;
		rvm->flog = fopen(
				(rvm->backingStore + string("transaction.log")).c_str(), "w");
		rvm->ftrace = fopen((rvm->backingStore + string("trace.log")).c_str(),
				"w");
		for (int i = 0; i < MAX_SEGMENTS; i++) {
			rvm->memSegs[i] = (memSeg *) malloc(sizeof(memSeg));
			rvm->memSegs[i]->mapped = 0;
			rvm->memSegs[i]->dirty = 0;
			//rvm->memSegs[i]->segName = " " ;
			rvm->memSegs[i]->Segmentsize = 0;
			rvm->memSegs[i]->fsegment = NULL;
		}
		if ((rvm->flog == NULL) || (rvm->ftrace == NULL)) {
			perror("rvm_init: Unable to create the log/trace file");
		} else {
			TRACE(rvm, ">>>Init<<<\n");
			printf("\n created the log & trace file \n");
		}
	} else {
		perror("rvm_init: Directory already exists");
		rvm->backingStore = "dirExistS"; // E and S in cap to avoid case when user creates direxists as a dir input
	}

	return rvm;

}
/*
 * void *rvm_map(rvm_t rvm, const char *segname, int size_to_create) - map a segment from disk into memory.
 * If the segment does not already exist, then create it and give it size size_to_create. If the segment
 * exists but is shorter than size_to_create, then extend it until it is long enough. It is an error to try
 * to map the same segment twice.
 */

void* rvm_map(rvm_t rvm, const char * segname, int size_to_create) {
	TRACE(rvm, ">>>>> rvm_map\n");
	printf("\n in map \n");
	// find if the segment already exists.
	int segment_index = 0;
	bool found = false;
	for (segment_index = 0; segment_index < rvm->memSeg_count; segment_index++) {
		if (!strcmp(rvm->memSegs[segment_index]->segName, segname)) {
			found = true;
			if (rvm->memSegs[segment_index]->mapped == 1) {
				perror("\n abort \n");
				return (void*) 0;
			}
			break;
		}
	}
	if (found == true) {
		/*
		 *  if found, check the size and do some alterations.
		 */
		if (rvm->memSegs[segment_index]->Segmentsize < size_to_create) {
			printf("\n size less than requested \n");
			// increase the size of the segment
			rvm->storage_size -= rvm->memSegs[segment_index]->Segmentsize;
			rvm->memSegs[segment_index]->Segmentsize = size_to_create;
			rvm->storage_size += rvm->memSegs[segment_index]->Segmentsize;
			rvm->memSegs[segment_index]->mapped = 1;
			printf("\n storage size - %ld \n", rvm->storage_size);
		} else if (rvm->memSegs[segment_index]->Segmentsize > size_to_create) {
			printf("\n size more than requested \n");
			// abort
			//printf("\n storage size - %ld \n", rvm->storage_size);
			perror("\n cant remap the same segment \n");
		} else {
			// dont alter the size
			rvm->memSegs[segment_index]->mapped = 1;
			printf("\n storage size - %ld \n", rvm->storage_size);
		}
		return ((void*) rvm->memSegs[segment_index]->fsegment);
	} else {
		/*
		 * lets create the new segment
		 */
		printf("\n create the new segment - %d \n", (int) rvm->memSeg_count);
		if (rvm->memSeg_count != MAX_SEGMENTS) {
			rvm->memSegs[rvm->memSeg_count]->fsegment
					= fopen(
							(rvm->backingStore + string(segname) + ".txt").c_str(),
							"w");
			if (rvm->memSegs[rvm->memSeg_count]->fsegment != NULL) {
				strcpy(rvm->memSegs[rvm->memSeg_count]->segName, segname);
				//				rvm->memSegs[rvm->memSeg_count]->segName.assign(segname);// = string(segname);
				rvm->memSegs[rvm->memSeg_count]->dirty = 0;
				rvm->memSegs[rvm->memSeg_count]->Segmentsize = size_to_create;
				rvm->memSeg_count = rvm->memSeg_count + 1;
				rvm->storage_size += size_to_create;
				rvm->memSegs[segment_index]->mapped = 1;
				printf("\n storage size - %ld \n", rvm->storage_size);
				return ((void*) rvm->memSegs[rvm->memSeg_count - 1]->fsegment);
			}
		}
	}

	return (void*) 0;

}

/*
 * void rvm_unmap(rvm_t rvm, void *segbase) - unmap a segment from memory.
 */
void rvm_unmap(rvm_t rvm, void *segbase) {
	int segment_index = 0;
	bool found = false;
	for (segment_index = 0; segment_index < rvm->memSeg_count; segment_index++) {
		if (!strcmp(rvm->memSegs[segment_index]->segName, (char*) segbase)) {
			found = true;
			break;
		}
	}
	if (found == true) {
		if (rvm->memSegs[segment_index]->dirty == 1) {
			perror(
					"\n cant unmap a dirty segment. Need to commit/abort before unmap");
		} else {
			rvm->memSegs[segment_index]->mapped = 0;
			rvm->memSegs[segment_index]->dirty = 0;
			fclose(rvm->memSegs[segment_index]->fsegment);
			rvm->memSegs[segment_index]->fsegment = NULL;
			rvm->memSeg_count--;
			printf("\n unmapped - %d", rvm->memSeg_count);
		}
	} else {
		perror("\n cant unmap a non existing segment");
	}
}

/*
 * void rvm_destroy(rvm_t rvm, const char *segname) - destroy a segment completely, erasing its backing store.
 * This function should not be called on a segment that is currently mapped.
 */

void rvm_destroy(rvm_t rvm, const char *segname) {
	int segment_index = 0;
	bool found = false;
	for (segment_index = 0; segment_index < rvm->memSeg_count; segment_index++) {
		if (!strcmp(rvm->memSegs[segment_index]->segName, segname)) {
			found = true;
			break;
		}
	}
	printf("\n hello \n");
	printf("\n found %s",rvm->memSegs[segment_index]->segName);
	if (found == true)
	{
		if (rvm->memSegs[segment_index]->mapped == 0)
		{
			rvm->storage_size -= rvm->memSegs[segment_index]->Segmentsize;
			rvm->memSegs[segment_index]->mapped = 0;
			if (rvm->memSegs[segment_index]->fsegment != NULL)
			{
				perror("File is open elsewhere");
				fclose(rvm->memSegs[segment_index]->fsegment);
			}
			rvm->memSegs[segment_index]->fsegment = NULL;
			rvm->memSegs[segment_index]->dirty = 0;
			rvm->memSegs[segment_index]->Segmentsize = 0;
			rvm->memSeg_count--;
			printf("\n to remove -%s \n", (rvm->backingStore + string(segname)+ ".txt").c_str());
			unlink((rvm->backingStore + string(segname)+".txt").c_str());
			printf("\n removed %d ", (int) rvm->memSeg_count);
		}
		else
		{
			perror("\n cant destroy a mapped segment. Need to unmap before destroy");
		}
	}
	else
	{
		perror("\n cant destroy a non existing segment");
	}
}

trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases) {
	trans_t tid = NULL;

	return tid;
}

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size) {

}

void rvm_commit_trans(trans_t tid) {

}

void rvm_commit_trans_heavy(trans_t tid) {

}

void rvm_abort_trans(trans_t tid) {

}

void rvm_truncate_log(rvm_t rvm) {

}

/*


int i = 5;
std::string s;
std::stringstream out;
out << i;
s = out.str();
*/

int write_log(rvm_t rvm, logitem log)
{
	string tolog = "";
	std::stringstream out;
	out << log.id << "~~" << log.tid << "~~" << log.segment_index << "~~" << log.offset << "~~" << log.size << "~~" << log.data<<"\n";
	//sprintf(tolog.c_str(), "%d~~%d~~%d~~%d~~%d~~%s\n", log.id, log.tid, log.segment_index, log.offset, log.size, log.data);
	if(rvm->flog != NULL)
	{
		fprintf(rvm->flog, out.str().c_str());
	}
	return 0;
}

/*
 *  if needed. to parse the log file and convert that into log vector
 */
int read_log(rvm_t rvm, vector <logitem> log)
{
	return 0;
}

int TRACE(rvm_t rvm,  const char * trace_stm)
{
	if(rvm->ftrace != NULL)
	{
		fprintf(rvm->ftrace, trace_stm);
	}
	return 0;
}


/*
 int main(int argc, char **argv) {
 puts("!!!Hello World!!!");
 return EXIT_SUCCESS;
 }
 */
