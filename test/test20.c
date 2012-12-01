/* test20.c - to test 20 segments and data at different offset at each segment */

#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#define TEST_STRING "hello, world"

#define OFFSET1 1000
#define SIZE 1024*1024*10

int count, modify;
FILE *fp;

long subtracttime(struct timeval *start, struct timeval *end) {
	long sStart, sEnd;
	sStart = start->tv_sec * 1000000 + start->tv_usec;
	sEnd = end->tv_sec * 1000000 + end->tv_usec;
	return (sEnd - sStart);
}

/* proc1 writes some data, commits it, then exits */
void proc1() {
	rvm_t rvm;
	trans_t trans;
	char* segs[20];
	char segname[20][10] = { { "page0" }, { "page1" }, { "page2" },
			{ "page3" }, { "page4" }, { "page5" }, { "page6" }, { "page7" }, {
					"page8" }, { "page9" }, { "page10" }, { "page11" }, {
					"page12" }, { "page13" }, { "page14" }, { "page15" }, {
					"page16" }, { "page17" }, { "page18" }, { "page19" } };
	int loop = 0;
	rvm = rvm_init("rvm_segments");
	//rvm_destroy(rvm, "testseg");
	for (loop = 0; loop < count; loop++) {
		segs[loop] = (char *) rvm_map(rvm, segname[loop], SIZE);
	}

	trans = rvm_begin_trans(rvm, count, (void **) segs);

	for (int i = 0; i < count; i++) {
		for (int j = 0; j < modify; j++) {
			rvm_about_to_modify(trans, segs[i], OFFSET1 * i + j * 10, j * 10
					+ 10);
			sprintf(segs[i] + OFFSET1 * i + j * 10, segname[i]);

		}
	}

	struct timeval start, temp, stop;
	gettimeofday(&start, NULL);
	rvm_commit_trans(trans); // _heavy
	gettimeofday(&stop, NULL);
	long timediff = subtracttime(&start, &stop);

	if (fp != NULL) {
		fprintf(fp, "%d\t%ld\n", modify, timediff);
		//    	 fflush(fp);
	} else {
		printf("\n fp is null \n");
	}
	printf("\n done proc 1 \n");
	abort();
}

/* proc2 opens the segments and reads from them */
void proc2() {
	rvm_t rvm;
	char* segs[20];
	char segname[20][10] = { { "page0" }, { "page1" }, { "page2" },
			{ "page3" }, { "page4" }, { "page5" }, { "page6" }, { "page7" }, {
					"page8" }, { "page9" }, { "page10" }, { "page11" }, {
					"page12" }, { "page13" }, { "page14" }, { "page15" }, {
					"page16" }, { "page17" }, { "page18" }, { "page19" } };
	int loop = 0;
	rvm = rvm_init("rvm_segments");
	//rvm_destroy(rvm, "testseg");
	for (loop = 0; loop < count; loop++) {
		segs[loop] = (char *) rvm_map(rvm, segname[loop], SIZE);
	}

	for (int i = 0; i < count; i++) {
		for (int j = 0; j < modify; j++) {
			if (strcmp(segs[i] + OFFSET1 * i + j * 10, segname[i])) {
				printf("ERROR: hello not present @ %d\n", i);
				exit(2);
			}
		}
	}
	//    system("rm -rf rvm_segments/");

	printf("\n OK\n");

	exit(0);
}

int main(int argc, char **argv) {
	int pid;
	if (argc != 3) {
		printf("\n enter: ./test20 seg_count modify_count \n");
		system("rm -rf rvm_segments/");
		exit(0);
	}
	count = atoi(argv[1]);
	int abouttomodify = atoi(argv[2]);
	modify = abouttomodify;
	fp = fopen("result.xls", "a+");
	//    for(modify=1; modify<=abouttomodify; modify++)
	{
		if (fp == NULL) {
			perror("result file");
		}
		pid = fork();
		if (pid < 0) {
			perror("fork");
			exit(2);
		}
		if (pid == 0) {
			proc1();
			//exit(0);
		}

		waitpid(pid, NULL, 0);

		proc2();
		//     sleep(2);
	}

	if (fp)
		fclose(fp);
	return 0;
}
