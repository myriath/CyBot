/*
 * adc.c
 *
 *  Created on: Mar 23, 2022
 *      Author: wmhudson
 */

#include "adc.h"

static uint8_t initialized = 0;

void adc_init() {
    //Check if already initialized
    if(initialized){
        return;
    }

    //ADCCC, ADCSSMUX10, ADCSSCTL10, ADCACTSS, ADCEMUX, ADCPSSI,

    SYSCTL_RCGCGPIO_R |= 0x2;
    SYSCTL_RCGCADC_R |= 0x1;
    while (!(SYSCTL_PRGPIO_R & 0x2)) {};

    GPIO_PORTB_DIR_R &= ~0x10;
    GPIO_PORTB_AFSEL_R |= 0x10;
    GPIO_PORTB_DEN_R &= ~0x10;
    GPIO_PORTB_AMSEL_R |= 0x10;

    while (!(SYSCTL_PRADC_R & 0x1)) {};
    ADC0_ACTSS_R &= ~0x1;

    ADC0_EMUX_R &= ~0xf;
    ADC0_SSMUX0_R &= ~0xf;
    ADC0_SSMUX0_R |= 0xa;
    ADC0_SSCTL0_R |= 0x6;
    ADC0_SAC_R |= 0x4;

    ADC0_ACTSS_R |= 0x1;

    initialized = 1;
}

uint16_t adc_read() {
    ADC0_PSSI_R |= 0x1;
    while (!(ADC0_RIS_R & ADC_RIS_INR0)) {}

    uint16_t ret = ADC0_SSFIFO0_R & 0xfff;
    ADC0_ISC_R |= 0x1;
    return ret;
}
