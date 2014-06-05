/*
 * pa2345.c
 *
 *  Created on: 23 мая 2014 г.
 *      Author: nikit
 */

#include <malloc.h>
#include <string.h>
#include <unistd.h>

#include "ipc.h"
#include "lamport.h"
#include "pa2345.h"
#include "queue.h"

extern void init_message(Message * const msg, const char * const line,
		const MessageType type);

extern size_t num_proc;
extern int done_count;
extern local_id my_local_id;

int request_cs(const void * self) {
	if (done_count == 0) {
		inc_lamport_time();
		return 0;
	}
	Message * msg = (Message *) calloc(1, sizeof(Message));
	memset(msg, 0, sizeof(Message));
	inc_lamport_time();
	init_message(msg, NULL, CS_REQUEST);
	queue_push(my_local_id, get_lamport_time());
	for (local_id id_from = 1; id_from <= num_proc; id_from++) {
		if (id_from != my_local_id) {
			send(NULL, id_from, msg);
		}
	}

	local_id from;

	int count_reply = num_proc - 2;

	while (1) {
		memset(msg, 0, sizeof(Message));
		receive_any(&from, msg);
//		printf("%d: type message %d\n", my_local_id, msg->s_header.s_type);
		switch (msg->s_header.s_type) {
		case CS_REPLY: {
			count_reply--;
//			printf("%d: reply head - %d\n", my_local_id, get_head());
			if (count_reply <= 0 && get_head() == my_local_id) {
				return 0;
			}
			break;
		}
		case CS_REQUEST: {
//			printf("%d: request\n", my_local_id);
			queue_push(from, msg->s_header.s_local_time);
			memset(msg, 0, sizeof(Message));
			inc_lamport_time();
			init_message(msg, NULL, CS_REPLY);
			send(NULL, from, msg);
			break;
		}
		case CS_RELEASE: {
//			printf("%d: release\n", my_local_id);
			next_proc();
			if (get_head() == my_local_id) {
				return 0;
			}
			break;
		}
		case DONE: {
			done_count--;
			if (done_count <= 0) {
				return 0;
			}
			break;
		}
		default:
			fprintf(stderr, "unknown type message: %d\n", msg->s_header.s_type);
		}
	}

	free(msg);
	return 0;
}

int release_cs(const void * self) {
	if (done_count == 0) {
		inc_lamport_time();
		return 0;
	}
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
