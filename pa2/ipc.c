/*
 * ipc.c
 *
 *  Created on: 10 апр. 2014 г.
 *      Author: nikit
 */

#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include "ipc.h"
#include "pipe.h"

/* локальный идентификатор процесса */
extern local_id my_local_id;

/* количество процессов */
extern size_t num_proc;

int send(void * self, local_id dst, const Message * msg) {
	int pipedes = get_write(my_local_id, dst);
	if (pipedes < 0) {
		return -1;
	}
	while (write(pipedes, msg,
			sizeof(MessageHeader) + msg->s_header.s_payload_len) <= 0)
		;
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
	// сообщения ожидаются только от дочернего процесса,
	// родительский только принимает сообщения
	while (read(pipedes, &msg->s_header, sizeof(MessageHeader)) <= 0);
	while (read(pipedes, &msg->s_payload, msg->s_header.s_payload_len) == -1);
	return 0;
}

int receive_any(void * self, Message * msg) {
	int status = -1;
	local_id from;
	// обходим все процессы, включая родительский
	for (from = 1; status <= 0; from = (from < num_proc) ? from + 1 : 0) {
		// не затрагивая себя самого
		if (from == my_local_id) {
			continue;
		}

		// получаем дескриптор канала на чтение
		int pipe_fd = get_read(my_local_id, from);
		// и ожидаем сообщения от них
		if (read(pipe_fd, &msg->s_header, sizeof(MessageHeader)) > 0) {
			*((local_id *) self) = from;

			while (read(pipe_fd, msg->s_payload, msg->s_header.s_payload_len) == -1);

			return 0;
		}
	}
	return -1;
}

int wait_all(const MessageType type, int * excludes) {
	// выделяем память под сообщение для сравнения типов
	Message msg;
	for (local_id i = 1; i < num_proc; i++) {
		if (i == my_local_id || (excludes != NULL && excludes[i] != 0)) {
			continue;
		}
		// получаем сообщение от процесса
		memset(&msg, 0, sizeof(msg));
		int result = receive(NULL, i, &msg);
		if (result != 0) {
			return -1;
		}
		// проверяем, что пришедшее сообщение того же типа, что и ожидалось
		if (msg.s_header.s_type != type) {
			fprintf(stderr, "expected type %d, but received %d\n", type,
					msg.s_header.s_type);
			return -1;
		}
	}
	return 0;
}
