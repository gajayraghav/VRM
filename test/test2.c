/* multi.c - test that basic persistency works for multiple segments */


#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SEGNAME0  "testseg1"
#define SEGNAME1  "testseg2"
#define SEGNAME2  "testseg3"

#define OFFSET0  10
#define OFFSET1  100
#define OFFSET2  900

#define STRING0 "hello, world"
#define STRING1 "black agagadrof!"
#define STRING2 "tough orld"

void proc1() 
{
     rvm_t rvm;
     char* segs[3];
     trans_t trans;
     
     rvm = rvm_init("rvm_segments");

     rvm_destroy(rvm, SEGNAME0);
     rvm_destroy(rvm, SEGNAME1);

     segs[0] = (char*) rvm_map(rvm, SEGNAME0, 1000);
     segs[1] = (char*) rvm_map(rvm, SEGNAME1, 1000);
     segs[2] = (char*) rvm_map(rvm, SEGNAME2, 1000);
     trans = rvm_begin_trans(rvm, 3, (void **)segs);

     rvm_about_to_modify(trans, segs[0], OFFSET0, 100);
     strcpy(segs[0]+OFFSET0, STRING0);
     rvm_about_to_modify(trans, segs[1], OFFSET1, 100);
     strcpy(segs[1]+OFFSET1, STRING1);
     rvm_about_to_modify(trans, segs[2], OFFSET2, 100);
     strcpy(segs[2]+OFFSET2, STRING2);
     rvm_commit_trans(trans);

     abort();
}


void proc2() 
{
     rvm_t rvm;
     char *segs[2];

     rvm = rvm_init("rvm_segments");
     segs[0] = (char*) rvm_map(rvm, SEGNAME0, 1000);
     segs[1] = (char*) rvm_map(rvm, SEGNAME1, 1000);
     segs[2] = (char*) rvm_map(rvm, SEGNAME2, 1000);
     if(strcmp(segs[0] + OFFSET0, STRING0)) {
	  printf("ERROR in segment 0 (%s)\n",
		 segs[0]+OFFSET0);
	  exit(2);
     }
     if(strcmp(segs[1] + OFFSET1, STRING1)) {
	  printf("ERROR in segment 1 (%s)\n",
		 segs[1]+OFFSET1);
	  exit(2);
     }
     if(strcmp(segs[2] + OFFSET2, STRING2)) {
	  printf("ERROR in segment 2 (%s)\n",
		 segs[2]+OFFSET2);
	  exit(2);
     }

     printf("OK\n");
}


int main(int argc, char **argv) 
{
     int pid;

     pid = fork();
     if(pid < 0) {
	  perror("fork");
	  exit(2);
     }
     if(pid == 0) {
	  proc1();
	  exit(0);
     }

     waitpid(pid, NULL, 0);

     proc2();

     return 0;
}

