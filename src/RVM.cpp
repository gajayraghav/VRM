#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/rvm.h"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
using namespace std;

int toScreen;
int transactionCount = 0;
int global_count = 00;
FILE * fTracePtr = NULL;

map<int, trans_s> transactionGlobal;

/*
 * rvm_t rvm_init(const char *directory) - Initialize the library with the specified directory as backing store.
 */

rvm_t rvm_init(const char *directory) {
	rvm_t rvm = new rvm_details;
	struct stat st = { 0 };
	TRACE("<<<<<\t rvm_init");
	toScreen = 0;
	write_to_tracefile();
	int truncate_flag = 0;
	if (stat(directory, &st) == -1) {

		mkdir(directory, 0766);
		rvm->backingStore = directory;
		rvm->backingStore.append("/");
		rvm->storage_size = 0;
		rvm->memSeg_count = 0;
		rvm->flog = fopen(
				(rvm->backingStore + string("transaction.log")).c_str(), "w");
		fTracePtr = fopen((rvm->backingStore + string("trace.log")).c_str(),
				"w");
		TRACE("rvm_init: " + rvm->backingStore + string("trace.log").c_str());
		for (int i = 0; i < MAX_SEGMENTS; i++) {
			rvm->memSegs[i] = (memSeg*) malloc(sizeof(memSeg));
			rvm->memSegs[i]->mapped = 0;
			rvm->memSegs[i]->dirty = 0;
			//rvm->memSegs[i]->segName = " " ;
			rvm->memSegs[i]->Segmentsize = 0;
			rvm->memSegs[i]->fsegment = NULL;
		}
	} else {
		TRACE("rvm_init: Directory already exists");
		// ajay printf("\n rvm_init: Directory already exists \n");
		rvm->backingStore = directory;
		rvm->backingStore.append("/");
		rvm->storage_size = 0;
		rvm->memSeg_count = 0;
		rvm->flog = fopen(
				(rvm->backingStore + string("transaction.log")).c_str(), "r+");
		fTracePtr = fopen((rvm->backingStore + string("trace.log")).c_str(),
				"w");
		//printf("\n %s",(rvm->backingStore+string("trace.log")).c_str() );
		fseek(rvm->flog, 0, SEEK_END);
		int size = ftell(rvm->flog);
		fseek(rvm->flog, 0, SEEK_SET);
		if (size > 0)
			truncate_flag = 1;
		else
			truncate_flag = 0;

		for (int i = 0; i < MAX_SEGMENTS; i++) {
			rvm->memSegs[i] = (memSeg*) malloc(sizeof(memSeg));
			rvm->memSegs[i]->mapped = 0;
			rvm->memSegs[i]->dirty = 0;
			//rvm->memSegs[i]->segName = " " ;
			rvm->memSegs[i]->Segmentsize = 0;
			rvm->memSegs[i]->fsegment = NULL;
		}
		DIR *dir;
		struct dirent *ent;
		dir = opendir(directory);
		if (dir != NULL) {
			while ((ent = readdir(dir)) != NULL) {
				string fname = string(ent->d_name);
				int len = fname.length();

				if ((ent->d_name[len - 1] == 't') && (ent->d_name[len - 2]
						== 'x') && (ent->d_name[len - 3] == 't')
						&& (ent->d_name[len - 4] == '.')) {
					string sub = fname.substr(0, len - 4);
					struct stat st1;
					stat((rvm->backingStore + fname).c_str(), &st1);
					int size = st1.st_size;
					char size_str[10];
					sprintf(size_str, "%d", size);
					TRACE(string("rvm_init: found") + ent->d_name + string(
							" of size ") + size_str + string(" bytes"));
					//ajay			printf("\n rvm:init found %s of size %d bytes\n", ent->d_name, size);
					if (rvm->memSeg_count != MAX_SEGMENTS) {
						rvm->memSegs[rvm->memSeg_count]->fsegment = fopen(
								(rvm->backingStore + string(sub.c_str())
										+ ".txt").c_str(), "r+");
						if (rvm->memSegs[rvm->memSeg_count]->fsegment != NULL) {
							strcpy(rvm->memSegs[rvm->memSeg_count]->segName,
									sub.c_str());
							rvm->memSegs[rvm->memSeg_count]->dirty = 0;
							rvm->memSegs[rvm->memSeg_count]->transaction = 0;

							rvm->memSegs[rvm->memSeg_count]->Segmentsize = size;
							rvm->storage_size += size;
							rvm->memSegs[rvm->memSeg_count]->mapped = 0;
							rvm->memSegs[rvm->memSeg_count]->segAddr
									= (char *) malloc(size);
							memset(rvm->memSegs[rvm->memSeg_count]->segAddr, 0,
									size);
							TRACE(string("rvm_init: recreated segment ")
									+ rvm->memSegs[rvm->memSeg_count]->segName
									+ string(" from disk"));
							rvm->memSeg_count = rvm->memSeg_count + 1;
						}
					}
				}
			}
			closedir(dir);
		}
	}
	if (rvm->flog == NULL) {
		TRACE("rvm_init: Unable to create the log/trace file");
		TRACE("rvm_init \t >>>>>");
		perror("rvm_init: Unable to create the log/trace file");
		return NULL;
	} else {
		TRACE("created the log & trace file");
		TRACE("rvm_init \t >>>>>");
		// ajay printf("\n created the log & trace file \n");
		if (truncate_flag != 0) {
			rvm_truncate_log(rvm);
			fclose(fTracePtr);
			fTracePtr = fopen(
					(rvm->backingStore + string("trace.log")).c_str(), "w");
			if (fTracePtr == NULL) {
				TRACE("truncating trace file failed");
			}
		}
		return rvm;
	}
}

