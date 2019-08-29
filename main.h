/*
 * main.h
 *
 * Created: 11.01.2019 16:19:46
 *  Author: Maxim
 */ 


#ifndef MAIN_H_
#define MAIN_H_
#define F_CPU 16000000L
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "cashless.h"
#include "raspberry.h"


#define START_POLL_TIMER TCCR5B |= (1<<CS50) | (1<<CS52);
#define STOP_POLL_TIMER TCCR5B &= ~(1<<CS52) & ~(1<<CS51) & ~(1<<CS50);

void poll_timer_init();
void slave_usart_init();

volatile uint16_t SUMM_REGISTER;
volatile uint16_t CAN_SEND;


#endif /* MAIN_H_ */