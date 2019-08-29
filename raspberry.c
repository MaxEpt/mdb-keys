/*
 * raspberry.c
 *
 * Created: 21.01.2019 17:08:37
 *  Author: Maxim
 */ 
#include "raspberry.h"



void rpi_usart_init(){
	UBRR0H = 0;
	UBRR0L = 103; // 9600bps for 16 MHz
	UCSR0C &= ~(1<<USBS0);//1 stop bit	
	UCSR0C |= (1<<UCSZ00) | (1<<UCSZ01);	
	UCSR0C &= ~(1<<UPM00) & ~(1<<UPM01);//no parity
	UCSR0B |= (1<<RXEN0) | (1<<TXEN0);//ENABLE Receive and TRASMIT
	UCSR0B |= (1<<RXCIE0);//ENABLE RX INTERRUPT
		/*UBRR0H = 0;
		UBRR0L = 103;
		UCSR0C |= (1<<UCSZ20) | (1<<UCSZ21);
		UCSR0B |= (1<<RXEN2)|(1<<TXEN2);*/
		//UCSR0B |= (1<<RXCIE2);//|(1<<UDRIE2);
	

}

void send_to_rpi(uint8_t * pRpiCommand, uint8_t rpiCommandSize){	
	ENABLE_RS485_TRANSMIT;	
	for (uint8_t i = 0; i<rpiCommandSize; i++){
		UDR0 = pRpiCommand[i];	
		do {} while ( !( UCSR0A & (1<<UDRE0)));					
	}
	//do {} while ( !( UCSR0A & (1<<TXC0)));
	_delay_ms(1);
	DISABLE_RS485_TRANSMIT;	
	
	
}

uint16_t calc_crc16( uint8_t rpiRespSize, uint8_t *pRpiResp, uint16_t seed ){
	int i, j;
	uint16_t crc = seed;
	for ( i = 0; i < rpiRespSize; ++i )
	{
		crc ^= ( pRpiResp[ i ] << 8 );
		for ( j = 0; j < 8; ++j )
		{
			if ( crc & 0x8000 )
			crc = ( crc << 1 ) ^ 0x1021; // 0001.0000 0010.0001 = x^12 + x^5+ 1 ( + x^16 )
			else
			crc <<= 1;
		}
	}
	return crc;
}


volatile uint8_t rpi_command[6] = {0,0,0,0,0,0};
volatile uint8_t rpi_received_byte_num = 0;
ISR(USART0_RX_vect){
	rpi_command[rpi_received_byte_num] = UDR0;	
	rpi_received_byte_num++;
	if(rpi_received_byte_num==6){
		rpi_received_byte_num = 0;
		rpi_complete_handler();
	}
	
}

volatile uint8_t rpi_answer[6] = {SLAVE_ID, 0x00, 0x00,0x00,0,0};
uint16_t summ = 0;

void rpi_complete_handler(){
	asm("nop");
	uint16_t crc = calc_crc16(4, rpi_command, 0);
	uint8_t lsb=crc;
	uint8_t msb=crc>>8;
	if(rpi_command[0] == SLAVE_ID && lsb == rpi_command[4] && msb == rpi_command[5]){

		switch (rpi_command[1])		
		{
			case 0x01:
				
				//ƒобавить флаг на проверку списани€
				//увеличить размер ответа до 4 байт +
				//если 1 в последнем байте, то отправл€ть списание 50 на сервер
				crc = calc_crc16(4, rpi_answer, 0);
				lsb=crc;
				msb=crc>>8;
				rpi_answer[4] = lsb;
				rpi_answer[5] = msb;
				send_to_rpi(rpi_answer, 6);
				rpi_answer[1] = 0;
				rpi_answer[2] = 0;
				rpi_answer[3] = 0;
				rpi_answer[4] = 0;
				rpi_answer[5] = 0;
				crc = 0;
				lsb = 0;
				msb = 0;	 
			break;
			case 0x02:
				//SUMM_REGISTER = 0;
				summ = rpi_command[2];
				summ=summ<<8;
				summ |= rpi_command[3];
				SUMM_REGISTER = summ;
				summ = 0;
				CAN_SEND = 1;
				//asm("nop");
			case 0x03:
				for(uint8_t i = 0; i<6; i++){
					rpi_answer[i] = 0;
					rpi_command[i] = 0;
				}
				rpi_answer[0] = SLAVE_ID;
			break;
		break;
			default:
			break;
		}
		
		
		
	}

}