/*
 * piezosensortoEEPROM.c
 *
 * Created: 20/3/2016 11:25:26
 * Author : J
 */ 



#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#define 	F_CPU   8000000UL
#define BAUD 9600                           // define baud
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1)

void uart_init (void)
{
	UBRR0H=(BAUDRATE>>8);
	UBRR0H=BAUDRATE;                         //set baud rate
	UCSR0B|=(1<<TXEN0)|(1<<RXEN0);             //enable receiver and transmitter
	UCSR0C|=(1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1);// 8bit data format
}


void uart_transmit (unsigned char data)
{
	while (!( UCSRA & (1<<UDRE)));            // wait while register is free
	UDR = data;                             // load data in the register
}

void adc_init()
{
	// AREF = AVcc
	ADMUX = (1<<REFS0);
	
	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

uint16_t adc_read(uint8_t ch)
{
	// select the corresponding channel 0~7
	// ANDing with '7' will always keep the value
	// of 'ch' between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8)|ch;     // clears the bottom 3 bits before ORing
	
	// start single conversion
	// write '1' to ADSC
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes '0' again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}

int main(void)
{
	uint16_t adc_result0, adc_result1;
	char int_buffer[10];
	adc_init();
	uart_init();  

	
    while(1)
    {
		adc_result0 = adc_read(2);
        
		itoa(adc_result0, int_buffer, 10);
		eeprom_write_byte ((uint8_t*) 23, int_buffer[0]);
		eeprom_write_byte ((uint8_t*) 24, int_buffer[1]);
		eeprom_write_byte ((uint8_t*) 25, int_buffer[2]);
		eeprom_write_byte ((uint8_t*) 26, int_buffer[3]);
		uart_transmit(int_buffer[0]);
		uart_transmit(int_buffer[1]);
		uart_transmit(int_buffer[2]);
		uart_transmit(int_buffer[3]);
		uart_transmit('\r');
		 _delay_ms(500);
    }
}

