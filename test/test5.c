/*
 * test5.c - checking reusing a segment in different transaction
 *
 *  Created on: Nov 29, 2012
 *      Author: Ajay
 */

#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define TEST_STRING "hello, world"

#define OFFSET1 1000

/* proc1 writes some data, commits it, then exits */
void proc1() {
	rvm_t rvm;
	trans_t trans1, trans2;
	char* segs[1];

	rvm = rvm_init("rvm_segments");
	//rvm_destroy(rvm, "testseg");
	segs[0] = (char *) rvm_map(rvm, "page0", 10000);

	trans1 = rvm_begin_trans(rvm, 1, (void **) segs);
	trans2 = rvm_begin_trans(rvm, 1, (void **) segs);

	/*   rvm_about_to_modify(trans, segs[1], OFFSET1, 100);
	 sprintf(segs[1], TEST_STRING);*/

	/*

	 rvm_about_to_modify(trans, segs[0], 0, 100);
	 sprintf(segs[0], TEST_STRING);

	 rvm_about_to_modify(trans, segs[1], OFFSET1*1, 100);
	 sprintf(segs[1], TEST_STRING);

	 rvm_about_to_modify(trans, segs[2], OFFSET1*2, 100);
	 sprintf(segs[2], TEST_STRING);
	 */

	/*
	 rvm_about_to_modify(trans, segs[0], OFFSET2, 100);
	 sprintf(segs[0]+OFFSET2, TEST_STRING);
	 */

	//   rvm_commit_trans(trans);
	//     rvm_commit_trans_heavy(trans);
	abort();
}

/* proc2 opens the segments and reads from them */
void proc2() {
	char* segs[5];
	rvm_t rvm;

	rvm = rvm_init("rvm_segments");

	segs[0] = (char *) rvm_map(rvm, "page0", 10000);
	segs[1] = (char *) rvm_map(rvm, "page1", 10100);
	segs[2] = (char *) rvm_map(rvm, "page2", 10200);
	segs[3] = (char *) rvm_map(rvm, "page3", 10300);
	segs[4] = (char *) rvm_map(rvm, "page4", 10400);

	for (int i = 0; i < 5; i++) {
		if (strcmp(segs[i] + OFFSET1 * i, TEST_STRING)) {
			printf("ERROR: hello not present @ %d\n", i);
			exit(2);
		}
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

	// proc2();

	return 0;
}
