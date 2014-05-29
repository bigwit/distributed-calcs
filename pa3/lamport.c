/*
 * lamport.c
 *
 *  Created on: 30 мая 2014 г.
 *      Author: nikit
 */

#include "ipc.h"

static timestamp_t l_time = 0;

timestamp_t get_lamport_time() {
	return l_time;
}

void set_lamport_time(timestamp_t local_time) {
	if (local_time > l_time) {
		l_time = local_time;
	}
	l_time++;
}

void inc_lamport_time() {
	l_time++;
}