/*
 * void *rvm_map(rvm_t rvm, const char *segname, int size_to_create) - map a segment from disk into memory.
 * If the segment does not already exist, then create it and give it size size_to_create. If the segment
 * exists but is shorter than size_to_create, then extend it until it is long enough. It is an error to try
 * to map the same segment twice.
 */

void* rvm_map(rvm_t rvm, const char * segname, int size_to_create) {
	TRACE("<<<<< \t rvm_map");
	// find if the segment already exists.
	int segment_index = 0;
	bool found = false;

	for (segment_index = 0; segment_index < rvm->memSeg_count; segment_index++) {
		if (!strcmp(rvm->memSegs[segment_index]->segName, segname)) {
			found = true;
			if (rvm->memSegs[segment_index]->mapped == 1) {
				TRACE("rvm_map: abort - Segment Already Mapped");
				TRACE("rvm_map \t >>>>>");
				perror("\n abort - Already Mapped\n");
				//abort();
				return (void*) 0;
			}
			break;
		}
	}
	if (found == true) {
		TRACE(string("rvm_map: found ") + rvm->memSegs[segment_index]->segName
				+ string(" segment"));
		// if found, check the size and do some alterations.
		if (rvm->memSegs[segment_index]->Segmentsize < size_to_create) {
			TRACE("rvm_map: size less than requested");
			// increase the size of the segment
			rvm->storage_size -= rvm->memSegs[segment_index]->Segmentsize;

			rvm->memSegs[segment_index]->mapped = 1;
			char *tempPtr;
			tempPtr = (char *) realloc(rvm->memSegs[segment_index]->segAddr,
					size_to_create);
			if (tempPtr == NULL) {
				return (void *) 0;
			}
			rvm->memSegs[segment_index]->segAddr = tempPtr;
			char *tempFill = (char *) malloc(sizeof(char) * (size_to_create
					- rvm->memSegs[segment_index]->Segmentsize));
			memset(tempFill, 0, (size_to_create
					- rvm->memSegs[segment_index]->Segmentsize));
			char tmp_string1[20];
			char tmp_string2[20];
			char tmp_string3[20];
			sprintf(tmp_string1, "%d", rvm->memSegs[segment_index]->Segmentsize);
			sprintf(tmp_string2, "%d", size_to_create);
			sprintf(tmp_string3, "%d", (size_to_create
					- rvm->memSegs[segment_index]->Segmentsize));

			TRACE("segsize " + string(tmp_string1) + " size_to_create "
					+ string(tmp_string2) + " diff " + string(tmp_string3));
			fseek(rvm->memSegs[segment_index]->fsegment, 0, SEEK_END);
			fwrite(tempFill, 1, size_to_create
					- rvm->memSegs[segment_index]->Segmentsize,
					rvm->memSegs[segment_index]->fsegment);
			rewind(rvm->memSegs[segment_index]->fsegment);
			for (int i = rvm->memSegs[segment_index]->Segmentsize; i
					< size_to_create; i++) {
				rvm->memSegs[segment_index]->segAddr[i] = 0;

			}
			rvm->memSegs[segment_index]->Segmentsize = size_to_create;
			rvm->storage_size += rvm->memSegs[segment_index]->Segmentsize;
		} else if (rvm->memSegs[segment_index]->Segmentsize > size_to_create) {
			TRACE("File size greater than the requested. aborting....");
			TRACE("rvm_map \t >>>>>");
			perror("\n File size greater than the requested. aborting.... \n");
			abort();
		} else {
			rvm->memSegs[segment_index]->mapped = 1;
		}
		int sizeread = fread(rvm->memSegs[segment_index]->segAddr, 1,
				size_to_create, rvm->memSegs[segment_index]->fsegment);
		rewind(rvm->memSegs[segment_index]->fsegment);
		TRACE("rvm_map: returning segment");
		TRACE("rvm_map \t >>>>>");
		return ((void*) rvm->memSegs[segment_index]->segAddr);
	} else {
		// Create new segment
		TRACE("rvm_map: create a new segment ");
		if (rvm->memSeg_count != MAX_SEGMENTS) {
			rvm->memSegs[rvm->memSeg_count]->fsegment = fopen(
					(rvm->backingStore + string(segname) + ".txt").c_str(),
					"w+x");
			if (rvm->memSegs[rvm->memSeg_count]->fsegment != NULL) {
				rewind(rvm->memSegs[rvm->memSeg_count]->fsegment);
				strcpy(rvm->memSegs[rvm->memSeg_count]->segName, segname);
				rvm->memSegs[rvm->memSeg_count]->dirty = 0;
				rvm->memSegs[rvm->memSeg_count]->Segmentsize = size_to_create;
				rvm->storage_size += size_to_create;
				rvm->memSegs[rvm->memSeg_count]->mapped = 1;
				char tmp_str[10];
				sprintf(tmp_str, "%d", rvm->storage_size);
				TRACE("rvm_map: storage size - " + string(tmp_str));
				rvm->memSegs[rvm->memSeg_count]->segAddr = (char *) malloc(
						size_to_create);
				memset(rvm->memSegs[rvm->memSeg_count]->segAddr, 0,
						size_to_create);
				fwrite(rvm->memSegs[rvm->memSeg_count]->segAddr, 1,
						size_to_create,
						rvm->memSegs[rvm->memSeg_count]->fsegment);
				rvm->memSeg_count = rvm->memSeg_count + 1;
				sprintf(tmp_str, "%d", rvm->memSeg_count);
				TRACE(" rvm_map: created a new segment " + string(tmp_str));
				// ajay			printf("\n created a new segment %d \n", rvm->memSeg_count);
				TRACE("rvm_map \t >>>>> 1 ");
				return ((void*) rvm->memSegs[rvm->memSeg_count - 1]->segAddr);
			}
		}
	}
	TRACE("rvm_map: Returning void* 0 ");
	// ajay printf("\n rvm_map: returning void* 0 \n");
	TRACE("rvm_map \t >>>>> 2 ");

	return (void*) 0;
}

