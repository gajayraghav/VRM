/* expand.c - test that if expansion works */

#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define TEST_STRING1 "hello, world"
#define TEST_STRING2 "ajay, balaji"
#define OFFSET1 700
#define OFFSET2 1500

/* proc1 writes some data, commits it, then exits */
void proc1() {
	rvm_t rvm;
	trans_t trans;
	char* segs[1];

	rvm = rvm_init("rvm_segments");
	segs[0] = (char *) rvm_map(rvm, "testseg", 1000);
	trans = rvm_begin_trans(rvm, 1, (void **) segs);

	rvm_about_to_modify(trans, segs[0], OFFSET1, 100);
	sprintf(segs[0] + OFFSET1, TEST_STRING1);

	rvm_commit_trans(trans);
	rvm_unmap(rvm, segs[0]);

	segs[0] = (char *) rvm_map(rvm, "testseg", 2000);
	trans = rvm_begin_trans(rvm, 1, (void **) segs);
	rvm_about_to_modify(trans, segs[0], OFFSET2, 100);
	sprintf(segs[0] + OFFSET2, TEST_STRING2);
	rvm_commit_trans(trans);
	//     rvm_commit_trans_heavy(trans);
	abort();
}

/* proc2 opens the segments and reads from them */
void proc2() {
	char* segs[1];
	rvm_t rvm;

	rvm = rvm_init("rvm_segments");

	segs[0] = (char *) rvm_map(rvm, "testseg", 2000);
	if (strcmp(segs[0] + OFFSET1, TEST_STRING1)) {
		printf("ERROR: first hello not present\n");
		exit(2);
	}
	if (strcmp(segs[0] + OFFSET2, TEST_STRING2)) {
		printf("ERROR: second hello not present\n");
		exit(2);
	}

	printf("OK\n");
	exit(0);
}

int main(int argc, char **argv) {
	int pid;

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
