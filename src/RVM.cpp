
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

///// git

/*
 * rvm_t rvm_init(const char *directory) - Initialize the library with the specified directory as backing store.
 */


int debugTraceFlag;
static void debugTrace(string output){
	if (debugTraceFlag){
		fprintf(stdout, "%s\n", output.c_str());
	}
}

rvm_t rvm_init(const char *directory)
{
	rvm_t rvm = new rvm_details;
	struct stat st = {0};

	if (stat(directory, &st) == -1) {

		mkdir(directory, 0766);
		rvm->backingStore = directory;
		rvm->backingStore.append("/");
		rvm->storage_size = 0;
		rvm->memSeg_count = 0;
		rvm->flog = fopen((rvm->backingStore+string("transaction.log")).c_str(), "w");
		rvm->ftrace = fopen((rvm->backingStore+string("trace.log")).c_str(), "w");
		printf("\n %s",(rvm->backingStore+string("trace.log")).c_str() );
		for (int i =0; i< MAX_SEGMENTS; i++)
		{
			rvm->memSegs[i] = (memSeg*)malloc(sizeof(memSeg));
			rvm->memSegs[i]->mapped = 0;
			rvm->memSegs[i]->dirty = 0;
			//rvm->memSegs[i]->segName = " " ;
			rvm->memSegs[i]->Segmentsize = 0 ;
			rvm->memSegs[i]->fsegment = NULL;
		}
	}
	else
	{
		printf("\n rvm_init: Directory already exists \n");
		rvm->backingStore = directory;
		rvm->backingStore.append("/");
		rvm->storage_size = 0;
		rvm->memSeg_count = 0;
		rvm->flog = fopen((rvm->backingStore+string("transaction.log")).c_str(), "w");
		rvm->ftrace = fopen((rvm->backingStore+string("trace.log")).c_str(), "w");
		//printf("\n %s",(rvm->backingStore+string("trace.log")).c_str() );
		for (int i =0; i< MAX_SEGMENTS; i++)
		{
			rvm->memSegs[i] = (memSeg*)malloc(sizeof(memSeg));
			rvm->memSegs[i]->mapped = 0;
			rvm->memSegs[i]->dirty = 0;
			//rvm->memSegs[i]->segName = " " ;
			rvm->memSegs[i]->Segmentsize = 0 ;
			rvm->memSegs[i]->fsegment = NULL;
		}
		DIR *dir;
		struct dirent *ent;
		dir = opendir(directory);
		if (dir != NULL)
		{
			while ((ent = readdir (dir)) != NULL)
			{
				string fname = string(ent->d_name);
				int len = fname.length();
				if((ent->d_name[len-1] == 't')&&(ent->d_name[len-2] == 'x')&&(ent->d_name[len-3] == 't')&&(ent->d_name[len-4] == '.'))
				{
					printf ("%s\n", ent->d_name);
					string sub = fname.substr(0, len-4);
					struct stat st1;
					stat((rvm->backingStore+fname).c_str(), &st1);
					int size = st1.st_size;
					printf("\n File Length - of %s is %d",sub.c_str(), size);
/*
					rvm_map(rvm, sub.c_str(), size );
					rvm->memSegs[rvm->memSeg_count-1]->mapped=0;
*/
					if (rvm->memSeg_count != MAX_SEGMENTS)
					{
						rvm->memSegs[rvm->memSeg_count]->fsegment = fopen((rvm->backingStore + string(sub.c_str())+".txt").c_str(), "r+");
						if(rvm->memSegs[rvm->memSeg_count]->fsegment != NULL)
						{
							//rewind(rvm->memSegs[rvm->memSeg_count]->fsegment);
							strcpy(rvm->memSegs[rvm->memSeg_count]->segName, sub.c_str());
							rvm->memSegs[rvm->memSeg_count]->dirty = 0;
							rvm->memSegs[rvm->memSeg_count]->Segmentsize = size;
							rvm->storage_size+=size;
							rvm->memSegs[rvm->memSeg_count]->mapped = 0;
							rvm->memSegs[rvm->memSeg_count]->segAddr = (char *) malloc (size+1);
							memset (rvm->memSegs[rvm->memSeg_count]->segAddr, 0, size);
							//fwrite(rvm->memSegs[rvm->memSeg_count]->segAddr, 1, size_to_create, rvm->memSegs[rvm->memSeg_count]->fsegment);
							//rvm->memSegs[rvm->memSeg_count]->segAddr[size_to_create] = '\0';
							rvm->memSeg_count = rvm->memSeg_count + 1;
							printf("\n created a new segment %d \n", rvm->memSeg_count);
						}
					}
				}
			}
			closedir (dir);
		}
		//		rvm->backingStore = "dirExistS"; // E and S in cap to avoid case when user creates direxists as a dir input
	}
	if ((rvm->flog == NULL)|| (rvm->ftrace == NULL))
	{
		perror("rvm_init: Unable to create the log/trace file");
		return NULL;
	}
	else
	{
		TRACE(rvm, ">>>Init<<<\n");
		printf("\n created the log & trace file \n");
		return rvm;
	}


}
/*
 * void *rvm_map(rvm_t rvm, const char *segname, int size_to_create) - map a segment from disk into memory.
 * If the segment does not already exist, then create it and give it size size_to_create. If the segment
 * exists but is shorter than size_to_create, then extend it until it is long enough. It is an error to try
 * to map the same segment twice.
 */