/*
 * void rvm_unmap(rvm_t rvm, void *segbase) - unmap a segment from memory.
 */
void rvm_unmap(rvm_t rvm, void *segbase) {
	TRACE("<<<<< \t rvm_unmap");
	int segment_index = 0;
	bool found = false;
	for (segment_index = 0; segment_index < rvm->memSeg_count; segment_index++) {
		if (!strcmp(rvm->memSegs[segment_index]->segAddr, (char *) segbase)) {
			TRACE(string(" rvm_unmap: found ")
					+ rvm->memSegs[segment_index]->segAddr
					+ string(" to unmap"));
			found = true;
			break;
		}
	}
	if (found == true) {
		if (rvm->memSegs[segment_index]->dirty > 0) {
			TRACE(
					" rvm_unmap: cant unmap a dirty segment. Need to commit/abort before unmap");
			TRACE(" rvm_unmap \t >>>>>");
			perror(
					"\n cant unmap a dirty segment. Need to commit/abort before unmap");
		} else {
			// ajay printf("\n need to unmap segment %s",rvm->memSegs[segment_index]->segName);
			TRACE(string(" need to unmap segment ")
					+ rvm->memSegs[segment_index]->segName);
			rvm->memSegs[segment_index]->mapped = 0;
			//fclose(rvm->memSegs[segment_index]->fsegment);
			//rvm->memSegs[segment_index]->fsegment = NULL;
			rvm->memSegs[segment_index]->segAddr = NULL;
			char tmp_str[20];
			sprintf(tmp_str, "%d", rvm->storage_size);
			TRACE("storage size - " + string(tmp_str));
			// need something related to the segAddr. make it null ?
		}
	} else {
		TRACE("rvm_unmap \t >>>>>");
		perror("\n cant unmap a non existing segment");
	}
	TRACE("rvm_unmap \t >>>>>");
}

