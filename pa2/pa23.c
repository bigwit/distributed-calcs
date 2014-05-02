#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <getopt.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <wait.h>
#include "pa1.h"
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
static void handle_child(const local_id _local_id, const size_t account) __attribute__((noreturn));

/*
 * Заполняет структуру Message данными
 *
 * @param msg указатель на область памяти для заполнения
 * @param line текст сообщения. Длина не должна превышать 4096 символов
 * @param type тип сообщения
 */
static void init_message(Message * const msg, const char * const line,
		const MessageType type);

/*
 * Ожидает от всех дочерних процессов сообщения BALANCE_HISTORY
 * и помещает данные сообщения в структуру history
 *
 * @param history структура, хранящая информацию о
 * 		балансах всех процессов
 */
static void wait_all_history(AllHistory * const history);

extern int wait_all(const MessageType type);

// локальный идентификатор процесса. У родительского процесса он равен 0
local_id my_local_id;
// количество филиалов, представленных дочерними процессами
size_t proc_count;
// размер счета
size_t size_account;
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
	int proc_count = atoi(optarg);
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
	wait_all(STARTED);
	sprintf(log_msg, log_received_all_started_fmt, 0);
	printf(log_msg, NULL);
	write(events_fd, log_msg, strlen(log_msg));

	// выполнение переводов
	bank_robbery(NULL, (local_id) proc_count);

	// отправка сообщения STOP всем дочерним процессам
	Message *msg = (Message *) calloc(1, sizeof(Message));
	init_message(msg, NULL, STOP);
	send_multicast(NULL, msg);
	free(msg);

	// ожидание окончания переводов всех дочерних процессов
	wait_all(DONE);
	sprintf(log_msg, log_received_all_done_fmt, 0);
	printf(log_msg, NULL);
	write(events_fd, log_msg, strlen(log_msg));

	// ожидаем от всех дочерних процессов истории переводов
	AllHistory * const history = calloc(1, sizeof(AllHistory));
	history->s_history_len = proc_count;
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

static void handle_child(const local_id _local_id, const size_t account) {
	printf("child: id: %d, account: %d\n", _local_id, account);
	_exit(0);
}

void transfer(void * parent_data, local_id src, local_id dst, balance_t amount) {
	// FIXME: implement me, please
}

static void wait_all_history(AllHistory * const history) {

	return;
}

static void init_message(Message * const msg, const char * const line,
		const MessageType type) {
	MessageHeader header;
	// заполнение заголовка сообщения
	header.s_local_time = (timestamp_t) get_physical_time(); // время создания сообщения
	header.s_magic = MESSAGE_MAGIC; // магическое число по заданию
	header.s_type = type; // тип сообщения
	if (line != NULL) {
		header.s_payload_len = strlen(line); // длина передаваемого сообщения
		strcpy(msg->s_payload, line); // текст сообщения
	} else {
		header.s_payload_len = 0;
	}
	msg->s_header = header;
}
