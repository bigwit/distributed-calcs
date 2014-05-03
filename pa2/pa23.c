/*
 * pa23.c
 *
 * Created 3 май 2014 г.
 * 		Author: nikit
 */

#include <errno.h>
#include <getopt.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

#include "pa2345.h"
#include "ipc.h"
#include "banking.h"
#include "pipe.h"

#define PERM 0666
#define FFLAGS O_CREAT | O_APPEND | O_TRUNC | O_WRONLY

/*
 * Действия дочерних процессов (типа С). Функция не возвращает
 * управление вызывающему коду, она завершает выполнение процесса
 *
 * @param _local_id локальный идентификатор процесса
 * @param account объем денежных средств на счету
 */
static void handle_child(const local_id _local_id, const size_t _balance) __attribute__((noreturn));

/*
 * Заполняет экземпляр структуры BalanceState данными о текущем балансе
 *
 * @param state указатель на структуру для заполнения
 */
static void init_history_state(BalanceState * const state);

/*
 * Заполняет структуру Message данными
 *
 * @param msg указатель на область памяти для заполнения
 * @param line текст сообщения. Длина не должна превышать 4096 символов
 * @param type тип сообщения
 */
static void init_message(Message * const msg, const void * const payload,
		const MessageType type);

/*
 * Ожидает от всех дочерних процессов сообщения BALANCE_HISTORY
 * и помещает данные сообщения в структуру history
 *
 * @param history структура, хранящая информацию о
 * 		балансах всех процессов
 */
static void wait_all_history(AllHistory * const history);

/*
 * Полезная работа дочернего процесса.
 * Ожидаем от любого процесса сообщение. Оно может быть двух типов - TRANSFER и STOP.
 * Если тип сообщения TRANSFER -
 * 		от родительского процесса - вычитаем amount из своего баланса,
 * 			записываем в историю переводов
 * 			баланса и пересылаем сообщение процессу dst
 * 		от дочернего процесса - прибавляем amount к своему балансу,
 * 			делаем запись в истории переводов, отправляем родительскому процессу
 * 			пустое сообщение типа ACK
 * Если тип сообщения STOP - отправляем всю историю переводов родительскому процессу
 *
 * @param history указатель на экземпляр структуры, хранящей историю всех переводов данного филиала
 */
static void handle_transfers(BalanceHistory * const history,
		int * excludes_done);

/*
 * Ожидает от всех дочерних процессов сообщение указанного типа
 *
 * @param type тип ожидаемого сообщения
 */
extern int wait_all(const MessageType type, int * excludes);

// локальный идентификатор процесса. У родительского процесса он равен 0
local_id my_local_id;
// количество филиалов, представленных дочерними процессами
size_t proc_count;
// размер счета
balance_t balance;
// буфер для сообщений логов. Также используется для указания текста сообщения
static char log_msg[4096];
// файл events.log
int events_fd;

extern char *optarg;
extern int optind;

