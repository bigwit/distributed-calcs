/*
 * queue.c
 *
 *  Created on: 30 мая 2014 г.
 *      Author: nikit
 */

#include <malloc.h>
#include <stddef.h>

typedef struct self {
	local_id id_proc;
	struct self * next;
} entry_t __attribute__((packed));

// первый элемент очереди
static entry_t * head = NULL;

// последний элемент очереди
static entry_t * tail = NULL;

void push(const local_id id_proc) {
	// создаем новый экземпляр элемента очереди
	entry_t * entry = (entry_t *) calloc(1, sizeof(entry_t));
	entry->id_proc = id_proc;
	entry->next = NULL;

	// если очередь пуста
	if (head == NULL) {
		// созданный элемент становится и началом, и концом очереди
		head = tail = entry;
	}
	// если не пуста
	else {
		// конец очереди указывает на созданный элемент
		tail->next = entry;
		// созданный элемент становится в конец очереди
		tail = entry;
	}
}

local_id next_proc(void) {
	entry_t entry = head;
	local_id id_proc = entry.id_proc;
	// если следующий элемент в очереди существует
	if (head->next != NULL) {
		// началом очереди становится следующий элемент
		head = head->next;
	}
	// если больше нет элементов
	else {
		// обнуляем начало и конец очереди
		head = tail = NULL;
	}
	// освобождаем память из-под выбранного элемента
	free(entry);
	return id_proc;
}

int is_empty_queue(void) {
	return (head == NULL) ? 1 : 0;
}