/*
 * void rvm_destroy(rvm_t rvm, const char *segname) - destroy a segment completely, erasing its backing store.
 * This function should not be called on a segment that is currently mapped.
 */

void rvm_destroy(rvm_t rvm, const char *segname) {
	TRACE("<<<<< \t rvm_destroy");
	int segment_index = 0;
	bool found = false;
	for (segment_index = 0; segment_index < rvm->memSeg_count; segment_index++) {
		if (!strcmp(rvm->memSegs[segment_index]->segName, segname)) {
			// ajay printf("\n found %s to destroy\n", segname);
			TRACE(string("rvm_destroy: found ")
					+ rvm->memSegs[segment_index]->segName + string(
					" to destroy"));
			found = true;
			break;
		}
	}
	if (found == true) {
		if (rvm->memSegs[segment_index]->mapped == 0) {
			rvm->storage_size -= rvm->memSegs[segment_index]->Segmentsize;
			rvm->memSegs[segment_index]->mapped = 0;
			if (rvm->memSegs[segment_index]->fsegment != NULL) {
				TRACE("File is open elsewhere");
				fclose(rvm->memSegs[segment_index]->fsegment);
			}
			rvm->memSegs[segment_index]->fsegment = NULL;
			rvm->memSegs[segment_index]->dirty = 0;
			rvm->memSegs[segment_index]->Segmentsize = 0;
			rvm->memSeg_count--;
			unlink((rvm->backingStore + string(segname) + ".txt").c_str());
			memset(rvm->memSegs[segment_index]->segName, 0, 20);
			char tmp_str[20];
			sprintf(tmp_str, "%d", rvm->memSeg_count);
			TRACE("\n destroyed segment " + string(tmp_str));
			// ajay printf("\nrvm_destroy: destroyed segment, %d \n", (int) rvm->memSeg_count);
		} else {
			TRACE(
					"rvm_destroy: cant destroy a mapped segment. Need to unmap before destroy");
			TRACE("rvm_destroy \t >>>>>");
			perror(
					"\n cant destroy a mapped segment. Need to unmap before destroy");
		}
	} else {
		TRACE("cant destroy a non existing segment");
		TRACE("rvm_destroy \t >>>>>");
		//	perror("\n cant destroy a non existing segment");
	}
}