void* rvm_map(rvm_t rvm, const char * segname, int size_to_create)
{
	TRACE(rvm, ">>>>> rvm_map\n");
	printf("\n in map \n");
	// find if the segment already exists.
	int segment_index=0;
	bool found = false;

	for (segment_index=0;segment_index<rvm->memSeg_count;segment_index++)
	{
		if(!strcmp(rvm->memSegs[segment_index]->segName, segname))
		{
			found = true;
			if(rvm->memSegs[segment_index]->mapped == 1)
			{
				perror("\n abort - Already Mapped\n");
				//abort();
				return (void*) 0;
			}
			break;
		}
	}
	if(found == true)
	{
		printf("\n found %s segment", rvm->memSegs[segment_index]->segName);
		/*
		 *  if found, check the size and do some alterations.
		 */
		if (rvm->memSegs[segment_index]->Segmentsize < size_to_create)
		{
			printf("\n size less than requested \n");
			// increase the size of the segment
			rvm->storage_size-=rvm->memSegs[segment_index]->Segmentsize;

			rvm->memSegs[segment_index]->mapped = 1;
			rvm->memSegs[segment_index]->segAddr = (char *) realloc(rvm->memSegs[segment_index]->segAddr, (size_to_create - rvm->memSegs[segment_index]->Segmentsize));
			for (int i = rvm->memSegs[segment_index]->Segmentsize;i < size_to_create; i++)
			{
				rvm->memSegs[segment_index]->segAddr[i] = 0;
			}
			//rvm->memSegs[segment_index]->segAddr[size_to_create] = '\0';
			//int fd = fileno(rvm->memSegs[segment_index]->fsegment); // file descriptor
			rvm->memSegs[segment_index]->Segmentsize = size_to_create;
			rvm->storage_size+=rvm->memSegs[segment_index]->Segmentsize;
			printf("\n storage size - %ld \n", rvm->storage_size);

			//			int filesize =
			//		char *buf = (char *) malloc(filestream.Length);
			//	memset(buf, 0, filestream.Length);*/
		}
		else if (rvm->memSegs[segment_index]->Segmentsize > size_to_create)
		{
			//printf("\n size more than requested \n");
			// abort
			//printf("\n storage size - %ld \n", rvm->storage_size);
			perror("\n File size greater than the requested. aborting.... \n");
			abort();
		}
		else
		{
			// dont alter the size
			rvm->memSegs[segment_index]->mapped = 1;
			printf("\n storage size - %ld \n", rvm->storage_size);
		}
		int sizeread = fread(rvm->memSegs[segment_index]->segAddr, 1, size_to_create,rvm->memSegs[segment_index]->fsegment);
		printf("\n size read = %d", sizeread);
		rewind(rvm->memSegs[segment_index]->fsegment);
		return ((void*) rvm->memSegs[segment_index]->segAddr); //rvm->memSegs[segment_index]->fsegment);
		//rewind (rvm->memSegs[segment_index]->segAddr);
		printf("\n ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~%s~~~~~~~~~~~~~~~~~~~``",rvm->memSegs[segment_index]->segAddr);
	}
	else
	{		
		// Create new segment
		printf("\n create the new segment - %d \n", (int) rvm->memSeg_count );
		if (rvm->memSeg_count != MAX_SEGMENTS)
		{
			rvm->memSegs[rvm->memSeg_count]->fsegment = fopen((rvm->backingStore + string(segname)+".txt").c_str(), "w+x");
			if(rvm->memSegs[rvm->memSeg_count]->fsegment != NULL)
			{
				rewind(rvm->memSegs[rvm->memSeg_count]->fsegment);
				strcpy(rvm->memSegs[rvm->memSeg_count]->segName, segname);
				rvm->memSegs[rvm->memSeg_count]->dirty = 0;
				rvm->memSegs[rvm->memSeg_count]->Segmentsize = size_to_create;
				rvm->storage_size+=size_to_create;
				rvm->memSegs[rvm->memSeg_count]->mapped = 1;
				printf("\n storage size - %ld \n", rvm->storage_size);
				rvm->memSegs[rvm->memSeg_count]->segAddr = (char *) malloc (size_to_create+1);
				memset (rvm->memSegs[rvm->memSeg_count]->segAddr, 0, size_to_create);
				fwrite(rvm->memSegs[rvm->memSeg_count]->segAddr, 1, size_to_create, rvm->memSegs[rvm->memSeg_count]->fsegment);
				//rvm->memSegs[rvm->memSeg_count]->segAddr[size_to_create] = '\0';
				rvm->memSeg_count = rvm->memSeg_count + 1;
				printf("\n created a new segment %d \n", rvm->memSeg_count);
				return ((void*) rvm->memSegs[rvm->memSeg_count-1]->segAddr);
			}
/*
			else
			{
				printf("\n file open failure\n");
				perror((rvm->backingStore + string(segname)+".txt").c_str());
			}
*/
		}
	}
	return (void*) 0;
}