int main(int argc, char * argv[]) {
	//bank_robbery(parent_data);
	//print_history(all);

	// получаем количество филиалов
	switch (getopt(argc, argv, "p:")) {
	case '?':
		_exit(-1);
	case -1:
		fprintf(stderr, "Required parameter -p\n");
		_exit(-1);
	}

	// количество филиалов должно быть в диапазоне [2; 10]
	proc_count = atoi(optarg);
	if (proc_count < 2 || proc_count > 10) {
		fprintf(stderr, "Invalid number processes\n");
		_exit(-1);
	}

	argc -= optind;
	argv += optind;

	if (argc != proc_count) {
		fprintf(stderr, "Invalid number accounts\n");
		_exit(-1);
	}

	// проверка размеров счетов у филиалов
	// размер счета должен быть в диапазоне [1; 99]
	size_t accounts[proc_count];
	for (size_t i = 0; i < proc_count; i++) {
		int acc_size = atoi(argv[i]);
		if (acc_size < 1 || acc_size > 99) {
			fprintf(stderr, "Invalid size account: %d. Must be [1; 99]\n",
					acc_size);
			_exit(-1);
		}
		accounts[i] = acc_size;
	}

	events_fd = open("events.log", FFLAGS, PERM);
	if (events_fd == -1) {
		perror("open");
		_exit(-1);
	}

	// создание каналов для процессов
	init_pipes(proc_count + 1);
	flush_pipes_to_log();

	// запуск дочерних процессов
	for (local_id id_proc = 1; id_proc <= proc_count; id_proc++) {
		switch (fork()) {
		case -1:
			perror("fork");
			_exit(-1);
		case 0:
			handle_child(id_proc, accounts[id_proc - 1]);
		}
	}

	my_local_id = 0;
	configure_pipes(my_local_id);

	// ожидание готовности всех дочерних процессов
	wait_all(STARTED, NULL);
	sprintf(log_msg, log_received_all_started_fmt, get_physical_time(), 0);
	printf(log_msg, NULL);
	write(events_fd, log_msg, strlen(log_msg));

	// выполнение переводов
	bank_robbery(NULL, (local_id) proc_count);

	// отправка сообщения STOP всем дочерним процессам
	Message msg;
	memset(&msg, 0, sizeof(msg));
	init_message(&msg, NULL, STOP);
	send_multicast(NULL, &msg);

	// ожидание окончания переводов всех дочерних процессов
	wait_all(DONE, NULL);
	sprintf(log_msg, log_received_all_done_fmt, get_physical_time(), 0);
	printf(log_msg, NULL);
	write(events_fd, log_msg, strlen(log_msg));
	close(events_fd);

	// ожидаем от всех дочерних процессов истории переводов
	AllHistory * const history = calloc(1, sizeof(AllHistory));
	wait_all_history(history);

	// и куда-то там выводим
	print_history(history);
	free(history);

	for (local_id id_proc = 1; id_proc <= proc_count; id_proc++) {
		if (wait(NULL) == -1) {
			perror("wait");
			_exit(-1);
		}
	}

	return 0;
}

void transfer(void * parent_data, local_id src, local_id dst, balance_t amount) {
	Message * msg = (Message *) calloc(1, sizeof(Message));
	TransferOrder *order = (TransferOrder *) calloc(1, sizeof(TransferOrder));
	order->s_src = src;
	order->s_dst = dst;
	order->s_amount = amount;
	init_message(msg, order, TRANSFER);
	send(NULL, src, msg);

	memset(msg, 0, sizeof(Message));
	receive(NULL, dst, msg);
	if (msg->s_header.s_type != ACK) {
		write(STDERR_FILENO, (void *) msg, sizeof(Message));
	}

	free(order);
	free(msg);
}

static void handle_child(const local_id _local_id, const size_t _balance) {
	// записываем свой локальный идентификатор и размер счета
	my_local_id = _local_id;
	balance = _balance;

	// запись в лог STARTED
	sprintf(log_msg, log_started_fmt, get_physical_time(), my_local_id,
			getpid(), getppid(), balance);
	printf(log_msg, NULL);
	write(events_fd, log_msg, strlen(log_msg));

	// дочерний процесс закрывает ненужные дескрипторы каналов
	configure_pipes(my_local_id);

	// создаем сообщение STARTED и отправляем его всем процессам
	Message msg;	// = calloc(1, sizeof(Message));
	init_message(&msg, log_msg, STARTED);

	send_multicast(NULL, &msg);

	// ожидаем от всех дочерних процессов сообщение STARTED
	wait_all(STARTED, NULL);
	sprintf(log_msg, log_received_all_started_fmt, get_physical_time(),
			my_local_id);
	printf(log_msg, NULL);
	write(events_fd, log_msg, strlen(log_msg));

	// выделяем место под историю переводов
	BalanceHistory * history = (BalanceHistory*) calloc(1,
			sizeof(BalanceHistory));
	history->s_id = my_local_id;
	int * exclude_done = calloc(proc_count + 1, sizeof(int));
	// делаем полезную работу
	handle_transfers(history, exclude_done);
	printf("%d balance history, len = %d\n", history->s_id,
			history->s_history_len);
	for (int i = 0; i < history->s_history_len; i++) {
		printf("%d balance state: time %d, balance %d\n", history->s_id,
				history->s_history[i].s_time, history->s_history[i].s_balance);
	}

	// выводим в лог сообщение об окончании работы
	sprintf(log_msg, log_done_fmt, get_physical_time(), my_local_id, balance);
	printf(log_msg, NULL);
	write(events_fd, log_msg, strlen(log_msg));

	// отправляем всем сообщение DONE
	memset(&msg, 0, sizeof(Message));
	init_message(&msg, log_msg, DONE);
	if (send_multicast(NULL, &msg) < 0) {
		perror("send_multicast");
		_exit(-12);
	}

	// отправить родительскому процессу сообщение BALANCE_HISTORY
	memset(&msg, 0, sizeof(Message));
	init_message(&msg, history, BALANCE_HISTORY);
	for (int i = 0; i < sizeof(BalanceHistory); i++) {
		msg.s_payload[i] = ((char *) history)[i];
	}
	msg.s_header.s_payload_len = sizeof(BalanceHistory);
	send(NULL, 0, &msg);

//	free(msg);

	// ожидаем от всех дочерних процессов сообщение DONE
	if (wait_all(DONE, exclude_done) < 0) {
		perror("wait_all");
		_exit(-11);
	}

	// write to log (received_all_done)
	sprintf(log_msg, log_received_all_done_fmt, get_physical_time(),
			my_local_id);
	printf(log_msg, NULL);
	write(events_fd, log_msg, strlen(log_msg));

	// закрываем все открытые дескрипторы каналов
	close_all();
	// завершаем процесс
	close(events_fd);
	_exit(0);

	_exit(0);
}