trans_t rvm_begin_trans(rvm_t rvm, int segcount, void **segbases) {
	TRACE("<<<< rvm_begin_trans");
	trans_s newTrans = new transactions;
	newTrans->rvm = rvm;
	int SegsFound = 0, counter;
	int index = 0;
	char tmp_string[10];
	sprintf(tmp_string, "%d", segcount);
	TRACE("rvm_begin_trans: beginning transaction with segcount " + string(
			tmp_string));
	transactionCount++;
	newTrans->transactionId = transactionCount;
	while (rvm->memSegs[index] != rvm->memSegs[MAX_SEGMENTS - 1]) {
		for (counter = 0; counter < segcount; counter++) {//for each segbase
			if (*(segbases + counter) == rvm->memSegs[index]->segAddr) {
				//if the same, then lock it
				TRACE(string("rvm_begin_trans: found segment ")
						+ rvm->memSegs[index]->segName);
				rvm->memSegs[index]->dirty = 0;
				newTrans->segBases[*(segbases + counter)] = rvm->memSegs[index];
				if (rvm->memSegs[index]->transaction == 0) {
					rvm->memSegs[index]->transaction = newTrans->transactionId;
					SegsFound++;
				}
			}
		}
		index++;
	}
	transactionGlobal[transactionCount] = newTrans;
	if (SegsFound != segcount) {
		TRACE("rvm_begin_trans: could not lock all required memory segments");
		TRACE("rvm_begin_trans \t >>>>>");
		// ajay printf("could not find all the required memory segments");
		transactionCount--;
		return (trans_t) -1;
	}
	TRACE("rvm_begin_trans \t >>>>>");
	return newTrans->transactionId;
}

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size) {
	TRACE("<<<<< \t rvm_about_to_modify");
	trans_s activeTransaction = transactionGlobal[tid];
	memSeg * currmSeg;
	char *tempAboutToModify;
	transItem *newtransItem = new transItem;
	if (activeTransaction->segBases.find(segbase)
			== activeTransaction->segBases.end()) {
		TRACE("rvm_about_to_modify: couldn't find the seg base. aborting... ");
		TRACE("rvm_about_to_modify \t <<<<<");
		// ajay printf("\n couldn't find the seg base \n");
		abort();
	}
	TRACE(string("rvm_about_to_modify: found ")
			+ activeTransaction->segBases[segbase]->segName);
	//specify that the segment is in a transaction
	currmSeg = activeTransaction->segBases[segbase];
	newtransItem->segmentinProcess = currmSeg;
	newtransItem->transactionSize = size;
	newtransItem->offset = offset;
	currmSeg->dirty++;
	tempAboutToModify = (char *) malloc((size));
	memset(tempAboutToModify, 0, size);
	memcpy(tempAboutToModify, currmSeg->segAddr + offset, size);
	newtransItem->aboutToModify = (char *) malloc((size));
	memcpy(newtransItem->aboutToModify, currmSeg->segAddr + offset, size);
	newtransItem->segmentinProcess->logItem.offset = newtransItem->offset;
	newtransItem->segmentinProcess->logItem.size
			= newtransItem->transactionSize;
	newtransItem->segmentinProcess->logItem.segName
			= newtransItem->segmentinProcess->segName;
	newtransItem->segmentinProcess->logItem.data
			= newtransItem->segmentinProcess->segAddr + newtransItem->offset;
	newtransItem->transactionType = ABOUT_TO_MODIFY;
	if (activeTransaction->rvm->flog != NULL) {
		if (fprintf(activeTransaction->rvm->flog, "about_to_modify/%s/%d/%d/",
				currmSeg->segName, offset, size) < 0) {
			TRACE("rvm_about_to_modify: fprintf error");
			TRACE("rvm_about_to_modify \t >>>>>");
			perror("fprintf error");
			abort();
		}
		fwrite((void*) ((char*) currmSeg->segAddr + newtransItem->offset), 1,
				newtransItem->transactionSize, activeTransaction->rvm->flog);
		fprintf(activeTransaction->rvm->flog, "\n");
		activeTransaction->action.push_back(newtransItem);
	} else {
		TRACE("rvm_about_to_modify: Log is null");
		TRACE("rvm_about_to_modify \t >>>>>");
		perror("rvm_about_to_modify: Log is null");
	}
	TRACE("rvm_about_to_modify \t >>>>>");
	return;

}

