/*
 * rvm.h
 *
 *  Created on: Nov 17, 2012
 *      Author: Ajay
 */

#ifndef RVM_H_
#define RVM_H_

#include <stdio.h>
#include <vector>
#include <string>
using namespace std;

#define MAX_SEGMENTS 20

/*
 *  Per Segment structure.
 */

/*
 *  i was thinking of adding offset here. But multiple transactions
 *  can use the same segment for reading and can read at differnet
 *  offsets at the same segment. so, it might be better to store the
 *  offset in the transactions.
 */

struct memSegment {
  char segName[20]; // seg name
  void * virtualAddress; // needed ?
  FILE * fsegment; // file pointer to the segment in the backing store
  int Segmentsize; // the size of the segment.
  int dirty; // yet to be written to disk
  int mapped; // to check it is mapped or not. remapping will lead to an abort
};
typedef memSegment memSeg;

struct rvm_details // main structure for rvm.
{
   string backingStore; // directory file name
   //vector<memSeg*> memSegs; // can be an array of pointer.
   memSeg * memSegs[MAX_SEGMENTS]; // number of segments
   long int storage_size; // not sure if we need this.
   int memSeg_count; // current segs may be. 20 memsegs is our restriction
   FILE * flog; // log file. flushed at commit.
   FILE * ftrace; // overall log file. never flushed
};
typedef rvm_details* rvm_t;

typedef struct transactionItem { // per transaction item.
	memSeg segmentinProcess; // current segment being processed
	int offset; // offset at that transaction.
	int TransactionSize; //
	int id;
}transItem;

struct transactions{
  vector<memSeg*> segbases; // can be a double pointer or a Map
  int numsegs;
  vector<transItem*> action;
  rvm_t rvm;
};
typedef transactions* trans_t;

typedef struct LogItem
{
	int id; // log id
	int tid; // transaction id
	int segment_index; // which segment
	int offset; // offset from the start
	int size; // size of the part of segment
	char *data; // no sure if we need it. still.....
}logitem;

rvm_t rvm_init(const char *directory);
void rvm_unmap(rvm_t rvm, void *segbase);
void rvm_destroy(rvm_t rvm, const char *segname);
trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases);
void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size);
void rvm_commit_trans(trans_t tid);
void rvm_commit_trans_heavy(trans_t tid);
void rvm_abort_trans(trans_t tid);
void rvm_truncate_log(rvm_t rvm);
void* rvm_map(rvm_t rvm, const char * segname, int size_to_create);

int write_log(FILE *fp, logitem log);
int TRACE(rvm_t rvm,  const char * trace_stm);
int read_log(rvm_t rvm, vector <logitem> log); // if needed


#endif /* RVM_H_ */
