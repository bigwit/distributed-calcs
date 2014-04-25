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
	int bytes = 0;
	if (pipedes <= 0 || (bytes = write(pipedes, msg, sizeof(Message))) <= 0) {
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
	// сообщения ожидаются только от дочернего процесса,
	// родительский только принимает сообщения
	if (pipedes <= 0) {
		return -1;
	}
	int bytes = read(pipedes, msg, sizeof(Message));
	if (bytes == -1) {
		return -1;
	}
	return 0;
}

int receive_any(void * self, Message * msg) {
	// обходим все дочерние процессы, не затрагивая родительский
	for (local_id id_from = 1; id_from < num_proc; id_from++) {
		if (id_from != my_local_id) {
			// и ожидаем сообщения от них
			if (receive(self, id_from, msg) == -1) {
				return -1;
			} else {
				return 0;
			}
		}
	}
	return 0;
}
//
//static int check_status(const char * const status) {
//	for (local_id id_proc = 0; id_proc < num_proc; id_proc++) {
//		if (!status[id_proc]) {
//			return 0;
//		}
//	}
//	return 1;
//}

int wait_all(const MessageType type) {
	// хранит информацию о факте получения сообщения от
	// дочерних процессов - 0 - сообщение не получено,
	// 1 - сообщение получено
	char * status = calloc(num_proc, sizeof(char));
	// сразу исключаем принятие сообщений от родительского и текущего процесса
	status[0] = 1;
	status[my_local_id] = 1;
	// выделяем память под сообщение для сравнения типов
	Message * msg = calloc(1, sizeof(Message));
//	local_id id_from = 0;
	//while (!check_status(status)) {
	for (int i = 1; i < num_proc; i++) {
		// ищем следующий идентификатор процесса,
		// от которого не получено сообщение
//		while (status[id_from]) {
//			id_from = (id_from < (num_proc - 1)) ? id_from + 1 : 0;
//		}

		if (i == my_local_id) {
			continue;
		}
		// получаем сообщение от процесса
		memset(msg, 0, sizeof(Message));
		int result = receive(NULL, i, msg);
		if (result != 0) {
			return -1;
		}
		// проверяем, что тип полученного сообщения совпадает с ожидаемым
//		if (msg->s_header.s_type == type && status[i] == 0) {
//			status[id_from] = 1;
//		}
	}
	free(status);
	free(msg);
	return 0;
}
