/* user-multiseg.c - test that basic persistency works for 20 segments */


#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <sys/wait.h>

#define SEGNAME0  "testseg1"
#define SEGNAME1  "testseg2"
#define SEGNAME2  "testseg3"
#define SEGNAME3  "testseg4"
#define SEGNAME4  "testseg5"
#define SEGNAME5  "testseg6"
#define SEGNAME6  "testseg7"
#define SEGNAME7  "testseg8"
#define SEGNAME8  "testseg9"
#define SEGNAME9  "testseg10"
#define SEGNAME10  "testseg11"
#define SEGNAME11  "testseg12"
#define SEGNAME12  "testseg13"
#define SEGNAME13  "testseg14"
#define SEGNAME14  "testseg15"
#define SEGNAME15  "testseg16"
#define SEGNAME16  "testseg17"
#define SEGNAME17  "testseg18"
#define SEGNAME18  "testseg19"
#define SEGNAME19  "testseg20"


#define STRING0 "hello, world"
#define STRING1 "black agagadrof!"
#define STRING2 "apple"
#define STRING3 "banana"
#define STRING4 "orange"
#define STRING5 "macbook pro"
#define STRING6 "gatech"
#define STRING7 "new york"
#define STRING8 "Whatsup Dude"
#define STRING9 "Random1"
#define STRING10 "Random2"
#define STRING11 "creation"
#define STRING12 "nexus"
#define STRING13 "google"
#define STRING14 "gangnam"
#define STRING15 "Troll"
#define STRING16 "Max Payne"
#define STRING17 "Reminder"
#define STRING18 "This"
#define STRING19 "Friday"

struct timeval  startTime, endTime, temp;
static int timeval_subtract(struct timeval *result, struct timeval *final,
		struct timeval *initial) {
	/* Perform the carry for the later subtraction by updating y. */
	if (final->tv_usec < initial->tv_usec) {
		int nsec = (initial->tv_usec - final->tv_usec) / 1000000 + 1;
		initial->tv_usec -= 1000000 * nsec;
		initial->tv_sec += nsec;
	}
	if (final->tv_usec - initial->tv_usec > 1000000) {
		int nsec = (final->tv_usec - initial->tv_usec) / 1000000;
		initial->tv_usec += 1000000 * nsec;
		initial->tv_sec -= nsec;
	}

	/* Compute the time remaining to wait. tv_usec is certainly positive. */
	result->tv_sec = final->tv_sec - initial->tv_sec;
	result->tv_usec = final->tv_usec - initial->tv_usec;

	/* Return 1 if result is negative. */
	return final->tv_sec < initial->tv_sec;
}

static void timeval_add(struct timeval *result, struct timeval *a,
		struct timeval *b) {
	result->tv_sec = a->tv_sec + b->tv_sec;
	result->tv_usec = a->tv_usec + b->tv_usec;
	while (result->tv_usec > 1000000) {
		result->tv_usec -= 1000000;
		result->tv_sec++;
	}
}