static void init_history_state(BalanceState * state) {
	state->s_balance = balance;
	state->s_balance_pending_in = 0;
	state->s_time = get_physical_time();
}

static void handle_transfers(BalanceHistory * const history,
		int * excludes_done) {
	Message msg;
	TransferOrder * order;
	MessageType msg_type = 0;
	local_id from = 0;
	uint8_t history_len = 0;
	do {
		// ожидаем сообщение от любого процесса
		memset(&msg, 0, sizeof(msg));
		receive_any(&from, &msg);
		// получаем тип сообщения
		msg_type = msg.s_header.s_type;
		// действия в зависимости от типа. Если тип сообщения
		// отличен от TRANSFER и STOP, процесс завершится с ошибкой
		switch (msg_type) {
		case TRANSFER:
			order = (TransferOrder*) msg.s_payload;
			// если родительский процесс прислал сообщение
			if (from == 0) {
				balance -= order->s_amount;
				sprintf(log_msg, log_transfer_out_fmt, get_physical_time(),
						order->s_src, order->s_amount, order->s_dst);
				printf(log_msg, NULL);
				write(events_fd, log_msg, strlen(log_msg));
				send(NULL, order->s_dst, &msg);
			}
			// если прислал сообщение дочерний процесс
			else {
				balance += order->s_amount;
				sprintf(log_msg, log_transfer_in_fmt, get_physical_time(),
						order->s_src, order->s_amount, order->s_dst);
				printf(log_msg, NULL);
				write(events_fd, log_msg, strlen(log_msg));
				memset(&msg, 0, sizeof(msg));
				init_message(&msg, NULL, ACK);
				send(NULL, 0, &msg);
			}
			// записываем в историю переводов запись о текущем балансе
			init_history_state(&history->s_history[history_len++]);
			history->s_history_len = history_len;
			break;
		case STOP:
			// заполняем количество записей о переводах
//			history->s_history_len = history_len;
			return;
		case DONE:
			excludes_done[from] = 1;
			break;
		default:
			fprintf(stderr, "unknown message type: %d\n", msg_type);
			_exit(-1);
		}
	} while (1);

//	free(msg);
}

static void wait_all_history(AllHistory * const history) {
	history->s_history_len = proc_count;
	Message * msg = (Message *) calloc(1, sizeof(Message));
	for (local_id src = 1; src <= proc_count; src++) {
		receive(NULL, src, msg);
		if (msg->s_header.s_type != BALANCE_HISTORY) {
			fprintf(stderr, "Expected message type BALANCE_HISTORY, but: %d\n",
					msg->s_header.s_type);
			_exit(-1);
		}
		history->s_history[src - 1] = *((BalanceHistory *) msg->s_payload);
	}
}

static void init_message(Message * const msg, const void * const payload,
		const MessageType type) {
	MessageHeader header;
	// заполнение заголовка сообщения
	header.s_local_time = (timestamp_t) get_physical_time(); // время создания сообщения
	header.s_magic = MESSAGE_MAGIC; // магическое число по заданию
	header.s_type = type; // тип сообщения
	if (payload != NULL) {
		header.s_payload_len = strlen(payload); // длина передаваемого сообщения
		strcpy(msg->s_payload, (const char *) payload); // текст сообщения
	} else {
		header.s_payload_len = 0;
	}
	msg->s_header = header;
}
