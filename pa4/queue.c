/*
 * queue.c
 *
 *  Created on: 30 мая 2014 г.
 *      Author: nikit
 */

#include <malloc.h>
#include <stddef.h>

#include "ipc.h"

typedef struct self {
	local_id id_proc;
	timestamp_t local_time;
	struct self * next;
}__attribute__((packed)) entry_t;

// первый элемент очереди
static entry_t * head = NULL;

static void push_internal(entry_t * new_entry);

void queue_push(const local_id id_proc, const timestamp_t local_time) {
	// создаем новый экземпляр элемента очереди
	entry_t * entry = (entry_t *) calloc(1, sizeof(entry_t));
	entry->id_proc = id_proc;
	entry->local_time = local_time;
	entry->next = NULL;

	// если очередь пуста
	if (head == NULL) {
		// созданный элемент становится в начало
		head = entry;
	}
	// если не пуста
	else {
		// добавить процесс в очередь на положенное место
		push_internal(entry);
	}
}

static void push_internal(entry_t * new_entry) {
	// возможно такая плюшка прокатит
	entry_t * top = (entry_t *) calloc(1, sizeof(entry_t));
	entry_t * e = top;
	e->next = head;

	// пока существует след. элемент,
	// локальное время нового процесса больше локального времени элемента,
	// локальное время нового процесса равно времени элемента
	// и идентификатор больше либо равен идентификатору элемента
	while (e->next
			&& (new_entry->local_time > e->next->local_time
					|| (new_entry->local_time == e->next->local_time
							&& new_entry->id_proc >= e->next->id_proc))) {
		// двигаться вглубь очереди
		e = e->next;
	}
	// добавить элемент на положенное место
	new_entry->next = e->next;
	e->next = new_entry;
	if (new_entry->next == head) {
		head = new_entry;
	}

	// и освобождаем место из-под хитрой плюшки
	free(top);
}

local_id get_head(void) {
	return (head) ? head->id_proc : -1;
}

local_id next_proc(void) {
	// если очередь пустая
	if (!head) {
		return -1;
	}

	entry_t * entry = head;
	// началом очереди становится следующий элемент
	head = head->next;
	// получаем идентификатор процесса
	local_id id_proc = entry->id_proc;

	// освобождаем память из-под выбранного элемента
	free(entry);
	return id_proc;
}

int is_empty_queue(void) {
	return (!head) ? 1 : 0;
}
