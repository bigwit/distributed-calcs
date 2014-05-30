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
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

#include "common.h"
#include "ipc.h"
#include "pipe.h"
#include "pa2345.h"
#include "lamport.h"

#define LIMIT_SIZE_LOG_MESSAGE 1024
#define PERM 0666
#define LOG_FILE_FLAGS O_CREAT | O_APPEND | O_TRUNC | O_WRONLY

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
void init_message(Message * const msg, const char * const line,
		const MessageType type);

/*
 * Ожидает сообщения указанного типа от всех дочерних процессов
 *
 * @param type тип ожидаемого сообщения
 */
extern int wait_all(const MessageType type);

extern char * optarg;
local_id my_local_id = 0;
int ev_log;

// 1, если параметр --mutexl указан, 0 в противном случае
int is_mutex = 0;

char log_msg[LIMIT_SIZE_LOG_MESSAGE];

int main(int argc, char ** argv) {
	pid_t pid;

	const char * short_opts = "mp:";
	const struct option long_opts[] = {
			{ "mutexl", no_argument, NULL, 'm' },
			{ NULL, 0, NULL, 0 }
	};

	int opt;
	int opt_index;
	int num_proc;

	while ((opt = getopt_long(argc, argv, short_opts, long_opts, &opt_index))
			!= -1) {
		switch (opt) {
		case 'p':
			printf("parameter p\n");
			num_proc = atoi(optarg);
			if (num_proc <= 0 || num_proc > 10) {
				fprintf(stderr, "Invalid number processes\n");
				_exit(-2);
			}
			break;
		case 'm': is_mutex = 1; break;
		case '?': default:
			fprintf(stderr, "unknown option: %c\n", opt);
			_exit(-1);
		}
	}

	ev_log = open(evengs_log, LOG_FILE_FLAGS, PERM);
	if (ev_log == -1) {
		fprintf(stderr, "Log 'events' is not initialize\n");
		_exit(-8);
	}

	init_pipes((size_t) num_proc + 1);

	flush_pipes_to_log();

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
	sprintf(log_msg, log_received_all_started_fmt, get_lamport_time(), 0);
	printf(log_msg, NULL);
	write(ev_log, log_msg, strlen(log_msg));

	wait_all(DONE);
	sprintf(log_msg, log_received_all_done_fmt, get_lamport_time(), 0);
	printf(log_msg, NULL);
	write(ev_log, log_msg, strlen(log_msg));

	for (int i = 0; i < num_proc; ++i) {
		// ожидаем завершения всех дочерних процессов
		if (wait(NULL) == -1) {
			perror("wait");
			_exit(-3);
		}
	}

	close_all();
	close(ev_log);

	return 0;
}

void handle_child(const local_id _local_id) {
	my_local_id = _local_id;
	sprintf(log_msg, log_started_fmt, get_lamport_time(), _local_id, getpid(), getppid(), 0);
	printf(log_msg, NULL);
	write(ev_log, log_msg, strlen(log_msg));

	// дочерний процесс закрывает ненужные дескрипторы каналов
	configure_pipes(my_local_id);

	// создаем сообщение STARTED и отправляем его всем процессам
	Message * msg = calloc(1, sizeof(Message));
	init_message(msg, log_msg, STARTED);
	inc_lamport_time();
	msg->s_header.s_local_time = get_lamport_time();
	send_multicast(NULL, msg);

	// ожидаем от всех дочерних процессов сообщение STARTED
	wait_all(STARTED);
	sprintf(log_msg, log_received_all_started_fmt, get_lamport_time(), _local_id);
	printf(log_msg, NULL);
	write(ev_log, log_msg, strlen(log_msg));

	char * str = (char *) calloc(256, sizeof(char));
	char * buf = (char *) calloc(256, sizeof(char));
	for (int i = 1; i <= my_local_id * 5; i++) {
		sprintf(str, log_loop_operation_fmt, my_local_id, i, my_local_id * 5);
		if (is_mutex) {
			request_cs(&my_local_id);
		}
		memcpy(buf, str, 256);
		print(buf);
		if (is_mutex) {
			release_cs(&my_local_id);
		}
	}

	sprintf(log_msg, log_done_fmt, get_lamport_time(), _local_id, 0);
	printf(log_msg, NULL);
	write(ev_log, log_msg, strlen(log_msg));

	// отправляем всем сообщение DONE
	memset(msg, 0, sizeof(Message));
	init_message(msg, log_msg, DONE);
	inc_lamport_time();
	msg->s_header.s_local_time = get_lamport_time();
	if (send_multicast(NULL, msg) < 0) {
		perror("send_multicast");
		_exit(-12);
	}

	free(msg);

	// ожидаем от всех дочерних процессов сообщение DONE
	if (wait_all(DONE) < 0) {
		perror("wait_all");
		_exit(-11);
	}

	// write to log (received_all_done)
	sprintf(log_msg, log_received_all_done_fmt, get_lamport_time(), _local_id);
	printf(log_msg, NULL);
	write(ev_log, log_msg, strlen(log_msg));

	// закрываем все открытые дескрипторы каналов
	close_all();
	// завершаем процесс
	close(ev_log);
	_exit(0);
}

void init_message(Message * const msg, const char * const line,
		const MessageType type) {
	MessageHeader header;
	// заполнение заголовка сообщения
	header.s_local_time = (timestamp_t) time(NULL); // время создания сообщения
	header.s_magic = MESSAGE_MAGIC; // магическое число по заданию
	header.s_type = type; // тип сообщения
	if (line != NULL) {
		header.s_payload_len = strlen(line); // длина передаваемого сообщения
		strcpy(msg->s_payload, line); // текст сообщения
	} else {
		header.s_payload_len = 0;
	}
	msg->s_header = header;
}
