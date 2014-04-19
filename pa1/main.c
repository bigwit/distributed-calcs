/*
 * main.c
 *
 *  Created on: 10 апр. 2014 г.
 *      Author: nikit
 */

#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "ipc.h"
#include "common.h"
#include "pa1.h"
#include "pipe.h"

void handle_child(const size_t local_id) __attribute__((noreturn));

extern char * optarg;
local_id my_local_id;

int main(int argc, char ** argv) {
	pid_t pid;

	if (getopt(argc, argv, "p:") == -1) {
		_exit(-1);
	}

	int num_proc = atoi(optarg);
	if (num_proc <= 0 || num_proc > 10) {
		fprintf(stderr, "Invalid number processes\n");
		_exit(-2);
	}

	init_pipes((size_t) num_proc);

	for (size_t i = 0; i < num_proc; i++) {
		switch (pid = fork()) {
		case -1: _exit(-1);
		case 0: handle_child(i); break;
		default:
			break;
		}
	}
}

void handle_child(const size_t _local_id) {
	my_local_id = _local_id;

	configure_pipes(my_local_id);

	_exit(0);
}
