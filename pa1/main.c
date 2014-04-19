/*
 * main.c
 *
 *  Created on: 10 апр. 2014 г.
 *      Author: nikit
 */

#include <getopt.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "common.h"
#include "ipc.h"
#include "pa1.h"
#include "pipe.h"

/*
 * Определяет действия дочернего процесса. Данная
 * функция не должна возвращать управление вызывающему коду,
 * а должна завершать выполнение процесса
 *
 * @param id_proc локальный идентификатор процесса
 */
void handle_child(const local_id id_proc) __attribute__((noreturn));

/*
 * Производит инициализацию сообщения указанного типа,
 * заполняя поля нужными значениями
 *
 * @param msg выделенная память под сообщение
 * @param type тип сообщения
 */
void init_message(Message * const msg, const char * const line, const MessageType type);

/*
 * Ожидает сообщения указанного типа от всех дочерних процессов
 *
 * @param type тип ожидаемого сообщения
 */
extern void wait_all(const MessageType type);

extern char * optarg;
local_id my_local_id = 0;

int main(int argc, char ** argv) {
	pid_t pid;

	if (getopt(argc, argv, "p:") == -1) {
		fprintf(stderr, "Parameter '-p' is required\n");
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
		case -1:
			_exit(-1);
		case 0:
			handle_child(i);
			break;
		default:
			break;
		}
	}

	// дожидаемся сообщений старта и завершения
	// всех дочерних процессов
	wait_all(STARTED);
	wait_all(DONE);

	for(int i = 0; i < num_proc; ++i) {
		// ожидаем завершения всех дочерних процессов
		if(wait(NULL) == -1) {
			fprintf(stderr, "wait: unknown error\n");
			_exit(-3);
		}
	}
	return 0;
}

void handle_child(const local_id _local_id) {
	my_local_id = _local_id;

	// дочерний процесс закрывает ненужные дескрипторы каналов
	configure_pipes(my_local_id);

	// создаем сообщение STARTED и отправляем его всем процессам
	Message * msg = calloc(1, sizeof(Message));
	init_message(msg, "HI!", STARTED);
	send_multicast(NULL, msg);

	// ожидаем от всех дочерних процессов сообщение STARTED
	wait_all(STARTED);

	// делаем полезную работу (логирование добавится позже...)

	// отправляем всем сообщение DONE
	memset(msg, 0, sizeof(Message));
	init_message(msg, "BY!", DONE);
	send_multicast(NULL, msg);

	free(msg);

	// ожидаем от всех дочерних процессов сообщение DONE
	wait_all(DONE);

	// завершаем процесс
	_exit(0);
}

void init_message(Message * const msg, const char * const line, const MessageType type) {
	MessageHeader header;
	// заполнение заголовка сообщения
	header.s_local_time = (timestamp_t) time(NULL); // время создания сообщения
	header.s_magic = MESSAGE_MAGIC; // магическое число по заданию
	header.s_payload_len = strlen(line); // длина передаваемого сообщения
	header.s_type = type; // тип сообщения
	strcpy(msg->s_payload, line); // текст сообщения
	msg->s_header = header;
}
