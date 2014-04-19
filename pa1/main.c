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
#include <string.h>
#include <malloc.h>
#include <time.h>
#include "ipc.h"
#include "common.h"
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
		case -1:
			_exit(-1);
		case 0:
			handle_child(i);
			break;
		default:
			break;
		}
	}
}

void handle_child(const local_id _local_id) {
	my_local_id = _local_id;

	// дочерний процесс закрывает ненужные дескрипторы каналов
	configure_pipes(my_local_id);

	_exit(0);
}

void init_message(Message * const msg, const char * const line, const MessageType type) {
	MessageHeader header;
	header.s_local_time = (timestamp_t) time(NULL); // время создания сообщения
	header.s_magic = MESSAGE_MAGIC; // магическое число по заданию
	strcpy(msg->s_payload, line);
	header.s_payload_len = strlen(line); // текст сообщения
	header.s_type = type;
	msg->s_header = header;
}
