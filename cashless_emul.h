/*
 * cashless_emul.h
 *
 * Created: 22.02.2019 2:06:40
 *  Author: Maxim
 */ 


#ifndef CASHLESS_EMUL_H_
#define CASHLESS_EMUL_H_

#include "main.h"


uint16_t JUST_RESET[2];
uint16_t SETUP_CONFIG_DATA_ANSWER[9];
uint16_t ACK_TO_VMC[1];
uint16_t SETUP_PRICE_DATA_ANSWER[7];
uint16_t EXPANSION_ANSWER[31];
uint16_t START_SESSION_ANSWER[4];
uint16_t SESSION_CANCEL_ANSWER[2];
uint16_t VEND_APPROVE_ANSWER[4];


volatile uint16_t VMC_COMMAND[40];


volatile uint8_t VMC_COUNTER;



void vmc_usart_init();
void send_to_vmc(uint16_t * pVmcAnswer, uint8_t VmcAnswerSize);
void vmc_complete_handler();


#endif /* CASHLESS_EMUL_H_ */