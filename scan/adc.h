/*
 * adc.h
 *
 *  Created on: Mar 23, 2022
 *      Author: wmhudson
 */

#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>
#include <inc/tm4c123gh6pm.h>

void adc_init(void);
uint16_t adc_read(void);

#endif /* ADC_H_ */
