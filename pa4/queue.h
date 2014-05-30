/*
 * queue.h
 *
 *  Created on: 30 мая 2014 г.
 *      Author: nikit
 */

#ifndef QUEUE_H_
#define QUEUE_H_

/**
 * Занять место в очереди
 */
void push(const local_id id_proc);

/**
 * Следующий процесс в очереди
 */
local_id next_proc(void);

/**
 * Проверяет размер очереди. Если очередь пуста,
 * возвращается 1, иначе возвращается 0
 */
int is_empty_queue(void);

#endif /* QUEUE_H_ */
