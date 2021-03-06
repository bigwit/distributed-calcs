/*
 * lamport.h
 *
 *  Created on: 30 мая 2014 г.
 *      Author: nikit
 */

#ifndef LAMPORT_H_
#define LAMPORT_H_

/*
 * Передвигает значение логических часов на следующую отметку.
 * Если значение local_time больше текущего, то часы уравниваются
 * с local_time и к ним прибавляется единица. В остальных случаях
 * к текущему значению логических часов прибавляется единица
 *
 * @param local_time значение логических часов, которое выставлено у
 * 		источника события. В данном случае это текущее значение
 * 		логических часов у процесса, отправившего сообщение
 */
void set_lamport_time(timestamp_t local_time);

/*
 * Увеличивает значение логических часов на единицу. Годится для
 * внутреннего перевода часов перед событием, например, отправкой сообщения
 */
void inc_lamport_time();

/*
 * Возвращает текущее значение логических часов
 */
timestamp_t get_lamport_time();

#endif /* LAMPORT_H_ */
