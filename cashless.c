/*
 * cashless.c
 *
 * Created: 11.01.2019 16:07:17
 *  Author: Maxim
 */ 

#include "cashless.h"

uint16_t ACK[1] = {0x000};
uint16_t POLL[2]={0x112,0x012};
uint16_t RESET[2] = {0x110,0x010};
uint16_t SETUP_CONFIG_DATA[7] = {0x111,0x000,0x001,0x010,0x002,0x001,0x025};
uint16_t SETUP_PRICE_DATA[7] = {0x111,0x001,0x0FF,0x0FF,0x000,0x000,0x010};	
uint16_t WRITE_TIME[32] = {//change name
	0x117,
	0x000,
	0x0FF,
	0x0FF,
	0x000,
	0x000,
	0x010,
	0x00B,
	0x001,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x000,
	0x031
};	
uint16_t ENABLE[3] = {0x114,0x001,0x015};
uint16_t SESSION_COMPLETE[3] = {0x113,0x004,0x017};
	
	
volatile uint16_t CASHLESS_RESPONSE[40] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

uint16_t last_cashless_command = 0;
volatile uint8_t CASHLESS_COUNTER = 0;	

void send_to_slave(uint16_t * pSlaveCommand, uint8_t slaveCommandSize){
	uint16_t byte = 0;
	last_cashless_command = pSlaveCommand[0];
	for (uint8_t i = 0; i<slaveCommandSize; i++){
		while ( !( UCSR2A & (1<<UDRE2)));
		UCSR2B &= ~(1<<TXB82);
		byte = pSlaveCommand[i];
		if(byte & 0x100){
			UCSR2B |= (1<<TXB82);
			byte &= 0xFF;
		}
		UDR2 = byte;
	}
}

uint8_t  change= 0;
void cashless_complete_handler(){		
	switch(last_cashless_command){
		case 0x112:
			switch (CASHLESS_RESPONSE[0]){
				case 0x000://just reset
					STOP_POLL_TIMER;
					send_to_slave(ACK,1);
					send_to_slave(SETUP_CONFIG_DATA,7);
					
				break;
				case 0x100:					

					//KEY_VALUE = 0;
					//rpi_answer[1] = 0;
					//rpi_answer[2] = 0;									
				break;
				
				case 0x003:
					KEY_VALUE = CASHLESS_RESPONSE[1];
					KEY_VALUE=KEY_VALUE<<8;
					KEY_VALUE |= CASHLESS_RESPONSE[2];
					rpi_answer[1] = CASHLESS_RESPONSE[1];
					rpi_answer[2] = CASHLESS_RESPONSE[2];
					send_to_slave(ACK,1);
				break;
				case 0x004:

					send_to_slave(ACK,1);
					send_to_slave(SESSION_COMPLETE,3);
				break;
				default:
				break;
			}
		break;
		case 0x111:
			if(CASHLESS_RESPONSE[0]==0x100){
				//send_to_slave(ACK,1);
				send_to_slave(WRITE_TIME,32);
			}else{
				send_to_slave(ACK,1);
				send_to_slave(SETUP_PRICE_DATA,7);
			}
		break;
		case 0x117:
			send_to_slave(ACK,1);
			send_to_slave(ENABLE,3);
		break;
		case 0x114:
			//send_to_slave(ACK,1);
			START_POLL_TIMER;
		break;
		case 0x113:
			KEY_VALUE = 0;
			SUMM_REGISTER = 0;
			send_to_slave(ACK,1);
		break;
		default:
		break;
		
	}
}
	
