/*
 * ipc.c
 *
 *  Created on: 10 апр. 2014 г.
 *      Author: nikit
 */

#include <unistd.h>
#include "ipc.h"
#include "pipe.h"

/* локальный идентификатор процесса */
extern local_id my_local_id;

/* количество процессов */
extern size_t num_proc;

int send(void * self, local_id dst, const Message * msg) {
	int pipedes = get_write(my_local_id, dst);
	if (pipedes <= 0 || write(pipedes, msg, sizeof(Message) == -1)) {
		return -1;
	}
	return 0;
}

int send_multicast(void * self, const Message * msg) {
	for (local_id id_dst = 0; id_dst < num_proc; id_dst++) {
		if (id_dst != my_local_id) {
			if (send(self, id_dst, msg) != 0) {
				return -1;
			}
		}
	}
	return 0;
}

int receive(void * self, local_id from, Message * msg) {
	int pipedes = get_read(my_local_id, from);
	if (pipedes <= 0 || read(pipedes, msg, sizeof(Message)) == -1) {
		return -1;
	}
	return 0;
}

int receive_any(void * self, Message * msg) {
	for (local_id id_from = 0; id_from < num_proc; id_from++) {
		if (id_from != my_local_id) {
			if (receive(self, id_from, msg) == -1) {
				return -1;
			}
		}
	}
	return 0;
}