/*
 * void rvm_unmap(rvm_t rvm, void *segbase) - unmap a segment from memory.
 */
void rvm_unmap(rvm_t rvm, void *segbase)
{
	int segment_index=0;
	bool found = false;
	for (segment_index=0;segment_index<rvm->memSeg_count;segment_index++)
	{
		//		printf("\n comparing '%s' with '%s'", rvm->memSegs[segment_index]->segAddr,(char*)segbase);
		if(rvm->memSegs[segment_index]->segAddr == (char *)segbase)
		{
			printf("\n found a segment to unmap");
			found = true;
			break;
		}
	}
	if(found == true)
	{
		if(rvm->memSegs[segment_index]->dirty > 0)
		{
			perror("\n cant unmap a dirty segment. Need to commit/abort before unmap");
		}
		else
		{
			printf("\n need to unmap segment %s",rvm->memSegs[segment_index]->segName);
			rvm->memSegs[segment_index]->mapped = 0;
			fclose(rvm->memSegs[segment_index]->fsegment);
			rvm->memSegs[segment_index]->fsegment = NULL;
			rvm->memSegs[segment_index]->segAddr = NULL;
			printf("\n storage size - %ld \n", rvm->storage_size);
			// need something related to the segAddr. make it null ?
		}
	}
	else
	{
		perror("\n cant unmap a non existing segment");
	}
}

