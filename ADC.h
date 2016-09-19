#ifndef _ADC_
#define _ADC_
#include "std_types.h"
#include "common_macros.h"
/*
 * Description :
 * Function responsible for initialize the ADC driver.
 */
void ADC_init();
/*
 * Description :
 * Function responsible for read analog data from a certain ADC channel
 * and convert it to digital using the ADC driver.
 */
uint16 ADC_read(uint8 channel_num);



#endif