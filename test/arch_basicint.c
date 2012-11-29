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
#define OFFSET2 4000
int a = 1, b = 101;
//char* segs1[1];

/* proc1 writes some data, commits it, then exits */
void proc1() {
	rvm_t rvm;
	trans_t trans;
	int* segs[2];
	//int a=145;

	rvm = rvm_init("rvm_segments");
	rvm_destroy(rvm, "testseg");
	segs[0] = (int *) rvm_map(rvm, "testseg", 10000);
	segs[1]=(int *)((char *)segs[0] + OFFSET2);

	trans = rvm_begin_trans(rvm, 1, (void **) segs);

	rvm_about_to_modify(trans, segs[0], 0, 100);
	*segs[0] = a;

	rvm_about_to_modify(trans, segs[0], OFFSET2, 100);
	*segs[1] = b;

	printf("Written %d and %d\n",*segs[0],*segs[1]);

	rvm_commit_trans(trans);

	abort();
}

/* proc2 opens the segments and reads from them */
void proc2() {
	rvm_t rvm;
	int* segs[2];
	rvm = rvm_init("rvm_segments");
	segs[0] = (int *) rvm_map(rvm, "testseg", 10000);
	segs[1]=(int *)((char *)segs[0] + OFFSET2);

	printf("Read `%d` and `%d`\n",*segs[0],*segs[1]);
	//*(segs[0]) = a;
	if (*(segs[0]) != a) {
		printf("ERROR: first %d not present\n", a);
		exit(2);
	}
	if (*segs[1] != b) {
		printf("ERROR: second %d not present\n", b);
		exit(2);
	}

	printf("OK\n");
	exit(0);
}

int main(int argc, char **argv) {
	int pid;
	//sprintf(segs1[0],"%d",a);i
	pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(2);
	}
	if (pid == 0) {
		proc1();
		exit(0);
	}

	waitpid(pid, NULL, 0);

	proc2();

	return 0;
}