void proc1(int arguments) 
{
	int i; 
	int rand1,rand2;    
	rvm_t rvm;
     char* segs[20];
     trans_t trans;
	
     
     rvm = rvm_init("rvm_segments");

     rvm_destroy(rvm, SEGNAME0);
     rvm_destroy(rvm, SEGNAME1);
rvm_destroy(rvm, SEGNAME2);
rvm_destroy(rvm, SEGNAME3);
rvm_destroy(rvm, SEGNAME4);
rvm_destroy(rvm, SEGNAME5);
rvm_destroy(rvm, SEGNAME6);
rvm_destroy(rvm, SEGNAME7);
  rvm_destroy(rvm, SEGNAME8);
     rvm_destroy(rvm, SEGNAME9);
rvm_destroy(rvm, SEGNAME10);
rvm_destroy(rvm, SEGNAME11);
rvm_destroy(rvm, SEGNAME12);
rvm_destroy(rvm, SEGNAME13);
rvm_destroy(rvm, SEGNAME14);
rvm_destroy(rvm, SEGNAME15);
  rvm_destroy(rvm, SEGNAME16);
     rvm_destroy(rvm, SEGNAME17);
rvm_destroy(rvm, SEGNAME18);
rvm_destroy(rvm, SEGNAME19);


     segs[0] = (char*) rvm_map(rvm, SEGNAME0, 1024*1024*10);
     segs[1] = (char*) rvm_map(rvm, SEGNAME1, 1024*1024*10);
segs[2] = (char*) rvm_map(rvm, SEGNAME2, 1024*1024*10);
segs[3] = (char*) rvm_map(rvm, SEGNAME3, 1024*1024*10);
segs[4] = (char*) rvm_map(rvm, SEGNAME4, 1024*1024*10);
segs[5] = (char*) rvm_map(rvm, SEGNAME5, 1024*1024*10);
segs[6] = (char*) rvm_map(rvm, SEGNAME6, 1024*1024*10);
segs[7] = (char*) rvm_map(rvm, SEGNAME7, 1024*1024*10);
     segs[8] = (char*) rvm_map(rvm, SEGNAME8, 1024*1024*10);
     segs[9] = (char*) rvm_map(rvm, SEGNAME9, 1024*1024*10);
segs[10] = (char*) rvm_map(rvm, SEGNAME10, 1024*1024*10);
segs[11] = (char*) rvm_map(rvm, SEGNAME11, 1024*1024*10);
segs[12] = (char*) rvm_map(rvm, SEGNAME12, 1024*1024*10);
segs[13] = (char*) rvm_map(rvm, SEGNAME13, 1024*1024*10);
segs[14] = (char*) rvm_map(rvm, SEGNAME14, 1024*1024*10);
segs[15] = (char*) rvm_map(rvm, SEGNAME15, 1024*1024*10);
segs[16] = (char*) rvm_map(rvm, SEGNAME16, 1024*1024*10);
segs[17] = (char*) rvm_map(rvm, SEGNAME17, 1024*1024*10);
segs[18] = (char*) rvm_map(rvm, SEGNAME18, 1024*1024*10);
segs[19] = (char*) rvm_map(rvm, SEGNAME19, 1024*1024*10);
//segs[20] = (char*) rvm_map(rvm, SEGNAME14, 1024*1024*10);
//segs[15] = (char*) rvm_map(rvm, SEGNAME15, 1024*1024*10);


	
     trans = rvm_begin_trans(rvm, 20, (void **)segs);

  /*   rvm_about_to_modify(trans, segs[0], OFFSET0, 100);
     strcpy(segs[0]+OFFSET0, STRING0);
     rvm_about_to_modify(trans, segs[1], OFFSET1, 100);
     strcpy(segs[1]+OFFSET1, STRING1);
	rvm_about_to_modify(trans, segs[2], OFFSET2, 100);
     strcpy(segs[2]+OFFSET2, STRING2);
rvm_about_to_modify(trans, segs[3], OFFSET3, 100);
     strcpy(segs[3]+OFFSET3, STRING3);
rvm_about_to_modify(trans, segs[4], OFFSET4, 100);
     strcpy(segs[4]+OFFSET4, STRING4);
rvm_about_to_modify(trans, segs[5], OFFSET5, 100);
     strcpy(segs[5]+OFFSET5, STRING5);
rvm_about_to_modify(trans, segs[6], OFFSET6, 100);
     strcpy(segs[6]+OFFSET6, STRING6);
rvm_about_to_modify(trans, segs[7], OFFSET7, 100);
     strcpy(segs[7]+OFFSET7, STRING7);
*/
while (gettimeofday(&startTime, NULL));
	for(i=0;i<arguments;i++)
{
	rand1=rand()%20;
	rand2=(rand()%((1024*1024*10)+10));
	rvm_about_to_modify(trans,segs[rand1],rand2,10);
	strcpy(segs[rand1]+rand2,STRING0);
	
     //abort();
}
rvm_commit_trans(trans);
//rvm_truncate_log(rvm);
while (gettimeofday(&endTime, NULL));
timeval_subtract(&temp, &endTime, &startTime);
		printf("%d times called about_to_modify for commit and Execution time: %ld.%06lds\n", arguments,temp.tv_sec, temp.tv_usec);
}


/*void proc2() 
{
     rvm_t rvm;
     char *segs[8];

     rvm = rvm_init("rvm_segments");
     segs[0] = (char*) rvm_map(rvm, SEGNAME0, 1000);
     segs[1] = (char*) rvm_map(rvm, SEGNAME1, 1000);
	segs[2] = (char*) rvm_map(rvm, SEGNAME2, 1000);
segs[3] = (char*) rvm_map(rvm, SEGNAME3, 1000);
segs[4] = (char*) rvm_map(rvm, SEGNAME4, 1000);
segs[5] = (char*) rvm_map(rvm, SEGNAME5, 1000);
segs[6] = (char*) rvm_map(rvm, SEGNAME6, 1000);
segs[7] = (char*) rvm_map(rvm, SEGNAME7, 1000);

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
 if(strcmp(segs[3] + OFFSET3, STRING3)) {
	  printf("ERROR in segment 3 (%s)\n",
		 segs[3]+OFFSET3);
	  exit(2);
     }
 if(strcmp(segs[4] + OFFSET4, STRING4)) {
	  printf("ERROR in segment 4 (%s)\n",
		 segs[4]+OFFSET4);
	  exit(2);
     }
 if(strcmp(segs[5] + OFFSET5, STRING5)) {
	  printf("ERROR in segment 5 (%s)\n",
		 segs[5]+OFFSET5);
	  exit(2);
     }
 if(strcmp(segs[6] + OFFSET6, STRING6)) {
	  printf("ERROR in segment 6 (%s)\n",
		 segs[6]+OFFSET6);
	  exit(2);
     }
 if(strcmp(segs[7] + OFFSET7, STRING7)) {
	  printf("ERROR in segment 7 (%s)\n",
		 segs[7]+OFFSET7);
	  exit(2);
     }


     printf("OK\n");
}

*/
int main(int argc, char **argv) 
{
     //int pid;
     int arguments = atoi(argv[1]);
	
  //   pid = fork();
   //  if(pid < 0) {
	//  perror("fork");
	 // exit(2);
    // }
    // if(pid == 0) {
	  proc1(arguments);
	 // exit(0);
     //}

     //waitpid(pid, NULL, 0);

     //proc2();

     return 0;
}