void rvm_commit_trans(trans_t tid) {
	TRACE("<<<<< \t rvm_commit_trans");
	trans_s activeTransaction = transactionGlobal[tid];
	vector<transItem *>::iterator it = activeTransaction->action.begin();

	while (it != activeTransaction->action.end()) {
		if ((*it)->segmentinProcess->dirty > 0) {
			TRACE(string("rvm_commit_trans: write commit log for ")
					+ (*it)->segmentinProcess->segName);
			// ajay printf("\n write commit log for %s \n",(*it)->segmentinProcess->segName );
			if (activeTransaction->rvm->flog != NULL) {
				/*(*it)->segmentinProcess->logItem.offset  = (*it)->offset;
				 (*it)->segmentinProcess->logItem.size = (*it)->transactionSize;
				 (*it)->segmentinProcess->logItem.segName= (*it)->segmentinProcess->segName;
				 (*it)->segmentinProcess->logItem.data = (*it)->segmentinProcess->segAddr+(*it)->offset;
				 */(*it)->transactionType = COMMIT;
				if (fprintf(activeTransaction->rvm->flog, "commit/%s/%d/%d/",
						(*it)->segmentinProcess->segName, (*it)->offset,
						(*it)->transactionSize) < 0) {
					TRACE("rvm_commit_trans: failure to log");
					TRACE("rvm_commit_trans \t >>>>>");
					// ajay printf("\n rvm_commit_trans: failure to log \n");
					abort();
				}
			} else {
				TRACE("rvm_commit_trans: log is null.. abort...");
				TRACE("rvm_commit_trans \t >>>>>");
				abort();
			}
			fwrite((void*) ((char*) (*it)->segmentinProcess->segAddr
					+ (*it)->offset), 1, (*it)->transactionSize,
					activeTransaction->rvm->flog);
			fprintf(activeTransaction->rvm->flog, "\n");
			(*it)->segmentinProcess->dirty--;
			if ((*it)->segmentinProcess->dirty == 0) {
				(*it)->segmentinProcess->transaction = 0;
			}
			/*		rewind((*it)->segmentinProcess->fsegment);
			 lseek(fileno((*it)->segmentinProcess->fsegment), (*it)->offset, SEEK_CUR);
			 fwrite((*it)->segmentinProcess->segAddr + (*it)->offset,
			 1,
			 (*it)->transactionSize,
			 (*it)->segmentinProcess->fsegment);
			 */
			(*it)->aboutToModify = NULL;
		}
		delete *it;
		it = activeTransaction->action.erase(it);
	}
	//	rvm_truncate_log(activeTransaction->rvm);
	TRACE("rvm_commit_trans \t >>>>>");
	return;
}

void rvm_commit_trans_heavy(trans_t tid) {
	TRACE("<<<<< \t rvm_commit_trans_heavy");
	trans_s activeTransaction = transactionGlobal[tid];
	vector<transItem *>::iterator it = activeTransaction->action.begin();

	while (it != activeTransaction->action.end()) {
		if ((*it)->segmentinProcess->dirty > 0) {
			//		TRACE(string("rvm_commit_trans: write commit log for ") +(*it)->segmentinProcess->segName );
			// ajay printf("\n write commit log for %s \n",(*it)->segmentinProcess->segName );
			/*
			 if (activeTransaction->rvm->flog != NULL)
			 {
			 (*it)->segmentinProcess->logItem.offset  = (*it)->offset;
			 (*it)->segmentinProcess->logItem.size = (*it)->transactionSize;
			 (*it)->segmentinProcess->logItem.segName= (*it)->segmentinProcess->segName;
			 (*it)->segmentinProcess->logItem.data = (*it)->segmentinProcess->segAddr+(*it)->offset;
			 (*it)->transactionType = COMMIT;
			 if (fprintf(activeTransaction->rvm->flog, "commit/%s/%d/%d/", (*it)->segmentinProcess->segName, (*it)->offset, (*it)->transactionSize ) < 0){
			 TRACE("rvm_commit_trans: failure to log");
			 TRACE("rvm_commit_trans \t >>>>>");
			 // ajay printf("\n rvm_commit_trans: failure to log \n");
			 abort();
			 }
			 }
			 else
			 {
			 TRACE("rvm_commit_trans: log is null.. abort...");
			 TRACE("rvm_commit_trans \t >>>>>");
			 abort();
			 }
			 */
			/*
			 fwrite((void*)((char*)(*it)->segmentinProcess->segAddr + (*it)->offset),
			 1,
			 (*it)->transactionSize,
			 activeTransaction->rvm->flog);
			 fprintf(activeTransaction->rvm->flog, "\n");
			 */
			(*it)->segmentinProcess->dirty--;
			if ((*it)->segmentinProcess->dirty == 0) {
				(*it)->segmentinProcess->transaction = 0;
			}
			rewind((*it)->segmentinProcess->fsegment);
			lseek(fileno((*it)->segmentinProcess->fsegment), (*it)->offset,
					SEEK_CUR);
			fwrite((*it)->segmentinProcess->segAddr + (*it)->offset, 1,
					(*it)->transactionSize, (*it)->segmentinProcess->fsegment);

			(*it)->aboutToModify = NULL;
		}
		delete *it;
		it = activeTransaction->action.erase(it);
	}
	//	rvm_truncate_log(activeTransaction->rvm);
	TRACE("rvm_commit_trans_heavy \t >>>>>");
	return;

}

