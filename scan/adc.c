/*
 * adc.c
 *
 *  Created on: Mar 23, 2022
 *      Author: wmhudson
 */

#include "adc.h"

// 0 if not initialized, 1 if initialized.
static uint8_t initialized = 0;

/**
 * Initializes the adc.
 */
void adc_init() {
    //Check if already initialized
    if(initialized){
        return;
    }

    // Enable clock to adc and gpio
    SYSCTL_RCGCGPIO_R |= 0x2;
    SYSCTL_RCGCADC_R |= 0x1;
    while (!(SYSCTL_PRGPIO_R & 0x2)) {};

    // Set gpio direction
    GPIO_PORTB_DIR_R &= ~0x10;
    // Enable alternate functions on the pins.
    GPIO_PORTB_AFSEL_R |= 0x10;
    // Enable analog functions on the pins.
    GPIO_PORTB_DEN_R &= ~0x10;
    // Set alternate mode for the gpio.
    GPIO_PORTB_AMSEL_R |= 0x10;

    while (!(SYSCTL_PRADC_R & 0x1)) {};
    // Disable adc0 for config.
    ADC0_ACTSS_R &= ~0x1;
    // Configure adc functionality.
    ADC0_EMUX_R &= ~0xf;
    ADC0_SSMUX0_R &= ~0xf;
    ADC0_SSMUX0_R |= 0xa;
    ADC0_SSCTL0_R |= 0x6;
    ADC0_SAC_R |= 0x4;
    // Re-enable adc0 after config.
    ADC0_ACTSS_R |= 0x1;
    // Set initialized.
    initialized = 1;
}

/**
 * Reads a value from the adc (ir sensor).
 */
uint16_t adc_read() {
    // Triggers adc conversion.
    ADC0_PSSI_R |= 0x1;
    // Waits until completed conversion.
    while (!(ADC0_RIS_R & ADC_RIS_INR0)) {}

    // Returns value of adc and resets it.
    uint16_t ret = ADC0_SSFIFO0_R & 0xfff;
    ADC0_ISC_R |= 0x1;
    return ret;
}
