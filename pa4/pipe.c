/*
 * pipe.c
 *
 *  Created on: 16 апр. 2014 г.
 *      Author: nikit
 */

#include <sys/types.h>
#include <unistd.h>
#include <stddef.h>
#include <malloc.h>
#include <fcntl.h>
#include <string.h>
#include "pipe.h"
#include "common.h"

#define PERM 0666
#define LOG_FILE_FLAGS O_CREAT | O_APPEND | O_TRUNC | O_WRONLY

/* Получить канал в указанной позиции */
static pipe_t * get(const size_t i, const size_t j);

/* устанавливает флаг O_NONBLOCK для дескрипторов каналов */
static void set_nonblocking(const int * fds);

/* запись каналов в лог */
void flush_pipes_to_log();

/* дескрипторы каналов */
pipe_t * pipes;

/* количество процессов, связанных каналами */
size_t num_proc;

int init_pipes(const size_t _num_proc) {
	// если ранее уже выделялась память под каналы,
	// все каналы будут закрыты и память будет освобождена
	if (pipes != NULL) {
		close_all();
		free(pipes);
	}
	num_proc = _num_proc;
	// двумерная матрица каналов представлена как одномерный массив
	pipes = calloc(num_proc * num_proc, sizeof(pipe_t));
	// открытие каналов всем процессам
	// далее все процессы сами должны закрывать ненужные дескрипторы каналов
	int * fds;
	for (size_t i = 0; i < num_proc; i++) {
		for (size_t j = 0; j < num_proc; j++) {
			// на главной диагонали все каналы должны быть закрыты
			if (i != j) {
				fds = (int *) get(i, j);
				if (pipe(fds) == -1) {
					return -1;
				}
				set_nonblocking(fds);
			}
		}
	}

	return 0;
}

static void set_nonblocking(const int * fds) {
	int fd, flags;
	for (int i = 0; i < 2; i++) {
		fd = fds[i];
		if ((flags = fcntl(fd, F_GETFL)) == -1) {
			perror("fcntl");
			_exit(-1);
		}
		if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
			perror("fcntl");
			_exit(-1);
		}
	}
}

void configure_pipes(const local_id id_proc) {
	pipe_t * pipe;

	// Процессы закрывают дескрипторы по след. правилам -
	// 1. Если канал связывает процессы, локальный id которых не совпадает с id_proc -
	// они будут полностью закрыты
	// 2. Каналы связывают процессы с идентификаторами, соответствующими
	// номерам строк и столбцов в матрице каналов, где
	// i - номер канала запись, j - номер канала на чтение
	// 3. Родительский процесс ничего не пишет в каналы
	for (size_t i = 0; i < num_proc; i++) {
		for (size_t j = 0; j < num_proc; j++) {
			pipe = get(i, j);
			// если канал никак не связан с данными процессами
			if (i != id_proc && j != id_proc) {
				// то закрыть оба дескриптора
				if (pipe->read != 0) {
					close(pipe->read);
					pipe->read = 0;
				}
				if (pipe->write != 0) {
					close(pipe->write);
					pipe->write = 0;
				}
			}
			// если локальный id процесса совпадает с номером строки
			if (i == id_proc && pipe->read != 0) {
				// закрыть на чтение, оставить на запись
				close(pipe->read);
				pipe->read = 0;
			}
			// если локальный id процесса совпадает с номером столбца
			if (j == id_proc && pipe->write != 0) {
				// закрыть на запись, оставить на чтение
				close(pipe->write);
				pipe->write = 0;
			}
		}
	}
}

int get_read(const local_id id_src, const local_id id_dst) {
	pipe_t * pipe = get(id_dst, id_src);
	return (pipe != NULL) ? pipe->read : -1;
}

int get_write(const local_id id_src, const local_id id_dst) {
	pipe_t * pipe = get(id_src, id_dst);
	return (pipe != NULL) ? pipe->write : -1;
}

static pipe_t * get(const size_t i, const size_t j) {
	if (i < num_proc && j < num_proc) {
		return &pipes[num_proc * i + j];
	}
	return NULL;
}

void close_all(void) {
	pipe_t * i = pipes;
	for (int index = 0; index < num_proc; index++) {
		if (i->read != 0) {
			close(i->read);
			i->read = 0;
		}
		if (i->write != 0) {
			close(i->write);
			i->write = 0;
		}
		i++;
	}
}

const char * const log_pipe_frm =
		"pipe from %d to %d with read desc %d and wrire desc %d\n";

void flush_pipes_to_log(void) {
	int pi_log = open(pipes_log, LOG_FILE_FLAGS, PERM);
	char log_msgs[1024];

	for (int i = 0; i < num_proc; ++i) {
		for (int j = 0; j < num_proc; ++j) {
			pipe_t * p = get(i, j);
			if (p != NULL) {
				sprintf(log_msgs, log_pipe_frm, i, j, p->read, p->write);
				write(pi_log, log_msgs, strlen(log_msgs));
			}
		}
	}

	close(pi_log);
}