void rvm_abort_trans(trans_t tid) {
	TRACE("<<<<< \t rvm_abort_trans");
	trans_s activeTransaction = transactionGlobal[tid];
	map<void *, memSeg*>::iterator segBases =
			activeTransaction->segBases.begin();
	vector<transItem *>::iterator it = activeTransaction->action.begin();
	while (it != activeTransaction->action.end()) {
		if ((*it)->segmentinProcess->dirty > 0) {
			fprintf(activeTransaction->rvm->flog, "abort/%s/%d/%d/",
					(*it)->segmentinProcess->segName, (*it)->offset,
					(*it)->transactionSize);
			(*it)->transactionType = ABORT;
			fwrite((void*) ((char*) (*it)->segmentinProcess->segAddr
					+ (*it)->offset), 1, (*it)->transactionSize,
					activeTransaction->rvm->flog);
			fprintf(activeTransaction->rvm->flog, "\n");
			TRACE(string("rvm_abort_trans: bkp-") + (*it)->aboutToModify);
			// ajay printf("bkp - %s\n", (*it)->aboutToModify);
			memcpy((*it)->segmentinProcess->segAddr + (*it)->offset,
					(*it)->aboutToModify, (*it)->transactionSize);
			(*it)->segmentinProcess->dirty--;
			if ((*it)->segmentinProcess->dirty == 0) {
				(*it)->segmentinProcess->transaction = 0;
			}
		}

		delete *it;
		activeTransaction->action.erase(it);
	}
	TRACE("rvm_abort_trans \t >>>>>");

}

void rvm_truncate_log(rvm_t rvm) {
	TRACE("<<<<< \t rvm_truncate_log");
	FILE *fp = NULL;
	FILE *flog = NULL;
	char *line = NULL;
	char *token = NULL;
	char *data = NULL;
	char *segName = NULL;
	char command[200];
	string linebkp;
	size_t len = 0;
	size_t read = 0;
	int commit_counter = 0;
	int abort_counter = 0;
	int offset = 0;
	int size = 0;
	int tid = 0;
	int newindex = 0;
	int index = 0;

	flog = fopen((rvm->backingStore + string("temp.log")).c_str(), "w");
	while ((read = getline(&line, &len, rvm->flog)) != -1) {
		newindex = 0;
		logitem newLogItem;
		linebkp = "";
		memcpy((char *) linebkp.c_str(), line, read);
		token = strtok(line, "/");
		if (strcmp(token, "commit") == 0) {
			tid = COMMIT;
			segName = strtok(NULL, "/");
			offset = atoi(strtok(NULL, "/"));
			size = atoi(strtok(NULL, "/"));
			data = strtok(NULL, "/");
			commit_counter++;
			for (newindex = 0; newindex < rvm->memSeg_count; newindex++) {
				if (!strcmp(segName, rvm->memSegs[newindex]->segName)) {
					fp = rvm->memSegs[newindex]->fsegment;
					break;
				}
			}
			rewind(rvm->memSegs[newindex]->fsegment);
			lseek(fileno(rvm->memSegs[newindex]->fsegment), offset, SEEK_CUR);
			fwrite(data, 1, size, rvm->memSegs[newindex]->fsegment);
			rewind(rvm->memSegs[newindex]->fsegment);
		} else if (strcmp(token, "abort") == 0) {
			fwrite(linebkp.c_str(), 1, read, flog);
			abort_counter++;
		}
	}
	fclose(flog);
	memset(command, 0, 200);
	sprintf(command, "mv %s %s",
			(rvm->backingStore + string("temp.log")).c_str(),
			(rvm->backingStore + string("transaction.log")).c_str());
	system(command);
	flog = NULL;
	flog = fopen((rvm->backingStore + string("transaction.log")).c_str(), "r+");
	rvm->flog = flog;
	if (rvm->flog == NULL) {
		TRACE("rvm_truncate_log: reopening log file failed");
		printf("\n reopening log file failed\n");
		TRACE("rvm_truncate_log \t >>>>> 1");
		abort();
	}
	TRACE("rvm_truncate_log \t >>>>> 2");
}

int TRACE(string trace_stm) {
	if (toScreen == 1) {
		printf("\n-> %s\n", trace_stm.c_str());
	} else {
		if (fTracePtr != NULL) {
			fprintf(fTracePtr, "\n %s \n", trace_stm.c_str());
		}
	}
	return 0;
}

void write_to_screen() {
	toScreen = 1;
}

void write_to_tracefile() {
	toScreen = 0;
}
