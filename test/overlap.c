/* overlap.c - test overlapping parts of segments */

#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define TEST1 "yov"
#define TEST2 "hello, world"
#define TEST3 "ajay"
#define TEST4 "archanaa"
#define TEST5 "balaji"
#define TEST6 "sriram"
#define TESTALL "abcdefghijklmnopqrstuvwxyz"
#define OFFSET2 1000


/* proc1 writes some data, commits it, then exits */
void proc1()
{
     rvm_t rvm;
     trans_t trans;
     char* segs[1];

     rvm = rvm_init("rvm_segments");
    // rvm_destroy(rvm, "testseg");
     segs[0] = (char *) rvm_map(rvm, "testseg", 10000);


     trans = rvm_begin_trans(rvm, 1, (void **) segs);

     rvm_about_to_modify(trans, segs[0], 0, 100);
     sprintf(segs[0], TESTALL);

     rvm_about_to_modify(trans, segs[0], 6, 10);
     sprintf(segs[0]+6, TEST1);

     rvm_about_to_modify(trans, segs[0], 0, 100);
     sprintf(segs[0], TEST3);

     rvm_about_to_modify(trans, segs[0], 13, 100);
     sprintf(segs[0]+13, TEST4);

     rvm_about_to_modify(trans, segs[0], 26, 100);
     sprintf(segs[0]+26, TEST5);

/*
     rvm_about_to_modify(trans, segs[0], OFFSET2, 100);
     sprintf(segs[0]+OFFSET2, TEST_STRING);
*/


     rvm_commit_trans(trans);
//     rvm_commit_trans_heavy(trans);
     abort();
}


/* proc2 opens the segments and reads from them */
void proc2()
{
     char* segs[1];
     rvm_t rvm;

     rvm = rvm_init("rvm_segments");

     segs[0] = (char *) rvm_map(rvm, "testseg", 10100);
     if(strcmp(segs[0], "hello yovv")) {
	  printf("ERROR: first hello not present\n");
	  exit(2);
     }
     if(strcmp(segs[0]+6, TEST1)) {
	  printf("ERROR: second hello not present\n");
	  exit(2);
     }

     printf("OK\n");
     exit(0);
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
