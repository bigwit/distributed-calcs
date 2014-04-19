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
#include "pipe.h"

/* Закрывает все дескрипторы каналов */
static void close_all(void);

/* Получить канал в указанной позиции */
static pipe_t * get(const size_t i, const size_t j);

/* дескрипторы каналов */
pipe_t * pipes;

/* количество процессов, связанных каналами */
size_t num_proc;

int init_pipes(const size_t _num_proc) {
	if (pipes != NULL) {
		close_all();
		free(pipes);
	}
	num_proc = _num_proc;
	pipes = calloc(num_proc * num_proc, sizeof(pipe_t));
	for (size_t i = 0; i < num_proc; i++) {
		for (size_t j = 0; j < num_proc; j++) {
			// на главной диагонали все каналы должны быть закрыты
			if (i != j) {
				if (!pipe((int *) get(i, j))) {
					return -1;
				}
			}
		}
	}
	return 0;
}

void configure_pipes(const local_id id_proc) {
	pipe_t * pipe;
	for (size_t i = 0; i < num_proc; i++) {
		for (size_t j = 0; j < num_proc; j++) {
			pipe = get(i, j);
			if (i != id_proc && j != id_proc) {
				close(pipe->read);
				close(pipe->write);
			} else if (i == id_proc) {
				close(pipe->read);
			} else if (j == id_proc) {
				close(pipe->write);
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

static void close_all(void) {
	pipe_t * i = pipes;
	while (i != NULL) {
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