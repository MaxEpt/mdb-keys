/*
 * cashless.h
 *
 * Created: 11.01.2019 16:06:01
 *  Author: Maxim
 */ 


#ifndef CASHLESS_H_
#define CASHLESS_H_
#include "main.h"


uint16_t POLL[2];
uint16_t SETUP_CONFIG_DATA[7];
uint16_t ACK[1];
uint16_t SETUP_PRICE_DATA[7];
uint16_t ENABLE[3];
uint16_t WRITE_TIME[32];

volatile uint16_t CASHLESS_RESPONSE[40];


volatile uint8_t CASHLESS_COUNTER;
volatile uint16_t KEY_VALUE;



void send_to_slave(uint16_t * pSlaveCommand, uint8_t slaveCommandSize);
void cashless_complete_handler();





#endif /* CASHLESS_H_ */