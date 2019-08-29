/*
 * raspberry.h
 *
 * Created: 21.01.2019 17:08:20
 *  Author: Maxim
 * USING RS-485 interface (MAX485)
 *
 *
 */ 


#ifndef RASPBERRY_H_
#define RASPBERRY_H_
#include "main.h"

#define ENABLE_RS485_TRANSMIT PORTJ |=(1<<PJ2)
#define DISABLE_RS485_TRANSMIT PORTJ &=~(1<<PJ2)
#define SLAVE_ID 0x06

volatile uint8_t rpi_answer[6];
void rpi_usart_init();
void send_to_rpi(uint8_t * pRpiCommand, uint8_t rpiCommandSize);
void rpi_complete_handler();
uint16_t calc_crc16( uint8_t rpiRespSize, uint8_t *pRpiResp, uint16_t seed );




#endif /* RASPBERRY_H_ */