#include "ADC.h"
#include <avr/io.h>
void ADC_init(){
	/* ADMUX Register Bits Description:
	 * REFS1:0 = 00 to choose to connect external reference voltage by input this voltage through AREF pin
	 * ADLAR   = 0 right adjusted
	 * MUX4:0  = 00000 to choose channel 0 as initialization
	 */
	ADMUX =0;
	/* ADCSRA Register Bits Description:
	 * ADEN    = 1 Enable ADC
	 * ADIE    = 0 Disable ADC Interrupt
	 * ADPS2:0 = 111 to choose ADC_Clock=F_CPU/128=16Mhz/128=125Khz --> ADC must operate in range 50-200Khz
	 */
	ADCSRA = (1<<ADEN) | (1<<ADPS1) | (1<<ADPS0)| (1<<ADPS2);
}

uint16 ADC_read(uint8 channel_num){
		channel_num &= 0x07; /* channel number must be from 0 --> 7 */
		ADMUX &= 0xE0; /* clear first 5 bits in the ADMUX (channel number MUX4:0 bits) before set the required channel */
		ADMUX = ADMUX | channel_num; /* choose the correct channel by setting the channel number in MUX4:0 bits */
		SET_BIT(ADCSRA,ADSC); /* start conversion write '1' to ADSC */
		while(BIT_IS_CLEAR(ADCSRA,ADIF)); /* wait for conversion to complete ADIF becomes '1' */
		SET_BIT(ADCSRA,ADIF); /* clear ADIF by write '1' to it :) */
		return ADC; /* return the data register */
}