/*
 * void rvm_destroy(rvm_t rvm, const char *segname) - destroy a segment completely, erasing its backing store.
 * This function should not be called on a segment that is currently mapped.
 */

void rvm_destroy(rvm_t rvm, const char *segname)
{
	int segment_index = 0;
	bool found = false;
	for (segment_index = 0; segment_index < rvm->memSeg_count; segment_index++) {
		if (!strcmp(rvm->memSegs[segment_index]->segName, segname)) {
			found = true;
			break;
		}
	}
	if (found == true)
	{
		printf("\n found %s",rvm->memSegs[segment_index]->segName);
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

trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases)
{
	trans_t newTrans = new transactions;
	newTrans->rvm = rvm;
	int mSegsLocked= 0, counter;
	int index = 0;
	printf("\n in begin trans \n");
	debugTrace("beginning transaction with numsegs " + numsegs);
	while(rvm->memSegs[index] != rvm->memSegs[MAX_SEGMENTS-1]){
		for (counter = 0; counter < numsegs; counter++){//for each segbase
			if (*(segbases + counter) == rvm->memSegs[index]->segAddr){
				//if the same, then lock it
				printf("\n found segment %s",rvm->memSegs[index]->segName );
		/*		if (flock((int)fileno(rvm->memSegs[index]->fsegment), LOCK_EX || LOCK_NB) == -1){
					debugTrace("already locked " + (string)rvm->memSegs[index]->segName);
					return (trans_t) -1;
				}*/
				debugTrace("locking a mSeg");
				rvm->memSegs[index]->dirty = 0;
				newTrans->segBases[*(segbases + counter)] = rvm->memSegs[index];
				mSegsLocked++;
			}
		}
		index++;
	}
	if (mSegsLocked != numsegs){
		debugTrace("could not lock all required memory segments");
		return (trans_t)-1;
	}
	debugTrace("returning newTrans");
	printf("\n done begin trans \n");
	return newTrans; 
}

void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size)
{
	printf("\n in modify \n");
	memSeg * currmSeg;
	char *tempAboutToModify;
	transItem *newtransItem = new transItem;
	if (tid->segBases.find(segbase) == tid->segBases.end()){
		printf("\n couldn't find the seg base \n");
		debugTrace("specified segbase does not exist");
		abort();
	}
	printf("\n found %s\n", tid->segBases[segbase]->segName);
	//specify that the segment is in a transaction
	currmSeg = tid->segBases[segbase];
	newtransItem->segmentinProcess = currmSeg;
	newtransItem->transactionSize = size;
	newtransItem->offset = offset;
	currmSeg->dirty++;
	tempAboutToModify = (char *) malloc((size));
	memset(tempAboutToModify, 0, size);
	memcpy(tempAboutToModify, currmSeg->segAddr+offset, size);
	newtransItem->aboutToModify = (char *) malloc((size));
	memcpy(newtransItem->aboutToModify, currmSeg->segAddr+offset, size);
	printf("about_to_modify - %s\n", newtransItem->aboutToModify);
//	currmSeg->aboutToModify.insert(currmSeg->aboutToModify.end(), (string)tempAboutToModify);
	if (fprintf(tid->rvm->flog, "about_to_modify/%s/%d/%d/", currmSeg->segName,offset, size) < 0){
		perror("fprintf error");
		abort();
	}
	fwrite((void*)((char*)currmSeg->segAddr + newtransItem->offset),
                                        1,
                                        newtransItem->transactionSize,
                                        tid->rvm->flog);
        fprintf(tid->rvm->flog, "\n"); 
	tid->action.push_back(newtransItem);
       
	return;

}

