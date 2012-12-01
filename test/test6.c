/* basic.c - test that basic persistency works */

#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#define TEST_STRING1 "hello"
#define TEST_STRING2 "bleg!"
#define TEST_STRING3 "wowww"
#define TEST_STRING4 "yeahhhh"
#define TEST_STRING5 "please"
#define TEST_STRING6 "this"
#define OFFSET2 110
#define OFFSET3 220
#define OFFSET4 330

/* proc1 writes some data, commits it, then exits */
void proc1() {
	rvm_t rvm;
	char *seg;
	char *segs[1];
	trans_t trans;

	rvm = rvm_init("rvm_segments");

	rvm_destroy(rvm, "testseg");

	segs[0] = (char *) rvm_map(rvm, "testseg", 10000);
	seg = segs[0];

	/* write some data and commit it */
	trans = rvm_begin_trans(rvm, 1, (void**) segs);
	rvm_about_to_modify(trans, seg, 0, 100);
	sprintf(seg, TEST_STRING1);

	rvm_about_to_modify(trans, seg, OFFSET2, 100);
	sprintf(seg + OFFSET2, TEST_STRING2);

	rvm_about_to_modify(trans, seg, OFFSET3, 100);
	sprintf(seg + OFFSET3, TEST_STRING3);
	rvm_commit_trans(trans);

	/* start writing some different data, but abort */
	trans = rvm_begin_trans(rvm, 1, (void**) segs);
	rvm_about_to_modify(trans, seg, 0, 100);
	sprintf(seg, TEST_STRING4);

	rvm_about_to_modify(trans, seg, OFFSET2, 100);
	sprintf(seg + OFFSET2, TEST_STRING5);

	rvm_about_to_modify(trans, seg, OFFSET3, 100);
	sprintf(seg + OFFSET3, TEST_STRING6);
	rvm_abort_trans(trans);
	abort();
}

/* proc2 opens the segments and reads from them */
void proc2() {
	char* segs[1];
	char *seg;
	rvm_t rvm;

	rvm = rvm_init("rvm_segments");
	segs[0] = (char *) rvm_map(rvm, "testseg", 10000);
	seg = segs[0];

	if (strcmp(seg + OFFSET2, TEST_STRING2)) {
		printf("ERROR: blew is incorrect (%s)\n", seg + OFFSET2);
		exit(2);
	}

	if (strcmp(seg, TEST_STRING1)) {
		printf("ERROR: first hello is incorrect (%s)\n", seg);
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
