#ifndef RVM_H_
#define RVM_H_
#include <stdio.h>
#include <vector>
#include <map>
#include <string>
using namespace std;

#define MAX_SEGMENTS 30
#define SEGMENT_SIZE 1024*1024*10

#define ABOUT_TO_MODIFY 0
#define COMMIT 		1
#define ABORT		2
typedef int trans_t;

// Log Structure
typedef struct LogItem {
	int id; // log id
	int tid; // transaction id
	int segment_index; // which segment
	char *segName; // which segment
	int offset; // offset from the start
	int size; // size of the part of segment
	char *data; // no sure if we need it. still.....
} logitem;

// Segment strucutre
struct memSegment {
	char segName[20]; // seg name
	char* segAddr;
	logitem logItem;
	FILE * fsegment; // file pointer to the segment in the backing store
	int Segmentsize; // the size of the segment.
	int dirty; // yet to be written to disk
	int mapped; // to check it is mapped or not. remapping will lead to an abort
	int transaction; // to check if the segment is used in a transaction
};
typedef memSegment memSeg;

// RVM structure
struct rvm_details {
	string backingStore; // directory file name
	memSeg * memSegs[MAX_SEGMENTS]; // number of segments
	long int storage_size; // not sure if we need this.
	int memSeg_count; // current segs may be. 20 memsegs is our restriction
	FILE * flog; // log file. flushed at commit.
	//FILE * ftrace; 			// overall log file. never flushed
};
typedef rvm_details* rvm_t;

// Transaction Item Structure
typedef struct transactionItem { // per transaction item
	memSeg *segmentinProcess; // current segment being processed
	int offset; // offset at that transaction
	int transactionSize; // transaction size
	char * aboutToModify; // about_to_modify
	int transactionType; // transaction type
	int id; // transaction id
} transItem;

// Transaction Structure 
struct transactions {
	map<void *, memSeg*> segBases; // can be a double pointer or a Map
	vector<transItem*> action; // transaction action
	int numsegs; // number of segments
	int transactionId; // transaction ID
	rvm_t rvm;
};
typedef transactions* trans_s;

// Function Prototypes
rvm_t rvm_init(const char *directory);
void* rvm_map(rvm_t rvm, const char * segname, int size_to_create);
void rvm_unmap(rvm_t rvm, void *segbase);
void rvm_destroy(rvm_t rvm, const char *segname);
trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases);
void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size);
void rvm_commit_trans(trans_t tid);
void rvm_commit_trans_heavy(trans_t tid);
void rvm_abort_trans(trans_t tid);
void rvm_truncate_log(rvm_t rvm);
int TRACE(string trace_stm);
void write_to_tracefile();
void write_to_screen();

#endif /* RVM_H_ */
