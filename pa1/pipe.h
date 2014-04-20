/*
 * pipe.h
 *
 *  Created on: 16 апр. 2014 г.
 *      Author: nikit
 */

#ifndef PIPE_H_
#define PIPE_H_

#include <sys/types.h>
#include "ipc.h"

/*
 * Структура представляет собой однонаправленный канал,
 * имеющий два дескриптора - на чтение и запись
 */
typedef struct {
	int read;
	int write;
} __attribute__((packed)) pipe_t;

/*
 * Создает каналы для полносвязной
 * топологии сети между процессами.
 * При повторном вызове функции старые
 * каналы закрываются
 *
 * @param num_proc количество процессов
 *
 * @return 0 в случае успешного открытия каналов,
 * 		ненулевое значение при ошибке
 */
int init_pipes(const size_t num_proc);

/*
 * Закрывает неиспользуемые каналы процессом
 * с заданным локальным идентификатором
 *
 * @param id_proc локальный идентификатор процесса
 */
void configure_pipes(const local_id id_proc);

/*
 * Возвращает дескриптор канала на чтение из
 * процесса с id_proc для процесса с local_id
 *
 * @param id_src локальный id
 * 		процесса, который запрашивает канал
 * @param id_dst локальный id
 * 		процесса, который связывается каналом
 *
 * @return дескриптор канала на чтение для процесса с local_id,
 * 		или -1 в случае если такого канала не существует
 */
int get_read(const local_id id_src, const local_id id_dst);

/*
 * Возвращает дескриптор канала на запись в
 * процесс с id_proc для процесса с local_id
 *
 * @param id_src локальный id
 * 		процесса, который запрашивает канал
 * @param id_dst локальный id
 * 		процесса, который связывается каналом
 *
 * @return дескриптор канала на запись для процесса с local_id,
 * 		или -1 в случае если такого канала не существует
 */
int get_write(const local_id id_src, const local_id id_dst);

/* запись каналов в лог */
void flush_pipes_to_log();

#endif /* PIPE_H_ */
