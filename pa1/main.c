/*
 * main.c
 *
 *  Created on: 10 апр. 2014 г.
 *      Author: nikit
 */

#include <getopt.h>
#include <malloc.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>
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
extern int wait_all(const MessageType type);

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

	init_pipes((size_t) num_proc + 1);

	for (size_t i = 1; i <= num_proc; i++) {
		switch (pid = fork()) {
		case -1:
			_exit(-1);
		case 0:
			handle_child(i);
		}
	}

	configure_pipes(0);

	// дожидаемся сообщений старта и завершения
	// всех дочерних процессов
	wait_all(STARTED);
	wait_all(DONE);

	for(int i = 0; i < num_proc; ++i) {
		// ожидаем завершения всех дочерних процессов
		if(wait(NULL) == -1) {
			perror("wait");
			_exit(-3);
		}
	}
	return 0;
}

void handle_child(const local_id _local_id) {
	my_local_id = _local_id;
	printf(log_started_fmt, _local_id, getpid(), getppid());

	// дочерний процесс закрывает ненужные дескрипторы каналов
	configure_pipes(my_local_id);

	// создаем сообщение STARTED и отправляем его всем процессам
	Message * msg = calloc(1, sizeof(Message));
	init_message(msg, "HI!", STARTED);
	send_multicast(NULL, msg);

	// ожидаем от всех дочерних процессов сообщение STARTED
	wait_all(STARTED);
	printf(log_received_all_started_fmt, _local_id);

	// делаем полезную работу (логирование добавится позже...)

	printf(log_done_fmt, _local_id);

	// отправляем всем сообщение DONE
	memset(msg, 0, sizeof(Message));
	init_message(msg, "BY!", DONE);
	if (send_multicast(NULL, msg) < 0) {
		perror("send_multicast");
		_exit(-1);
	}

	free(msg);

	// ожидаем от всех дочерних процессов сообщение DONE
	if (wait_all(DONE) < 0) {
		perror("wait_all");
		_exit(-1);
	}

	printf(log_received_all_done_fmt, _local_id);

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
