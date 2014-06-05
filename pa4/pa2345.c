/*
 * pa2345.c
 *
 *  Created on: 23 мая 2014 г.
 *      Author: nikit
 */

#include <malloc.h>
#include <string.h>

#include "ipc.h"
#include "lamport.h"
#include "pa2345.h"
#include "queue.h"

extern void init_message(Message * const msg, const char * const line,
		const MessageType type);

extern size_t num_proc;

extern local_id my_local_id;

static int is_all_responded(const char * st, const size_t size);

int request_cs(const void * self) {
	Message msg;
	memset(&msg, 0, sizeof(msg));
	inc_lamport_time();
	init_message(&msg, NULL, CS_REQUEST);
	queue_push(my_local_id, get_lamport_time());
	for (local_id id_from = 1; id_from <= num_proc; id_from++) {
		send(NULL, id_from, &msg);
	}

	local_id from;

	char * st_answer = (char *) calloc(num_proc - 1, sizeof(char));
	st_answer[my_local_id - 1] = 1;
	while (!is_all_responded(st_answer, num_proc - 1)
			&& get_head() != my_local_id) {
		memset(&msg, 0, sizeof(msg));
		receive_any(&from, &msg);

		switch (msg.s_header.s_type) {
		case CS_REPLY: {
			st_answer[from - 1] = 1;
			break;
		}
		case CS_REQUEST: {
			queue_push(from, msg.s_header.s_local_time);
			memset(&msg, 0, sizeof(msg));
			inc_lamport_time();
			init_message(&msg, NULL, CS_REPLY);
			send(NULL, from, &msg);
			break;
		}
		case CS_RELEASE: {
			next_proc();
			break;
		}
		}
	}

	return 0;
}

static int is_all_responded(const char * st, const size_t size) {
	for (size_t i = 0; i < size; i++) {
		if (!st[i]) {
			return 0;
		}
	}
	return 1;
}

int release_cs(const void * self) {
	Message msg;
	memset(&msg, 0, sizeof(msg));

	next_proc();
	inc_lamport_time();
	init_message(&msg, NULL, CS_RELEASE);

	for (local_id id_from = 1; id_from <= num_proc; id_from++) {
		send(NULL, id_from, &msg);
	}
	return 0;
}
