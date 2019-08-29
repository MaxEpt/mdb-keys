/*
 * mdb_stop_keys.c
 *
 * Created: 11.01.2019 15:24:18
 * Author : Maxim
 * SLAVE CONNECTS TO USART 2
 * MASTER CONNECTS TO USART 3 
 * RS485(raspberry) CONNECTS TO USART 0
 */ 


#include "main.h"
//uint8_t arTest[4] = {0x05,0x06,0x07,0x08};

//CASHLESS

volatile uint8_t ucsr2b_reg=0;
volatile uint8_t udr2_reg=0;



ISR(TIMER5_COMPA_vect){
	TCNT5 = 0;
	send_to_slave(POLL,2);	
	//send_to_rpi(arTest,4);
}
int main(void)
{
    _delay_ms(5000);
	//PORT CONFIGURTION: OUTPUT, WILL CONTROL RECIEVE AND TRANSMIT on MAX 485
    DDRJ |= (1<<PJ2);
    DISABLE_RS485_TRANSMIT;
	
    poll_timer_init();
	slave_usart_init();
	
	rpi_usart_init();
	
	vmc_usart_init();
	SUMM_REGISTER = 0;
	CAN_SEND = 0;
	//SUMM_REGISTER = 9950;
	sei();
	
	
	START_POLL_TIMER;
	
	while (1) 
    {
		asm("nop");
		
    }
}

void poll_timer_init(){
	OCR5A = 500;//3125
	TIMSK5 |= (1<<OCIE5A);
}

void slave_usart_init(){
	UBRR2H = 0;
	UBRR2L = 103; // 9600bps for 16 MHz
	UCSR2C &= ~(1<<USBS2);//1 stop bit
	//set 9 bit mode
	UCSR2C |= (1<<UCSZ20) | (1<<UCSZ21);
	UCSR2B |= (1<<UCSZ22);
	//=====================
	UCSR2C &= ~(1<<UPM20) & ~(1<<UPM21);//no parity
	UCSR2B |= (1<<RXEN2) | (1<<TXEN2);//ENABLE Receive and TRASMIT
	UCSR2B |= (1<<RXCIE2);//ENABLE RX INTERRUPT
}

ISR(USART2_RX_vect){
	ucsr2b_reg = UCSR2B;
	udr2_reg = UDR2;
	
	CASHLESS_RESPONSE[CASHLESS_COUNTER] = ((((ucsr2b_reg>>1)&1)<<8) | udr2_reg);
	CASHLESS_COUNTER++;
	if(ucsr2b_reg & (1<<RXB82)){
		CASHLESS_COUNTER = 0;
		cashless_complete_handler();
	}
	
}