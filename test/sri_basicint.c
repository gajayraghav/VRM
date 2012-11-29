/* basicint.c - test that basic persistency works with data as integers */

#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/wait.h>

#define TEST_STRING "hello, world"
#define OFFSET2 1000
int a=1;
//char* segs1[1];

/* proc1 writes some data, commits it, then exits */
void proc1() 
{
     rvm_t rvm;
     trans_t trans;
     char* segs[1];
	//int a=145;
     
     rvm = rvm_init("rvm_segments");
     rvm_destroy(rvm, "testseg");
     segs[0] = (char *) rvm_map(rvm, "testseg", 10000);

     
     trans = rvm_begin_trans(rvm, 1, (void **) segs);
     
     rvm_about_to_modify(trans, segs[0], 0, 100);
     sprintf(segs[0],"%d",a);
     
     rvm_about_to_modify(trans, segs[0], OFFSET2, 100);
     sprintf(segs[0]+OFFSET2,"%d",a);
     
     rvm_commit_trans(trans);

     abort();
}


/* proc2 opens the segments and reads from them */
void proc2() 
{
	rvm_t rvm;
     	char* segs[1];
	//char dig = (char)(((int)'0')+1);
	//printf("%s",dig);
//	char *comp;
//	strcpy(comp,"145");
//	char* segscomp[1];
     //sprintf(segs1[0],"%d",a);
//sprintf(segs1[0],"%d",a);
     //char* b;
	//sprintf(segscomp[0],"%d",a);
	//sprintf(segscomp[0]+OFFSET2,"%d",a);
     rvm = rvm_init("rvm_segments");
	//char buffer[30];
	//itoa(a,buffer,10);
     segs[0] = (char *) rvm_map(rvm, "testseg", 10000);
	//printf("%s",segs[0]);
	
     if(strcmp(segs[0],"1")) {
	  printf("ERROR: first %d not present\n",a);
	  exit(2);
     }
     if(strcmp(segs[0]+OFFSET2,"1")) {
	  printf("ERROR: second %d not present\n",a);
	  exit(2);
     }

     printf("OK\n");
     exit(0);
}


int main(int argc, char **argv)
{
     int pid;
//sprintf(segs1[0],"%d",a);i
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
