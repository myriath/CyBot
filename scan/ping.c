/*
 * ping.c
 *
 *  Created on: Mar 30, 2022
 *      Author: wmhudson
 */

#include "ping.h"\

static uint8_t initialized = 0;

static uint8_t state = 0;
float startTime = 0;
float endTime = 0;

void TIMER3B_Handler() {
    TIMER3_ICR_R |= 0x400;

    if (state) {
        startTime = TIMER3_TBR_R;
        state = 0;
    } else {
        endTime = TIMER3_TBR_R;
        state = 1;
    }
}

void ping_init() {
    //Check if already initialized
    if(initialized){
        return;
    }

    SYSCTL_RCGCGPIO_R |= 0x2;
    while (!(SYSCTL_PRGPIO_R & 0x2)) {}
    SYSCTL_RCGCTIMER_R |= 0x8;
    while (!(SYSCTL_PRTIMER_R & 0x8)) {}

    GPIO_PORTB_DIR_R &= ~0x8;
    GPIO_PORTB_AFSEL_R |= 0x8;
    GPIO_PORTB_DEN_R |= 0x8;
    GPIO_PORTB_PCTL_R |= 0x7000;

//    IntRegister(INT_TIMER3B, TIMER3B_Handler);
    IntMasterEnable();

    TIMER3_CTL_R &= ~0x100;

    TIMER3_CFG_R |= 0x4;
    TIMER3_TBMR_R |= 0x7;
    TIMER3_CTL_R |= 0xc00;
    TIMER3_TBPR_R |= 0xff;
    TIMER3_TBILR_R |= 0xffff;
    TIMER3_IMR_R |= 0x400;
    TIMER3_ICR_R |= 0x400;
    NVIC_EN1_R |= 0x10;
    NVIC_PRI9_R |= 0x20;

    IntRegister(INT_TIMER3B, TIMER3B_Handler);
//    IntMasterEnable();

    TIMER3_CTL_R |= 0x100;

    initialized = 1;
}

void ping_trigger() {
    TIMER3_IMR_R &= ~0x400;
    TIMER3_CTL_R &= ~0x100;
    GPIO_PORTB_DIR_R |= 0x8;
    GPIO_PORTB_AFSEL_R &= ~0x8;

    GPIO_PORTB_DATA_R |= 0x8;
    timer_waitMillis(0.005);
    GPIO_PORTB_DATA_R &= ~0x8;

    GPIO_PORTB_DIR_R &= ~0x8;
    GPIO_PORTB_AFSEL_R |= 0x8;
    TIMER3_IMR_R |= 0x400;
    TIMER3_CTL_R |= 0x100;
}

float ping_getDistanceRAW() {
    return endTime - startTime;
}

float ping_getDistanceCM() {
    return (endTime - startTime) / 32000000 * 34300;
}
