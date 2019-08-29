/*
 * cashless_emul.c
 *
 * Created: 22.02.2019 2:19:02
 *  Author: Maxim
 */ 
#include "cashless_emul.h"

uint16_t last_vmc_answer = 0;
uint16_t ACK_TO_VMC[1] = {0x100};
uint16_t JUST_RESET[2] = {0x000,0x100};
uint16_t SETUP_CONFIG_DATA_ANSWER[9] = {0x001,0x001,0x018,0x010,0x001,0x000,0x0F0,0x00B,0x126};
uint16_t EXPANSION_ANSWER[31] = {
	0x009,
	0x052,
	0x046,
	0x054,
	0x030,
	0x030,
	0x030,
	0x030,
	0x030,
	0x030,
	0x030,
	0x031,
	0x033,
	0x032,
	0x033,
	0x030,
	0x032,
	0x031,
	0x030,
	0x035,
	0x030,
	0x045,
	0x045,
	0x043,
	0x044,
	0x046,
	0x057,
	0x04B,
	0x001,
	0x034,
	0x164
};

uint16_t START_SESSION_ANSWER[4] = {0x003,0x000,0x000,0x000};
uint16_t SESSION_CANCEL_ANSWER[2] = {0x004,0x104};
uint16_t SESSION_COMPLETE_ANSWER[2] = {0x007,0x107};
uint16_t VEND_APPROVE_ANSWER[4] = {0x005,0x000,0x032,0x137};	
	
	
/*-----*/	
void vmc_usart_init(){
	UBRR3H = 0;
	UBRR3L = 103; // 9600bps for 16 MHz
	UCSR3C &= ~(1<<USBS3);//1 stop bit
	//set 9 bit mode
	UCSR3C |= (1<<UCSZ30) | (1<<UCSZ31);
	UCSR3B |= (1<<UCSZ32);
	//=====================
	UCSR3C &= ~(1<<UPM30) & ~(1<<UPM31);//no parity
	UCSR3B |= (1<<RXEN3) | (1<<TXEN3);//ENABLE Receive and TRASMIT
	UCSR3B |= (1<<RXCIE3);//ENABLE RX INTERRUPT
}
/*-----*/
void send_to_vmc(uint16_t * pVmcAnswer, uint8_t VmcAnswerSize){
	uint16_t byte = 0;
	last_vmc_answer = pVmcAnswer[0];
	for (uint8_t i = 0; i<VmcAnswerSize; i++){
		while ( !( UCSR3A & (1<<UDRE3)));
		UCSR3B &= ~(1<<TXB83);
		byte = pVmcAnswer[i];
		if(byte & 0x100){
			UCSR3B |= (1<<TXB83);
			byte &= 0xFF;
		}
		UDR3 = byte;
	}
}
/*-----*/
volatile uint8_t ucsr3b_reg=0;
volatile uint8_t udr3_reg=0;
volatile uint16_t CRC=0;
volatile uint8_t VEND_SUCCESED=0;
ISR(USART3_RX_vect){
	ucsr3b_reg = UCSR3B;
	udr3_reg = UDR3;
	
	VMC_COMMAND[VMC_COUNTER] = ((((ucsr3b_reg>>1)&1)<<8) | udr3_reg);
	
		
	if(VMC_COMMAND[VMC_COUNTER]==(CRC & 0xFF)){		
		VMC_COUNTER = 0;
		CRC=0;
		vmc_complete_handler();
	}else{
		if(!(VMC_COMMAND[VMC_COUNTER]==0 & VMC_COUNTER==0)){
			CRC += VMC_COMMAND[VMC_COUNTER];
			VMC_COUNTER ++;
		}
	}
	
}
/*-----*/
uint8_t JUST_ENABLED = 1;
uint8_t NEED_CANCEL = 0;
uint8_t SESSION_IDLE_STATE = 0;
uint8_t NEED_VEND_APPROVED = 0;
void vmc_complete_handler(){
	//asm("nop");
	switch (VMC_COMMAND[0]){
		case 0x112:
			if(JUST_ENABLED){
				JUST_ENABLED = 0;
				send_to_vmc(JUST_RESET, 2);
			}else{
				if(KEY_VALUE>0){
					if(NEED_VEND_APPROVED){
						NEED_VEND_APPROVED = 0;
						send_to_vmc(VEND_APPROVE_ANSWER,4);						
					}else{						
						if(SESSION_IDLE_STATE){							
							send_to_vmc(ACK_TO_VMC, 1);
						}else{
							if(CAN_SEND){
								for(uint8_t i = 1; i<4; i++)
									START_SESSION_ANSWER[i] = 0x00;
								CAN_SEND = 0;								
								NEED_CANCEL = 1;
								SESSION_IDLE_STATE = 1;
								START_SESSION_ANSWER[1] |= SUMM_REGISTER>>8;
								START_SESSION_ANSWER[2] |= SUMM_REGISTER & 0xFF;							
								START_SESSION_ANSWER[3] = START_SESSION_ANSWER[0]+START_SESSION_ANSWER[1]+START_SESSION_ANSWER[2];
								if(!(START_SESSION_ANSWER[3] & 0x0100)){
									START_SESSION_ANSWER[3]+=0x100;
								}//Исправить
								send_to_vmc(START_SESSION_ANSWER, 4);

							}else{
								send_to_vmc(ACK_TO_VMC, 1);
							}
						}
					}
				}else{
					if(NEED_CANCEL){
						send_to_vmc(SESSION_CANCEL_ANSWER, 2);
						NEED_CANCEL = 0;						
						SESSION_IDLE_STATE = 0;
					}else{
						send_to_vmc(ACK_TO_VMC, 1);	
					}
					
					//send_to_vmc(ACK_TO_VMC, 1);	
				}
				
			}
		break;
		case 0x111:
			if(VMC_COMMAND[1]==0x000){
				send_to_vmc(SETUP_CONFIG_DATA_ANSWER, 9);
			}
			if(VMC_COMMAND[1]==0x001){
				send_to_vmc(ACK_TO_VMC,1);
			}
		break;
		case 0x117:
			send_to_vmc(EXPANSION_ANSWER,31);
		break;
		case 0x114:
			send_to_vmc(ACK_TO_VMC,1);
		break;
		case 0x113:
			if(VMC_COMMAND[1]==0x004){
				send_to_vmc(SESSION_COMPLETE_ANSWER, 2);	
				if(!VEND_SUCCESED){
					SUMM_REGISTER = 0;
					CAN_SEND = 0;
					VEND_SUCCESED = 0;
				}
				//asm("nop");	
			}
			if(VMC_COMMAND[1]==0x000){
				NEED_VEND_APPROVED = 1;				
				send_to_vmc(ACK_TO_VMC,1);
			}
			if(VMC_COMMAND[1]==0x002){
				send_to_vmc(ACK_TO_VMC,1);
				SUMM_REGISTER=SUMM_REGISTER-50;
				SESSION_IDLE_STATE = 0;
				CAN_SEND = 1;
				VEND_SUCCESED = 1;
				KEY_VALUE = CASHLESS_RESPONSE[1];
				KEY_VALUE=KEY_VALUE<<8;
				KEY_VALUE |= CASHLESS_RESPONSE[2];
				rpi_answer[1] = CASHLESS_RESPONSE[1];
				rpi_answer[2] = CASHLESS_RESPONSE[2];
				rpi_answer[3] = 0x01;
			}
			
		break;
		
		default:
		break;
	}
}