/* basic.c - test that basic persistency works */

#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define TEST_STRING "hello, world"
#define OFFSET2 1000


/* proc1 writes some data, commits it, then exits */
void proc1() 
{		 
     rvm_t rvm;
     trans_t trans;
     char* segs[1];
     
     rvm = rvm_init("rvm_segments");
     rvm_destroy(rvm, "testseg");
     segs[0] = (char *) rvm_map(rvm, "testseg", 1000);

     
     trans = rvm_begin_trans(rvm, 1, (void **) segs);
     
     rvm_about_to_modify(trans, segs[0], 0, 1000);
     char * currChar = segs[0];
     for (int counter= 0; counter < 1000; counter++){
       *currChar = '!';
       currChar++;
     }
     
     rvm_commit_trans(trans);
     
     

     abort();
}


/* proc2 opens the segments and reads from them */
void proc2() 
{
     char* segs[1];
     rvm_t rvm;
     
     rvm = rvm_init("rvm_segments");
     char * currChar;
     segs[0] = (char *) rvm_map(rvm, "testseg", 1000 + 1000);
     currChar = segs[0];
     for (int counter = 0; counter < 1000; counter++){
       if (*currChar !='!'){
	 fprintf(stderr, "ERROR %c instead of !", *currChar);
       }
       
     }

     printf("OK\n");
     exit(0);
}


int main(int argc, char **argv)
{
     int pid;

  //   rvm_verbose(1);

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