void rvm_commit_trans(trans_t tid)
{	vector<transItem *>::iterator it = tid->action.begin();

	while (it != tid->action.end()){
		if ((*it)->segmentinProcess->dirty > 0){
			if (tid->rvm->flog != NULL)
			{
				(*it)->segmentinProcess->logItem.offset  = (*it)->offset;
				(*it)->segmentinProcess->logItem.size = (*it)->transactionSize;
				(*it)->segmentinProcess->logItem.segName= (*it)->segmentinProcess->segName;
				(*it)->segmentinProcess->logItem.data = (*it)->segmentinProcess->segAddr+(*it)->offset;
				if (fprintf(tid->rvm->flog, "commit_data/%s/%d/%d/", (*it)->segmentinProcess->segName, (*it)->offset, (*it)->transactionSize ) < 0){
					printf("\n failure to log \n");
					abort();
				}
			}
			else
			{
				printf("\n log pointer is null");
			}
			fwrite((void*)((char*)(*it)->segmentinProcess->segAddr + (*it)->offset),
					1,
					(*it)->transactionSize,
					tid->rvm->flog);
			fprintf(tid->rvm->flog, "\n");
			(*it)->segmentinProcess->dirty--;
			rewind((*it)->segmentinProcess->fsegment);	
			lseek(fileno((*it)->segmentinProcess->fsegment), (*it)->offset, SEEK_CUR);
			fwrite((*it)->segmentinProcess->segAddr + (*it)->offset, 
				1, 
				(*it)->transactionSize, 
				(*it)->segmentinProcess->fsegment);
			(*it)->aboutToModify = NULL;
		}
		delete *it;
		it = tid->action.erase(it); 
	}
	return;
}

void rvm_commit_trans_heavy(trans_t tid)
{
	vector<transItem *>::iterator it = tid->action.begin();

	while (it != tid->action.end()){
		if ((*it)->segmentinProcess->dirty){
			(*it)->segmentinProcess->dirty = 0;
			fwrite((*it)->segmentinProcess->segAddr, 1, (*it)->segmentinProcess->Segmentsize, (*it)->segmentinProcess->fsegment);
			(*it)->aboutToModify = NULL;
		}
		delete *it;
		it = tid->action.erase(it);
	}
	return;

}

void rvm_abort_trans(trans_t tid)
{
	map <void *, memSeg*>::iterator segBases = tid->segBases.begin();
	//FILE * oldbackingFile;

	vector<transItem *>::iterator it = tid->action.begin();
	while (it != tid->action.end() ){
		if ((*it)->segmentinProcess->dirty > 0){
			fprintf(tid->rvm->flog,"abort/%s/%d/%d/", (*it)->segmentinProcess->segName, (*it)->offset, (*it)->transactionSize);
			fwrite((void*)((char*)(*it)->segmentinProcess->segAddr + (*it)->offset),
					1,
					(*it)->transactionSize,
					tid->rvm->flog);
			fprintf(tid->rvm->flog, "\n");
			printf("bkp - %s\n", (*it)->aboutToModify);	
			memcpy((*it)->segmentinProcess->segAddr+(*it)->offset,(*it)->aboutToModify, (*it)->transactionSize);
			(*it)->segmentinProcess->dirty--;
		}
		
		delete *it;
		tid->action.erase(it);
	}
/*
	segBases = tid->segBases.begin();
	lseek(fileno(segBases->second->fsegment), 0, SEEK_SET);
	while (segBases != tid->segBases.end()){
		debugTrace("remapping up file ");
		segBases->second->segAddr = NULL;	
		segBases++;
	}
*/
}

void rvm_truncate_log(rvm_t rvm)
{

}

int write_log(rvm_t rvm, logitem log)
{
	string tolog = "";
	std::stringstream out;
	out << log.id << "~~" << log.tid << "~~" << log.segment_index << "~~" << log.offset << "~~" << log.size << "~~";//  << log.data<<"\n"; ( who to write data )
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

