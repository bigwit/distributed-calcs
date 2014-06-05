/*
 * queue.h
 *
 *  Created on: 30 мая 2014 г.
 *      Author: nikit
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include "ipc.h"

/**
 * Занять место в очереди
 */
void queue_push(const local_id id_proc, const timestamp_t local_time);

/**
 * Следующий процесс в очереди
 */
local_id next_proc(void);

local_id get_head(void);

/**
 * Проверяет размер очереди. Если очередь пуста,
 * возвращается 1, иначе возвращается 0
 */
int is_empty_queue(void);

#endif /* QUEUE_H_ */
