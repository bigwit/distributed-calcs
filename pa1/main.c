#include "main.h"
#include <time.h>
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

extern char * optarg;
descriptor * pds;
size_t count_proc;
int * receved;

int main(int argc, char* argv[]) {
	// get option -p X
	if (getopt(argc, argv, PARAM_COUNT_PROC_NAME) == -1) {
		fprintf(stderr, "getopts: not found -%s parameter\n",
		PARAM_COUNT_PROC_NAME);
		_exit(-1);
	}
	// get count processes as int value
	count_proc = atoi(optarg);
	if (count_proc < 1 || count_proc > 10) {
		fprintf(stderr, "main: invalid count processes in parameter -%s\n",
		PARAM_COUNT_PROC_NAME);
		_exit(-1);
	}
	// create buffer for all descriptors
	pds = (descriptor*) malloc((count_proc + 1) * sizeof(descriptor));
	// NullPointer check
	if (pds == NULL) {
		printf("main: out of memory error");
		_exit(-1);
	}
	// initialization parent descriptor
	// it saved by 0 index in descriptors storage
	pds[0].v_pid = 0;
	pds[0].r_pid = getpid();
	if (pipe(pds[0].pipedes) == -1) {
		perror("pipe");
		_exit(-1);
	}

	// initialization other descriptors
	for (int i = 1; i < count_proc + 1; ++i) {
		// save pipe descriptors
		if (pipe(pds[i].pipedes) == -1) {
			perror("pipe");
			_exit(-1);
		}
		// save local id of process.
		// this id equals with index in
		// descriptions storage
		pds[i].v_pid = i;
	}

	// create child-processes
	for (int i = 1; i <= count_proc; ++i) {
		int pid = fork();
		if (pid == 0) {
			// run handle of new process
			handle_child();
		}
		pds[i].r_pid = pid;
	}

	wait_all(0); // wait started
	wait_all(0);		// and done messages
	for (int i = 1; i <= count_proc; ++i) {
		wait(NULL);
	}
	free(pds);
	return 0;
}

int send(void * self, local_id dst, const Message * msg) {
	return write(pds[dst].pipedes[1], msg, sizeof(Message));
}

/**
 *
 * @return zero if success and -1 if fail
 */
int send_multicast(void * self, const Message * msg) {
	for (int i = 0; i <= count_proc; ++i) {
		if (pds[i].r_pid != getpid()) {
			if (write(pds[i].pipedes[1], msg, sizeof(Message)) == -1) {
				return -1;
			}
		}
	}
	return 0;
}

int receive(void * self, local_id from, Message * msg) {
	if (read(pds[from].pipedes[0], msg, sizeof(Message)) == -1) {
		return -1;
	}
	return 0;
}

int receive_any(void * self, Message * msg) {
	for (int i = 1; i <= count_proc; ++i) {

	}
	return 0;
}

void wait_all(local_id id) {
	for (int proc = 1; proc <= count_proc; ++proc) {
		if (pds[proc].v_pid == id) {
			continue;
		}
		Message * msg = (Message*) malloc(sizeof(Message));
		int result_call;
		if ((result_call = receive(NULL, proc, msg)) != 0) {
			fprintf(stderr, "receive: can not execute");
		}
		// print to log message
	}
}

void init_message(Message * message, char * const line, MessageType type) {
	// set body of message
	//message->s_payload = line;
	strcpy(message->s_payload, line);
	// create and initial header
	MessageHeader header;
	header.s_local_time = (timestamp_t) time(NULL);
	header.s_magic = MESSAGE_MAGIC;
	header.s_payload_len = (uint16_t) strlen(line);
	header.s_type = type;
	// set header in message
	message->s_header = header;
}

void handle_child(void) {
	Message * msg = (Message*) malloc(sizeof(Message));
	init_message(msg, "hi, man!", STARTED);
	send_multicast(NULL, msg);

	// logging

	for (int i = 1; i <= count_proc; ++i) {
		if (pds[i].r_pid == getpid()) {
			wait_all(pds[i].v_pid);
			break;
		}
	}

	Message * msg2 = (Message*) malloc(sizeof(Message));
	init_message(msg, "fuck you", DONE);
	send_multicast(NULL, msg2);

	for (int i = 1; i <= count_proc; ++i) {
		if (pds[i].r_pid == getpid()) {
			wait_all(pds[i].v_pid);
			break;
		}
	}
	free(msg);
	free(msg2);

	_exit(SUCCESS);
}

