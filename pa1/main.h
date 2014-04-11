/*
 * main.h
 *
 *  Created on: 11 апр. 2014 г.
 *      Author: alex
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "ipc.h"
#include "common.h"
#include "pa1.h"
#include <sys/types.h>

#define PARAM_COUNT_PROC_NAME "p:"
#define SUCCESS 0

void handle_child(void) __attribute__((noreturn));
int has_receved_by_flag(int pattern);
void init_message(Message * message, char * const line, MessageType type);
void wait_all(local_id id);

typedef struct {
	local_id v_pid;
	pid_t r_pid;
	int pipedes[2];
} descriptor;


#endif /* MAIN_H_ */
